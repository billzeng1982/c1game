#include "MailDBSvrMsgLayer.h"
#include "hash_func.cpp"
#include "LogMacros.h"
#include "ObjectPool.h"
#include "../MailDBSvr.h"
#include "../logic/MailTableMsg.h"

using namespace PKGMETA;

MailDBSvrMsgLayer::MailDBSvrMsgLayer() : CMsgLayerBase( sizeof(PKGMETA::SCPKG)*2 , sizeof(PKGMETA::SSPKG)*2 )
{
    m_pstConfig = NULL;
}

bool MailDBSvrMsgLayer::Init()
{
    MAILDBSVRCFG& rstDBSvrCfg =  MailDBSvr::Instance().GetConfig();
    if( _Init(rstDBSvrCfg.m_iBusGCIMKey, rstDBSvrCfg.m_iProcID) < 0 )
    {
        return false;
    }

    m_pstConfig = &rstDBSvrCfg;
    
    return true;
}


void MailDBSvrMsgLayer::_RegistServerMsg()
{
    /*
    //私人邮件箱
    REGISTER_MSG_HANDLER( SS_MSG_MAIL_PRI_TABLE_CREATE_REQ,     CSsMailPriTableCreateReq,     m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_MAIL_PRI_TABLE_GET_DATA_REQ,   CSsMailPriTableGetDataReq,    m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_MAIL_PRI_TABLE_UPDATE_NTF,     CSsMailPriTableUptNtf,        m_oSsMsgHandlerMap );

    //公共邮件
    REGISTER_MSG_HANDLER( SS_MSG_MAIL_PUB_TABLE_GET_DATA_REQ,   CSsMailPubTableGetDataReq,    m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_MAIL_PUB_TABLE_DEL_NTF,        CSsMailPubTableDelNtf,        m_oSsMsgHandlerMap ); 
    REGISTER_MSG_HANDLER( SS_MSG_MAIL_PUB_TABLE_ADD_REQ,        CSsMailPubTableAddReq,        m_oSsMsgHandlerMap ); 
    */
}

int MailDBSvrMsgLayer::DealPkg()
{ 
    int iRecvBytes = 0;
    int iDealPkgNum = 0;
    int i = 0;
    
    for (; i<DEAL_PKG_PER_LOOP; i++ )
    {
        iRecvBytes = m_oCommBusLayer.Recv();
        if( iRecvBytes < 0 )
        {
            LOGERR_r("bus recv error!");
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
    CDBWorkThread* poWorkThread = NULL;
    CThreadQueue* poRspQueue = NULL;
    int iRet = 0;
    uint32_t dwPkgLen = 0;
    
    for( int i=0; i < m_pstConfig->m_iWorkThreadNum; i++ )
    {
        poWorkThread = MailDBSvr::Instance().GetWorkThread(i);
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
    
        m_oCommBusLayer.Send( m_pstConfig->m_iMailSvrID, m_stSsSendBuf );
        poRspQueue->PopMsg( ); 
        iDealPkgNum++;
    }

    return iDealPkgNum;
}


IMsgBase* MailDBSvrMsgLayer::GetMsgHandler( int iMsgID )
{
    MsgHandlerMap_t::iterator it = m_oSsMsgHandlerMap.find( iMsgID );
    return ( it == m_oSsMsgHandlerMap.end()) ? NULL : it->second;
}

void MailDBSvrMsgLayer::_ForwardToWorkThread( MyTdrBuf* pstTdrBuf )
{
    assert( pstTdrBuf );

    int iThreadIdx = -1;
    CDBWorkThread* poWorkThread = NULL;
    MailDBSvr& roMailDBSvr = MailDBSvr::Instance();
    int iRet = 0;

    /* select work thread */
    iThreadIdx = (int)( SSPKG_HEAD_UIN( pstTdrBuf->m_szTdrBuf ) % m_pstConfig->m_iWorkThreadNum );
    if (iThreadIdx < 0)
    {
        LOGERR_r("forward thread error. iThreadIdx: %d", iThreadIdx);
        return;
    }
    
    poWorkThread = roMailDBSvr.GetWorkThread(iThreadIdx);
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

int MailDBSvrMsgLayer::SendToMailSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(MailDBSvr::Instance().GetConfig().m_iMailSvrID, rstSsPkg);
}


