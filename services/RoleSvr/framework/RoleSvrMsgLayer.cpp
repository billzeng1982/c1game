#include "RoleSvrMsgLayer.h"
#include "hash_func.cpp"
#include "LogMacros.h"
#include "ObjectPool.h"
#include "../RoleSvr.h"
//#include "../logic/RoleTableMsg.h"
//#include "../logic/RoleGmTableMsg.h"

using namespace PKGMETA;

RoleSvrMsgLayer::RoleSvrMsgLayer() : CMsgLayerBase( sizeof(PKGMETA::SCPKG)*2 , sizeof(PKGMETA::SSPKG)*2 )
{
    m_pstConfig = NULL;
}

bool RoleSvrMsgLayer::Init()
{
    ROLESVRCFG& rstDBSvrCfg =  RoleSvr::Instance().GetConfig();
    if( _Init(rstDBSvrCfg.m_iBusGCIMKey, rstDBSvrCfg.m_iProcID) < 0 )
    {
        return false;
    }

    m_pstConfig = &rstDBSvrCfg;

    return true;
}


void RoleSvrMsgLayer::_RegistServerMsg()
{
    // 放到各个处理线程里去
    /*REGISTER_MSG_HANDLER( SS_MSG_MAJESTY_REGISTER_REQ,          CSsMajestyRegReq,           m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_ROLE_CREATE_REQ,               CSsRoleCreateReq,           m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_ROLE_LOGIN_REQ,                CSsRoleLoginReq,            m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_ROLE_UPDATE_REQ,                CSsRoleUptReq,             m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_ROLE_GM_UPDATE_INFO_REQ,     CSsRoleGmUpdateInfoReq,       m_oSsMsgHandlerMap );*/
}

int RoleSvrMsgLayer::DealPkg()
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
    RoleSvr& roRoleSvr = RoleSvr::Instance();
    CDBWorkThread* poWorkThread = NULL;
    CThreadQueue* poRspQueue = NULL;
    int iRet = 0;
    uint32_t dwPkgLen = 0;

    for( int i=0; i < m_pstConfig->m_iWorkThreadNum; i++ )
    {
        poWorkThread = roRoleSvr.GetWorkThread(i);
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


IMsgBase* RoleSvrMsgLayer::GetMsgHandler( int iMsgID )
{
    MsgHandlerMap_t::iterator it = m_oSsMsgHandlerMap.find( iMsgID );
    return ( it == m_oSsMsgHandlerMap.end()) ? NULL : it->second;
}


void RoleSvrMsgLayer::_ForwardToWorkThread( MyTdrBuf* pstTdrBuf )
{
    assert( pstTdrBuf );
	LOGERR_r("_ForwardToWorkThread  %d", SSPKG_MSGID(pstTdrBuf->m_szTdrBuf));
    int iThreadIdx = -1;
    CDBWorkThread* poWorkThread = NULL;
    RoleSvr& roRoleSvr = RoleSvr::Instance();
    int iRet = 0;

    /* select work thread */
    if( SS_MSG_MAJESTY_REGISTER_REQ == SSPKG_MSGID( pstTdrBuf->m_szTdrBuf ) )
    {
        iThreadIdx = 0;
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

    poWorkThread = roRoleSvr.GetWorkThread(iThreadIdx);
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

