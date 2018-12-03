#include "LogMacros.h"
#include "../MatchSvr.h"
#include "ObjectPool.h"
#include "MatchSvrMsgLayer.h"
#include "../MatchSvr.h"
#include "../msglogic/MsgLogicMatch.h"

using namespace PKGMETA;

MatchSvrMsgLayer::MatchSvrMsgLayer() : CMsgLayerBase(sizeof(PKGMETA::SCPKG) + 256, sizeof(PKGMETA::SSPKG) + 256)
{

}

void MatchSvrMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER(SS_MSG_MATCH_START_REQ,        MatchStartReq_SS,        m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MATCH_CANCEL_REQ,       MatchCancelReq_SS,       m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MACTH_FIGHT_SERV_NTF,   MatchFightSvrNtf_SS,     m_oSsMsgHandlerMap);
}

bool MatchSvrMsgLayer::Init()
{
    MATCHSVRCFG& rConfig = MatchSvr::Instance().GetConfig();
    if (!this->_Init(rConfig.m_iBusGCIMKey, rConfig.m_iProcID)) 
    {
        return false;
    }
    
    return true;
}

int MatchSvrMsgLayer::DealPkg()
{
    int iRecvBytes = 0;
    int i = 0;
    
    for (; i < DEAL_PKG_PER_LOOP; i++) 
    {
        iRecvBytes = m_oCommBusLayer.Recv();
        if (iRecvBytes < 0) 
        {
            LOGERR("bus recv error!");
            return -1;
        }

        if (0 == iRecvBytes) 
        {
            break;
        }

        this->_DealSvrPkg();
    }

    return i;
}

int MatchSvrMsgLayer::SendToClusterGate(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(MatchSvr::Instance().GetConfig().m_iClusterGateID, rstSsPkg);
}

int MatchSvrMsgLayer::SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg)
{
	rstSsPkg.m_stHead.m_ullReservId |= (uint64_t)PROC_ZONE_SVR << 32;
	return SendToServer(MatchSvr::Instance().GetConfig().m_iClusterGateID, rstSsPkg);
}

int MatchSvrMsgLayer::SendToFightSvr(PKGMETA::SSPKG& rstSsPkg)
{
	rstSsPkg.m_stHead.m_ullReservId |= (uint64_t)PROC_FIGHT_SVR << 32;
	return SendToServer(MatchSvr::Instance().GetConfig().m_iClusterGateID, rstSsPkg);
}

