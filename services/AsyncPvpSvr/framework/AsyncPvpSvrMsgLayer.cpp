#include "AsyncPvpSvrMsgLayer.h"
#include "LogMacros.h"
#include "AsyncPvpSvr.h"
#include "MsgLogicAsyncPvp.h"

using namespace PKGMETA;

AsyncPvpSvrMsgLayer::AsyncPvpSvrMsgLayer() : CMsgLayerBase(sizeof(PKGMETA::SCPKG) + 256, sizeof(PKGMETA::SSPKG) + 256)
{
    m_pstConfig = NULL;
}

bool AsyncPvpSvrMsgLayer::Init()
{
    ASYNCPVPSVRCFG& rstDBSvrCfg =  AsyncPvpSvr::Instance().GetConfig();
    if( _Init(rstDBSvrCfg.m_iBusGCIMKey, rstDBSvrCfg.m_iProcID) < 0 )
    {
        return false;
    }

    m_pstConfig = &rstDBSvrCfg;

    return true;
}

void AsyncPvpSvrMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER( SS_MSG_ASYNC_PVP_START_REQ,       AsyncPvpStartReq, m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_ASYNC_PVP_SETTLE_REQ,       AsyncPvpSettleReq, m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_ASYNC_PVP_REFRESH_OPPONENT_REQ,         AsyncPvpRefreshOpponentReq, m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_ASYNC_PVP_GET_DATA_REQ,        AsyncPvpGetDataReq, m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_ASYNC_PVP_UPT_PLAYER_NTF,       AsyncPvpUptPlayerNtf, m_oSsMsgHandlerMap );
	REGISTER_MSG_HANDLER( SS_MSG_ASYNC_PVP_WORSHIPPED_NTF,       AsyncPvpWorshippedNtf, m_oSsMsgHandlerMap );
	REGISTER_MSG_HANDLER( SS_MSG_ASYNC_PVP_GET_WORSHIP_GOLD_REQ, AsyncPvpGetWorshipGold, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER( SS_MSG_ASYNC_PVP_GET_PLAYER_INFO_REQ, AsyncPvpGetPlayerInfo, m_oSsMsgHandlerMap);
}

int AsyncPvpSvrMsgLayer::DealPkg()
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

        this->_DealSvrPkg();
        iDealPkgNum++;
    }

    return iDealPkgNum;
}

IMsgBase* AsyncPvpSvrMsgLayer::GetMsgHandler( int iMsgID )
{
    MsgHandlerMap_t::iterator it = m_oSsMsgHandlerMap.find( iMsgID );
    return ( it == m_oSsMsgHandlerMap.end()) ? NULL : it->second;
}

int AsyncPvpSvrMsgLayer::SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(m_pstConfig->m_iZoneSvrID, rstSsPkg);
}

int AsyncPvpSvrMsgLayer::SendToMailSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(m_pstConfig->m_iMailSvrID, rstSsPkg);
}

