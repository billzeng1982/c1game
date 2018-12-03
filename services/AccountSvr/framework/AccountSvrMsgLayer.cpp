#include "AccountSvrMsgLayer.h"
#include "LogMacros.h"
#include "ObjectPool.h"
#include "hash_func.cpp"
#include "../AccountSvr.h"
#include "../logic/AccountTableMsg.h"

using namespace PKGMETA;

#define BASE_TIMESTAMP 1477324800
#define MAX_NUM_CREATE_ONE_SEC 255

AccountSvrMsgLayer::AccountSvrMsgLayer() : CMsgLayerBase( sizeof(PKGMETA::SCPKG)*2 , sizeof(PKGMETA::SSPKG)*2 )
{
    m_pstConfig = NULL;
}

bool AccountSvrMsgLayer::Init()
{
    ACCOUNTSVRCFG& rstDBSvrCfg =  AccountSvr::Instance().GetConfig();
    if( _Init(rstDBSvrCfg.m_iBusGCIMKey, rstDBSvrCfg.m_iProcID) < 0 )
    {
        return false;
    }

    m_pstConfig = &rstDBSvrCfg;
    m_ullLastCreateUinTime =  CGameTime::Instance().GetCurrSecond();
    return true;
}


void AccountSvrMsgLayer::_RegistServerMsg()
{
    /*REGISTER_MSG_HANDLER( SS_MSG_ACCOUNT_LOGIN_REQ,                     CSsAccLoginReq,       m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_ACCOUNT_GM_UPDATE_BANTIME_REQ,         CSsGmUpdateBanTimeReq,       m_oSsMsgHandlerMap );*/
}

int AccountSvrMsgLayer::DealPkg()
{
    int iRecvBytes = 0;
    int iDealPkgNum = 0;
    int i = 0;

    for (; i<DEAL_PKG_PER_LOOP; i++ )
    {
        iRecvBytes = m_oCommBusLayer.Recv();
        if( iRecvBytes < 0 )
        {
            LOGERR_r("bus recv error: %s", m_oCommBusLayer.GetErrMsg());
            return -1;
        }
        if( 0 == iRecvBytes )
        {
            break;
        }

        MyTdrBuf* pstTdrBuf  = m_oCommBusLayer.GetRecvTdrBuf();
        this->_ForwardToWorkThread( pstTdrBuf );

        iDealPkgNum++;
    }

    /*处理work threads的response*/
    AccountSvr& roAccountSvr = AccountSvr::Instance();
    CDBWorkThread* poWorkThread = NULL;
    CThreadQueue* poRspQueue = NULL;
    int iRet = 0;
    uint32_t dwPkgLen = 0;

    for( int i=0; i < m_pstConfig->m_iWorkThreadNum; i++ )
    {
        poWorkThread = roAccountSvr.GetWorkThread(i);
        poRspQueue = poWorkThread->GetRspQueue();
        iRet = poRspQueue->PeekMsg( &m_stSsSendBuf.m_szTdrBuf, &dwPkgLen );
        m_stSsSendBuf.m_uPackLen = dwPkgLen;
        if( iRet != THREAD_Q_SUCESS )
        {
            continue;
        }

        if( !m_stSsSendBuf.m_szTdrBuf )
        {
            continue;
        }

        m_oCommBusLayer.Send( m_pstConfig->m_iProxySvrID, m_stSsSendBuf );
        poRspQueue->PopMsg( );
        iDealPkgNum++;
    }

    return iDealPkgNum;
}


IMsgBase* AccountSvrMsgLayer::GetMsgHandler( int iMsgID )
{
    MsgHandlerMap_t::iterator it = m_oSsMsgHandlerMap.find( iMsgID );
    return ( it == m_oSsMsgHandlerMap.end()) ? NULL : it->second;
}


void AccountSvrMsgLayer::_ForwardToWorkThread( MyTdrBuf* pstTdrBuf )
{
    assert( pstTdrBuf );

    int iThreadIdx = -1;
    CDBWorkThread* poWorkThread = NULL;
    AccountSvr& roAccountSvr = AccountSvr::Instance();
    int iRet = 0;

    /* select work thread */
    if( SS_MSG_ACCOUNT_LOGIN_REQ == SSPKG_MSGID( pstTdrBuf->m_szTdrBuf ) )
    {

        iRet = m_stSsRecvPkg.unpack( pstTdrBuf->m_szTdrBuf, pstTdrBuf->m_uPackLen );
        if( iRet != TdrError::TDR_NO_ERROR)
        {
            LOGERR_r("unpack account login req failed!");
            return;
        }
        uint32_t dwHash = ::zend_inline_hash_func( m_stSsRecvPkg.m_stBody.m_stAccountLoginReq.m_szAccountName,
                                                   strlen( m_stSsRecvPkg.m_stBody.m_stAccountLoginReq.m_szAccountName ) );
        uint64_t ullNewUin = this->_CreateUin(m_stSsRecvPkg.m_stBody.m_stAccountLoginReq.m_dwLoginSvrId);
        //LOGRUN_r("NewUin <%lu> ", ullNewUin);
        uint64_t* pullNewUin = ((uint64_t*) &(((PKGMETA::SSPKG*)(pstTdrBuf->m_szTdrBuf))->m_stHead.m_ullUin));
        TDR_SET_INT_NET( pullNewUin, 8, ullNewUin) ;

        iThreadIdx = int(dwHash % m_pstConfig->m_iWorkThreadNum);
    }
    else
    {
        iThreadIdx = (int)( SSPKG_HEAD_UIN( pstTdrBuf->m_szTdrBuf ) % m_pstConfig->m_iWorkThreadNum );
    }

    if (iThreadIdx < 0)
    {
        LOGERR_r("forward thread error. iThreadIdx: %d", iThreadIdx);
        return;
    }

    poWorkThread = roAccountSvr.GetWorkThread(iThreadIdx);
    if (!poWorkThread)
    {
        LOGERR_r("get thread error. iThreadIdx: %d", iThreadIdx);
        return;
    }

    iRet = poWorkThread->GetReqQueue()->WriteMsg( pstTdrBuf->m_szTdrBuf, pstTdrBuf->m_uPackLen );
    if( iRet != THREAD_Q_SUCESS )
    {
        LOGERR_r("Write msg to worker %d failed! errno <%d>", iThreadIdx, iRet );
        return;
    }
}



uint64_t AccountSvrMsgLayer::_CreateUin(uint32_t dwSvrId)
{
    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();

    if (m_ullLastCreateUinTime != ullCurTime)
    {
        m_ullLastCreateUinTime = ullCurTime;
        m_dwSeq = 0;
    }
    else
    {
        if (++m_dwSeq >= MAX_NUM_CREATE_ONE_SEC)
        {
            return 0;
        }
    }

    //ID生成算法
    // 26位(时间戳,单位:秒)  |  8位(当前时间戳内的序号)   |  10位(服务器ID)

    uint64_t ullTimestamp = m_ullLastCreateUinTime - BASE_TIMESTAMP;

    uint64_t ullUin = ullTimestamp << 18;
    ullUin |= m_dwSeq << 10;
    ullUin |= dwSvrId;

    return ullUin;
}


