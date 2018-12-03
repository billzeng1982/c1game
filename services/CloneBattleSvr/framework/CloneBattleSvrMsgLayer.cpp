#include "CloneBattleSvrMsgLayer.h"
#include "LogMacros.h"
#include "../CloneBattleSvr.h"
#include "../msglogic/MsgLogicCloneBattle.h"

using namespace PKGMETA;

CloneBattleSvrMsgLayer::CloneBattleSvrMsgLayer() : CMsgLayerBase_c(sizeof(PKGMETA::SSPKG) + 256)
{
    m_pstConfig = NULL;
}

bool CloneBattleSvrMsgLayer::Init()
{
    CLONEBATTLESVRCFG& rstDBSvrCfg = CloneBattleSvr::Instance().GetConfig();
    if (_Init(rstDBSvrCfg.m_iBusGCIMKey, rstDBSvrCfg.m_iProcID) < 0)
    {
        return false;
    }

    m_pstConfig = &rstDBSvrCfg;

    return true;
}

void CloneBattleSvrMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER_c(SS_MSG_CLONE_BATTLE_GET_INFO_REQ,            CSsCloneBattleGetInfoReq,       m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_c(SS_MSG_CLONE_BATTLE_FIGHT_REQ,               CSsCloneBattleFightReq,         m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_c(SS_MSG_CLONE_BATTLE_JOIN_TEAM_REQ,           CSsCloneBattleJoinTeamReq,      m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_c(SS_MSG_CLONE_BATTLE_SET_TEAM_REQ,            CSsCloneBattleSetTeamReq,       m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_c(SS_MSG_CLONE_BATTLE_QUIT_TEAM_REQ,           CSsCloneBattleQuitTeamReq,      m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_c(SS_MSG_CLONE_BATTLE_GET_DATA_RSP,            CSsCloneBattleGetDataRsp,       m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_c(SS_MSG_CLONE_BATTLE_REWARD_NTF,              CSsCloneBattleRewardNtf,        m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_c(SS_MSG_CLONE_BATTLE_ZONESVR_ONLINE_NTF,      CSsCloneBattleZoneSvrOnlineNtf,        m_oSsMsgHandlerMap);
}

int CloneBattleSvrMsgLayer::DealPkg()
{
    int iRecvBytes = 0;
    int iDealPkgNum = 0;
    int i = 0;

    for (; i<DEAL_PKG_PER_LOOP; i++ )
    {
        iRecvBytes = m_oCommBusLayer.Recv();
        if( iRecvBytes < 0 )
        {
            LOGERR_r("bus recv error %s", m_oCommBusLayer.GetErrMsg());
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

int CloneBattleSvrMsgLayer::SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(CloneBattleSvr::Instance().GetConfig().m_iZoneSvrID, rstSsPkg);
}



int CloneBattleSvrMsgLayer::SendToMiscSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(CloneBattleSvr::Instance().GetConfig().m_iMiscSvrID, rstSsPkg);
}

