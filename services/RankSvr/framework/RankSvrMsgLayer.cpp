
#include "RankSvrMsgLayer.h"
#include "define.h"
#include "hash_func.cpp"
#include "LogMacros.h"
#include "ObjectPool.h"
#include "../RankSvr.h"
#include "../logic/RankMsgLogic.h"

using namespace PKGMETA;

RankSvrMsgLayer::RankSvrMsgLayer() : CMsgLayerBase( sizeof(PKGMETA::SCPKG)*2 , sizeof(PKGMETA::SSPKG)*2 )
{

}

void RankSvrMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER(SS_MSG_6V6_RANK_UPDATE_REQ,                      Rank_6v6UpdateRankReq_SS,       m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_RANK_COMMON_UPDATE_NTF,                   RankCommonUpdateNtf_SS,             m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_RANK_COMMON_GET_TOPLIST_REQ,              RankCommonGetTopListReq_SS,         m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_DAILY_CHALLENGE_UPDATE_RANK_NTF,          DailyChallengeUptRankNtf_SS,     m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_DAILY_CHALLENGE_GET_TOPLIST_REQ,          DailyChallengeGetTopListReq_SS,  m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_DAILY_CHALLENGE_SETTLE_RANK_NTF,          DailyChallengeSettleRankNtf_SS,     m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_DAILY_CHALLENGE_CLEAR_RANK_NTF,           DailyChallengeClearRankNtf_SS,  m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_LI_RANK_SETTLE_NTF,                       LiRankSettleNtf_SS,         m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_RANK_COMMON_GET_RANK_REQ,                 RankCommonGetRankReq_SS,    m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_NAME_CHANGE_NTF,						  NameChangeNtf_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_BOSS_GET_RANK_LIST_REQ,             RankGuildGetRankListReq_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_PEAK_ARENA_RANK_SETTLE_NTF,               PeakArenaSettleNtf_SS, m_oSsMsgHandlerMap);
}

bool RankSvrMsgLayer::Init()
{
    RANKSVRCFG& rConfig = RankSvr::Instance().GetConfig();
    if (!this->_Init(rConfig.m_iBusGCIMKey, rConfig.m_iProcID))
    {
        return false;
    }

    return true;
}

int RankSvrMsgLayer::DealPkg()
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

int RankSvrMsgLayer::SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(RankSvr::Instance().GetConfig().m_iZoneSvrID, rstSsPkg);
}

int RankSvrMsgLayer::SendToMailSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(RankSvr::Instance().GetConfig().m_iMailSvrID, rstSsPkg);
}

int RankSvrMsgLayer::SendToGuildSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(RankSvr::Instance().GetConfig().m_iGuildSvrID, rstSsPkg);
}
