#include "GuildSvrMsgLayer.h"
#include "LogMacros.h"
#include "ObjectPool.h"
#include "hash_func.cpp"
#include "../GuildSvr.h"
#include "../msglogic/MsgLogicGuild.h"
#include "../msglogic/MsgLogicGFight.h"

using namespace PKGMETA;

GuildSvrMsgLayer::GuildSvrMsgLayer() : CMsgLayerBase( sizeof(PKGMETA::SCPKG)*2 , sizeof(PKGMETA::SSPKG)*2 )
{
    m_pstConfig = NULL;
}

bool GuildSvrMsgLayer::Init()
{
    GUILDSVRCFG& rstDBSvrCfg =  GuildSvr::Instance().GetConfig();
    if( _Init(rstDBSvrCfg.m_iBusGCIMKey, rstDBSvrCfg.m_iProcID) < 0 )
    {
        return false;
    }

    m_pstConfig = &rstDBSvrCfg;

    return true;
}


void GuildSvrMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_DETAILS_REQ,                 GuildDetailsReq_SS,        m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_REFRESH_GUILDLIST_REQ,             RefreshGuildListReq_SS,    m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_CREATE_GUILD_REQ,                  CreateGuildReq_SS,         m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_JOIN_GUILD_REQ,                    JoinGuildReq_SS,           m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_SET_NEED_APPLY_REQ,          GuildSetNeedApplyReq_SS,   m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_DEAL_APPLICATION_REQ,        GuildDealApplyReq_SS,      m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_DISSOLVE_GUILD_REQ,                DissolveGuildReq_SS,       m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_QUIT_GUILD_REQ,                    QuitGuildReq_SS,           m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_REFRESH_MEMBERINFO_NTF,            RefreshMemInfoNtf_SS,      m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_LEVELUP_REQ,                 GuildLvUpReq_SS,           m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_UPDATE_GUILD_NOTICE_REQ,           UpdateGulidNotice_SS,      m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_UPDATE_FUND_NTF,             GulidUptFundNtf_SS,        m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_SEARCH_GUILD_REQ,                  SearchGuildReq_SS,         m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_SET_GUILD_MEM_JOB_REQ,             SetGuildMemJob_SS,         m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_UPDATE_GUILD_NOTICE_REQ,           UpdateGulidNotice_SS,	   m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_UPDATE_GUILD_NOTICE_REQ,           UpdateGulidNotice_SS,      m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_GET_GUILD_RANK_REQ,          GetGuildRankReq_SS,        m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_GET_GFIGHT_RANK_REQ,          GetGFightRankReq_SS,      m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_REFRESH_GUILD_PVP_ROOM_NTF,         RefreshRoomNtf_SS,        m_oSsMsgHandlerMap );
	REGISTER_MSG_HANDLER( SS_MSG_GUILD_SET_LEVEL_LIMIT_REQ,         GuildSetLevelLimitReq_SS,  m_oSsMsgHandlerMap );
	REGISTER_MSG_HANDLER( SS_MSG_GUILD_LEVEL_EVENT_NTF,				GuildLevelEventNtf_SS,		m_oSsMsgHandlerMap );
	REGISTER_MSG_HANDLER( SS_MSG_GUILD_DRAW_SALARY_REQ,				GuildDrawSalaryReq_SS,		m_oSsMsgHandlerMap );

    REGISTER_MSG_HANDLER( SS_MSG_GUILD_FIGHT_APPLY_REQ,				GFightApplyReq_SS,		    m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_FIGHT_ARENA_JOIN_REQ,		GFightArenaJoinReq_SS,		m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_FIGHT_ARENA_QUIT_REQ,		GFightArenaQuitReq_SS,		m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_FIGHT_ARENA_MOVE_REQ,        GFightArenaMoveReq_SS,      m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_FIGHT_QUIT_MATCH_REQ,        GFightQuitMatchReq_SS,      m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_FIGHT_CANCLE_GOD_REQ,        GFightCancleGodReq_SS,      m_oSsMsgHandlerMap );

    REGISTER_MSG_HANDLER( SS_MSG_GUILD_FIGHT_PVP_MATCH_RSP,			GFightPvpMatchRsp_SS,		m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_FIGHT_PVP_SETTLE_NTF,        GFightPvpSettle_SS,			m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_FIGHT_PVP_CREATE_DUNGEON_RSP,GFightCreateDungeonRsp_SS,  m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_MESSAGE_SEND_REQ,            GuildMessageSendReq_SS,     m_oSsMsgHandlerMap );

    REGISTER_MSG_HANDLER( SS_MSG_GUILD_GM_UPDATE_INFO_REQ,           GuildGmUpdateInfoReq_SS,    m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_KICK_PLAYER_REQ,              GuildKickPlayerReq_SS,      m_oSsMsgHandlerMap );

    REGISTER_MSG_HANDLER( SS_MSG_UPLOAD_REPLAY_REQ,                 GuildUploadReplayReq_SS,      m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_BOSS_GET_INFO_REQ,           GulidBossGetInfoReq_SS,      m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_BOSS_RESET_REQ,              GulidBossResetReq_SS,        m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_BOSS_FIGHT_SETTLE_NTF,       GulidBossFightSettleNtf_SS,        m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_GUILD_BOSS_ENTER_FIGHT_REQ,        GuildBossEnterFightReq_SS,        m_oSsMsgHandlerMap );
    //REGISTER_MSG_HANDLER(SS_MSG_RANK_GUILD_COMPETITOR_REQ, GuildCompetitorInfoReq_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_BOSS_DAMAGE_RANK_IN_GUILD_REQ,GuildBossDamageRankInGuildReq_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_GET_MEMBER_LIST_REQ,          GuildGetMemberListReq_SS,     m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_BOSS_PASSED_GUILD_LIST_REQ, GuildBossGetPassedListReq_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_BOSS_GET_MEM_FIGHT_TIMES_REQ, GuildBossGetMemFightTimesReq_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_SET_BOSS_FIGHT_TIMES_TO_ZERO_NTF, GuildBossSetBossFightTimesToZeroNtf_SS, m_oSsMsgHandlerMap);
    //REGISTER_MSG_HANDLER(SS_MSG_GUILD_BOSS_GET_RANK_LIST_RSP, GuildBossGetRankListRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_BOSS_GET_MEM_WHO_GET_SUB_RWD_REQ, GuildBossGetMemWhoGetSubRwdReq_SS, m_oSsMsgHandlerMap);

	REGISTER_MSG_HANDLER(SS_MSG_SPEED_PARTNER_REQ,					GuildSpeedPartnerReq_SS,     m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_GET_BE_SPEED_INFO_REQ,				GuildGetBeSpeededInfoReq_SS, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_GUILD_HANG_STAR_REQ,				GuildHangStarReq_SS, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_GUILD_EXPEDITION_COMMON_NTF,		GuildExpeditionCommonNtf_SS, m_oSsMsgHandlerMap);
}
	


int GuildSvrMsgLayer::DealPkg()
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

int GuildSvrMsgLayer::SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(m_pstConfig->m_iZoneSvrID, rstSsPkg);
}

int GuildSvrMsgLayer::SendToMailSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(m_pstConfig->m_iMailSvrID, rstSsPkg);
}


int GuildSvrMsgLayer::SendToRankSvr(PKGMETA::SSPKG& rstSsPkg)
{
	return SendToServer(m_pstConfig->m_iRankSvrID, rstSsPkg);
}

int GuildSvrMsgLayer::SendToGuildExpeditionSvr(PKGMETA::SSPKG& rstSsPkg)
{
	rstSsPkg.m_stHead.m_ullReservId |= (uint64_t)PROC_GUILD_EXPEDITION_SVR << 32;
	return SendToServer(m_pstConfig->m_iClusterGateID, rstSsPkg);
}
