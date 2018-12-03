#include "LogMacros.h"
#include "ObjectPool.h"
#include "ZoneSvrMsgLayer.h"
#include "CpuSampleStats.h"
#include "define.h"
#include "../ZoneSvr.h"
#include "../msglogic/MsgLogicLogin.h"
#include "../msglogic/MsgLogicMatch.h"
#include "../msglogic/MsgLogicSystem.h"
#include "../msglogic/MsgLogicTask.h"
#include "../module/player/Player.h"
#include "../module/player/PlayerMgr.h"
#include "../module/player/PlayerLogic.h"
#include "../msglogic/MsgLogicMasterSkill.h"
#include "../msglogic/MsgLogicPve.h"
#include "../msglogic/MsgLogicGuild.h"
#include "../msglogic/MsgLogicMessage.h"
#include "../msglogic/MsgLogicMail.h"
#include "../msglogic/MsgLogicFriend.h"
#include "../msglogic/MsgLogicLevelRecord.h"
#include "../msglogic/MsgLogicAsyncPvp.h"
#include "../msglogic/MsgLogicCloneBattle.h"
#include "../msglogic/MsgLogicMine.h"
#include "../msglogic/MsgLogicGuildExpedition.h"
using namespace PKGMETA;

ZoneSvrMsgLayer::ZoneSvrMsgLayer() : CMsgLayerBase(sizeof(PKGMETA::SCPKG) + 256, sizeof(PKGMETA::SSPKG) + 256)
{
    m_iConnID = 0;
}

void ZoneSvrMsgLayer::_RegistClientMsg()
{
    REGISTER_MSG_HANDLER(CS_MSG_ACCOUNT_LOGIN_REQ,          AccountLoginReq_CS,        m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_RECONNECT_REQ,              ReconnectReq_CS,           m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_ACCOUNT_LOGOUT,             AccountLogoutReq_CS,    m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_ROLE_LOGIN_REQ,             RoleLoginReq_CS,        m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_MATCH_START_REQ,            MatchStart_CS,          m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_MATCH_CANCEL_REQ,           MatchCancel_CS,         m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_GCARD_STAR_UP_REQ,			GCardStarUp_CS,			m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GCARD_PHASE_UP_REQ,         GCardPhaseUp_CS,        m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GCARD_COMPOSITE_REQ,        GCardComposite_CS,      m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GCARD_LVUP_REQ,             GCardLvUp_CS,           m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GCARD_LVPHASE_UP_REQ,		GCardLvPhaseUp_CS,      m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GCARD_SKILL_LVUP_REQ,		GCardSkillLvUp_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GCARD_ARMY_LVUP_REQ,		GCardArmyLvUpReq_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GCARD_ARMY_PHASEUP_REQ,		GCardArmyPhaseUpReq_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GCARD_REBORN_REQ,			GCardReborn_CS,             m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GCARD_EQUIP_STAR_DOWN_REQ,  GCardEquipStarDownReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GCARD_EQUIP_STAR_REBORN_REQ, GCardEquipRebornReq_CS, m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_GCARD_TRAIN_REQ,			GCardFeedTrain_CS,          m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_TRAIN_LVUP_REQ,				GCardTrainLvUp_CS,          m_oCsMsgHandlerMap);

	REGISTER_MSG_HANDLER(CS_MSG_TRANS_UNIVERSAL_FARG_REQ,   TransUniversalFragReq_CS,   m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_EQUIP_LV_UP_REQ,            EquipLvUp_CS,           m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_EQUIP_PHASE_UP_REQ,         EquipPhaseUp_CS,        m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_EQUIP_UP_STAR_REQ,			EquipUpStar_CS,			m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_MASTER_SKILL_SELECT_REQ,    MasterSkillSelect_CS,   m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_CHG_BATTLE_ARRAY_FLAG_REQ,	ChgBattleArrayFlag_CS,	m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_CHG_GENERAL_SKILL_FLAG_REQ,	ChgGeneralSkillFlag_CS,	m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_MAJESTY_REGISTER_REQ,       MajestyRegReq_CS,       m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_TASK_DRAW_REQ,				TaskDraw_CS,			m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_TASK_FINAL_AWARD_REQ,		TaskFinalAward_CS,			m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_TASK_MODIFY_REQ,            TaskModify_CS,          m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_MS_UPGRADE_REQ,				MsgLogicMSUpgrade,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_MS_COMPOSITE_REQ,		    MsgLogicMSComposite,	m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_LOTTERY_DRAW_REQ,           LotteryDraw_CS,         m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_LOTTERY_INFO_REQ,           LotteryInfo_CS,         m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_FAKE_SETTLE_REQ,	    PvpFakeSettle_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_PVE_SETTLE_REQ,       PveSettle_CS,			m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_PVE_ENTER_REQ,		PveEnterDungeon_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_DW_LOG,		                CltDwLog_CS       ,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_TUTORIAL_DATA_SYN,		    TutorialDataSyn_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_TUTORIAL_BONUS_REQ,		    TutorialBonusReq_CS,    m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_TUTORIAL_LOTTERY_REQ,		TutorialLotteryReq_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_TUTORIAL_GCARD_LVUP_REQ,	TutorialGCardLvUpReq_CS,  m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_TUTORIAL_EQUIP_LVUP_REQ,	TutorialEquipLvUpReq_CS,  m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_TUTORIAL_GCARD_COMPOSITE_REQ,TutorialGCardCompositeReq_CS,  m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_PVE_SKIP_FIGHT_REQ,	        PveSkipFight_CS,	    m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_PVE_CHAPTER_REWARD_REQ,	    PveChapterReward_CS,	m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_PVE_PURCHASE_TIMES_REQ,	    PvePurchaseTimes_CS,	m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_LEVEL_RECORD_REQ,     PveFightLevelRecord_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_PVE_GET_TREASURE_REQ,       PveGetTreasure_CS, m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_CHEATS_LVUP_REQ,			CheatsLvUp_CS,			m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_CHEATS_CHG_REQ,	            CheatsChg_CS,	        m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_PURCHASE_REQ,	            Purchase_CS,            m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_PROP_SYNTHETIC_REQ,	        PropSyntheticReq_CS,    m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_RANK_GET_TOPLIST_REQ,	    GetTopList_CS,	        m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_RANK_COMMON_GET_TOPLIST_REQ,	RankCommonGetTopListReq_CS,	m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_COMMON_REWARD_REQ,	        CommonRewardRes_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_CHG_GENERAL_LIST_REQ,	    ChgGeneralList_CS,	    m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_MAJESTY_ITEM_CHANGE_REQ,	MajestyChgImageId_CS,   m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_OPEN_BOX_REQ,		        OpenBox_CS,		        m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_OPEN_CHOSEN_BOX_REQ,        OpenChosenBox_CS,       m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_PROP_DECOMPOSITION_REQ,		PropDecomposition_CS,	m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_UPLOAD_REPLAY_REQ,		    UploadReplay_CS,        m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_REFRESH_REPLAYLIST_REQ,		RefreshReplay_CS,	    m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_CREATE_GUILD_REQ,		    CreateGuildReq_CS,	    m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_DISSOLVE_GUILD_REQ,         DissolveGuildReq_CS,    m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_JOIN_GUILD_REQ,		        JoinGuildReq_CS,	    m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_QUIT_GUILD_REQ,             QuitGuildReq_CS,        m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_GUILD_SET_NEED_APPLY_REQ,   GuildSetNeedApplyReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_REFRESH_GUILDLIST_REQ,      RefreshGuildListReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_DETAILS_REQ,          GetGuildDetailReq_CS,   m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_DEAL_APPLICATION_REQ, GuildDealApplyReq_CS,   m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_UPDATE_GUILD_NOTICE_REQ,    UpdateGulidNotice_CS,   m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_SET_GUILD_MEM_JOB_REQ,         SetGuildMemJob_CS,   m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_KICK_PLAYER_REQ,          GuildKickPlayer_CS,  m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_SEARCH_GUILD_REQ,               SearchGuildReq_CS,   m_oCsMsgHandlerMap);
    //REGISTER_MSG_HANDLER(CS_MSG_GUILD_MARKET_PURCHASE_REQ,           GuildPurchaseReq_CS,      m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_LEVELUP_REQ,                   GuildLvUpReq_CS,          m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_GET_GUILDTASK_REQ,             GuildGetTask_CS,          m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_DONATE_REQ,                    GuildDonateReq_CS,        m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_GET_GUILD_RANK_REQ,            GuildGetGuildRankReq_CS,  m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_GET_GFIGHT_RANK_REQ,           GuildGetGFightRankReq_CS,  m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_RESET_MARKET_OR_TASK_REQ,          GuildResetMarketOrTaskReq_CS,  m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_ACTIVITY_REWARD_REQ,        GuildActivityReq_CS,  m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_BOSS_RESET_REQ,        GuildBossResetReq_CS,  m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_BOSS_GET_KILLED_AWARD_REQ,        GuildBossGetKilledAwardReq_CS,  m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_GUILD_SET_LEVEL_LIMIT_REQ,           GuildSetLevelLimitReq_CS,  m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_PVP_JOIN_ROOM_REQ,          PvpJoinRoomReq_CS,      m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_PVP_QUIT_ROOM_REQ,           PvpQuitRoomReq_CS,  m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_PVP_CREATE_ROOM_REQ,         PvpCreateRoomReq_CS,  m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_GUILD_DRAW_SALARY_REQ,		GuildDrawSalaryReq_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_BOSS_COMPETITOR_INFO_REQ, GuildBossCompetitorInfoReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_BOSS_PASSED_GUILD_LIST_REQ, GuildBossPassedGuildList_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_BOSS_GET_SUB_RWD_MEM_LIST_REQ, GuildBossGetSubRwdMemListReq_CS, m_oCsMsgHandlerMap);

	REGISTER_MSG_HANDLER(CS_MSG_GUILD_HANG_SETTLE_REQ,				GuildHangSettleReq_CS,			m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_GUILD_HANG_PURCHASE_SLOT_REQ,		GuildHangPurchaseSlotReq_CS,	m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_GUILD_HANG_LAY_GENERAL_REQ,			GuildHangLayGeneralReq_CS,		m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_GUILD_HANG_SPEED_PARTNER_REQ,		GuildHangSpeedPartnerReq_CS,	m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_HANG_STAR_REQ,		        GuildHangStarReq_CS,	m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_GUILD_FIGHT_APPLY_REQ,					GuildFightApplyReq_CS,				m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_FIGHT_GET_APPLY_LIST_REQ,			GuildFightGetApplyListReq_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_FIGHT_GET_AGAINST_INFO_REQ,		GuildFightGetAgainstInfoReq_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_FIGHT_ARENA_JOIN_REQ,				GuildFightArenaJoinReq_CS,			m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_FIGHT_ARENA_QUIT_REQ,				GuildFightArenaQuitReq_CS,			m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_FIGHT_ARENA_MOVE_REQ,             GuildFightArenaMoveReq_CS,          m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_FIGHT_QUIT_MATCH_REQ,             GuildFightQuitMatchReq_CS,          m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_FIGHT_CANCLE_GOD_REQ,             GuildFightCancleGodReq_CS,          m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_SIGN7D_CLICK_REQ,           Sign7dClick_CS,         m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_SIGN30D_CLICK_REQ,          Sign30dClick_CS,         m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_MESSAGE_SEND_REQ,           MessageSendReq_CS,      m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_MESSAGE_GET_BOX_REQ,        MessageGetBoxReq_CS,    m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_MESSAGE_DEL_BOX_REQ,        MessageDelBoxReq_CS,    m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_MAIL_SEND_REQ,              MailSendReq_CS,         m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_MAIL_DELETE_NTF,            MailDeleteNtf_CS,       m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_MAIL_OPEN_NTF,              MailOpenNtf_CS,         m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_MAIL_DRAW_ATTACHMENT_REQ,   MailDrawAttachmentReq_CS, m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_FRIEND_HANDLE_REQ,          FriendHandleReq_CS,     m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FRIEND_GET_LIST_REQ,        FriendGetListReq_CS,    m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FRIEND_SEARCH_REQ,          FriendSearchReq_CS,     m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GET_PLAYER_BRIEF_INFO_REQ,  GetPlayerBriefInfoReq_CS, m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_FRIEND_SEND_AP_REQ,         FriendSendApReq_CS,     m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_FRIEND_GET_AP_REQ,          FriendGetApReq_CS,      m_oCsMsgHandlerMap);


    REGISTER_MSG_HANDLER(CS_MSG_GEM_UP_REQ,                 GemUpReq_CS,            m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GEM_DOWN_REQ,               GemDownReq_CS,          m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GEM_SYNTHETIC_REQ,          GemSyntheticReq_CS,     m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_GROWTH_AWARD_REQ,			GrowthAwardHandleReq_CS,m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_BUY_LV_FUND_REQ, BuyLvFundReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GET_LV_FUND_AWARD_REQ, GetLvFundAwardReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_SERIAL_NUM_REQ,			    SerialNumReq_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GM_REQ,						GmReq_CS,		        m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_AFFICHE_GET_REQ,			AfficheGetReq_CS,		m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_RECV_VIP_DAILY_GIF_REQ,		RecvVipDailyGifReq_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_RECV_VIP_LEVEL_GIF_REQ,		RecvVipLevelGifReq_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_MALL_BUY_REQ,				MallBuyReq_CS,				m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_WEEK_LEAGUE_APPLY_REQ,	      WeekLeagueApplyReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_WEEK_LEAGUE_RECV_REWADR_REQ,		  WeekLeagueRecvRewardReq_CS, m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_DAILY_AP_REQ,				DailyAp_CS,					m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_PAY_REQ,				    PayReq_CS,					m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_PAY_GET_AWARD_REQ,			PayGetAwardReq_CS,		    m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_TEST_PAY_REQ,			    TestPayReq_CS,				m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_DISCOUNT_PROPS_GET_REQ,			DiscountGetReq_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_DISCOUNT_PROPS_BUY_REQ,			DiscountBuyReq_CS,		m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_ASYNC_PVP_START_REQ, AsyncpvpStartReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_ASYNC_PVP_SETTLE_REQ,		 AsyncpvpSettleReq_CS,	m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_ASYNC_PVP_REFRESH_OPPONENT_REQ, 		 AsyncpvpRefreshOpponentReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_ASYNC_PVP_GET_DATA_REQ,		AsyncpvpGetDataReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_ASYNC_PVP_CHG_TEAM_REQ,		    AsyncpvpChgTeamReq_CS,  m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_ASYNC_PVP_SCORE_REWARD_REQ,		    AsyncpvpScoreRewardReq_CS,  m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_ASYNC_PVP_RANK_REWARD_REQ,		    AsyncpvpRankRewardReq_CS,  m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_ASYNC_PVP_FIGHT_TIMES_BUY_REQ,	    AsyncpvpFightTimesBuyReq_CS,  m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_ASYNC_PVP_GET_TOPLIST_REQ,	      AsyncpvpGetTopListReq_CS,  m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_ASYNC_PVP_WORSHIP_REQ,            AsyncpvpWorshipReq_CS,	m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_ASYNC_PVP_GET_WORSHIP_GOLD_REQ,   AsyncpvpGetWorshipGoldReq_CS,	m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_ASYNC_PVP_RESET_CD_REQ,		AsyncpvpResetCdReq_CS,	m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_ASYNC_PVP_SKIP_FIGHT_REQ,   AsyncpvpSkipFightReq_CS,	m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_ASYNC_PVP_GET_PLAYER_INFO_REQ,		AsyncpvpGetPlayerInfoReq_CS,	m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_DAILY_CHALLENGE_GET_TOPLIST_REQ, DailyChallengeGetTopListReq_CS,      m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_DAILY_CHALLENGE_RECV_REWADR_REQ,          DailyChallengeRecvRewardReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_DAILY_CHALLENGE_SKIP_FIGHT_REQ,        DailyChallengeShipFightReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_DAILY_CHALLENGE_BUY_BUFF_REQ,          DailyChallengeBuyBuffReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_DAILY_CHA_SELECT_SIEGE_EQUIP_REQ,      DailyChallengeSelectSiegeEquipReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_DAILY_CHA_GET_ENEMY_INFO_REQ,          DailyChallengeGetEnemyInfoReq_CS, m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_COIN_SHOP_PURCHASE_REQ,     CoinShopPurchaseReq_CS,        m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_COIN_SHOP_UPDATE_REQ,       CoinShopUpdateReq_CS,          m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_PLAYER_INFO_REQ,       PlayerInfoReq_CS,  m_oCsMsgHandlerMap);

	REGISTER_MSG_HANDLER(CS_MSG_WRITE_SIGNATURE_REQ,	WriteSignature_CS,	m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_READ_SIGNATURE_REQ,		ReadSignature_CS,		m_oCsMsgHandlerMap);

	REGISTER_MSG_HANDLER(CS_MSG_WRITE_NAME_CHANGE_REQ,	WriteNameChange_CS,		m_oCsMsgHandlerMap);

	REGISTER_MSG_HANDLER(CS_MSG_SP_PURCHASE_REQ,	SPPurchaseReq_CS,	m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_CLONE_BATTLE_GET_INFO_REQ,  CloneBattleGetInfoReq_CS,  m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_CLONE_BATTLE_FIGHT_REQ,     CloneBattleFightReq_CS,    m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_CLONE_BATTLE_JOIN_TEAM_REQ, CloneBattleJoinTeamReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_CLONE_BATTLE_REWARD_REQ,    CloneBattleRewardReq_CS,   m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_CLONE_BATTLE_QUIT_TEAM_REQ, CloneBattleQuitTeamReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_CLONE_BATTLE_SET_TEAM_REQ, CloneBattleSetTeamReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_BOSS_SINGLE_REWARD_REQ, GuildBossSingleRewardReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_BOSS_DAMAGE_RANK_IN_GUILD_REQ, GuildBossDamageRankInGuildReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_GUILD_BOSS_MEMBER_FIGHT_TIMES_REQ, GuildBossFightTimesReq_CS, m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_PEAK_ARENA_GET_ACTIVE_REWARD_REQ,PeakArenaGetActiveRewardReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_PEAK_ARENA_GET_OUTPUT_REQ, PeakArenaGetOutputReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_PEAK_ARENA_GET_RULE_REQ,    PeakArenaGetRuleReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_PEAK_ARENA_SPEED_UP_OUTPUT_REQ, PeakArenaSpeedUpOutputReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_TACTICS_ADD_REQ,    TacticsAddReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_TACTICS_LV_UP_REQ,  TacticsLvUpReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_TACTICS_SELECT_REQ, TacticsSelectReq_CS, m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_CHG_SKIN_REQ, ChgSkinReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_BUY_SKIN_REQ, BuySkinReq_CS, m_oCsMsgHandlerMap);

    REGISTER_MSG_HANDLER(CS_MSG_MINE_GET_INFO_REQ, MineGetInfoReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_MINE_EXPLORE_REQ, MineExploreReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_MINE_DEAL_ORE_REQ, MineDealOreReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_MINE_GET_AWARD_REQ, MineGetAwardReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_MINE_BUY_COUNT_REQ, MineBuyCountReq_CS, m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_MINE_GET_REVENGER_INFO_REQ, MineGetRevengerInfoReq_CS, m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_MINE_FIGHT_RESULT_REQ, MineFightResultReq_CS, m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_PEAK_ARENA_BUY_REWARD_TIMES_REQ, PeakArenaBuyTimesReq_CS, m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_GUILD_EXPEDITION_GET_ALL_INFO_REQ, GuildExpeditionGetAllInfoReq_CS, m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_GUILD_EXPEDITION_SET_BATTLE_ARRAY_REQ, GuildExpeditionSetBattleArrayReq_CS, m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_GUILD_EXPEDITION_FIGHT_REQUEST_REQ, GuildExpeditionGetFightRequestReq_CS, m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_GUILD_EXPEDITION_FIGHT_RESULT_REQ, GuildExpeditionGetFightResultReq_CS, m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_GUILD_EXPEDITION_MATCH_REQ, GuildExpeditionMatchReq_CS, m_oCsMsgHandlerMap);


}

void ZoneSvrMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER(SS_MSG_SDK_ACCOUNT_LOGIN_RSP,        SDKAccountLoginRsp_SS,     m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MAJESTY_REGISTER_RSP,         MajestyRegRsp_SS,    m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_ACCOUNT_LOGIN_RSP,          AccountLoginRsp_SS,        m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_ROLE_CREATE_RSP,            RoleCreateRsp_SS,          m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_ROLE_LOGIN_RSP,             RoleLoginRsp_SS,        m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_ROLE_UPDATE_RSP,            RoleUpdateRsp_SS,       m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_DUNGEON_CREATE_RSP,         DungeonCreateRsp_SS,    m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MATCH_CANCEL_RSP,           MatchCancelRsp_SS,      m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_FIGHT_SETTLE_NTF,           FightSettleNtf_SS,      m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_RANK_COMMON_GET_TOPLIST_RSP,    RankCommonGetTopListRsp_SS,    m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_RANK_COMMON_GET_RANK_RSP,    RankCommonGetRankRsp_SS,    m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_UPLOAD_REPLAY_RSP,	        UploadReplay_SS,        m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_REFRESH_REPLAYLIST_NTF,	    RefreshReplayNtf_SS,    m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_CREATE_GUILD_RSP,	        CreateGuildRsp_SS,      m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_DISSOLVE_GUILD_RSP,	        DissolveGuildRsp_SS,    m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_QUIT_GUILD_RSP,	            QuitGuildRsp_SS,        m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_REFRESH_GUILDLIST_RSP,      RefreshGuildListRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_DETAILS_RSP,          GetGuildDetailRsp_SS,   m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_BROADCAST_NTF,        GuildBroadCastNtf_SS,   m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_DEAL_APPLICATION_RSP, GuildDealApplyRsp_SS,   m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_JOIN_GUILD_RSP,             JoinGuildRsp_SS,        m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_GUILD_SET_NEED_APPLY_RSP,   GuildSetNeedApplyRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_UPDATE_GUILD_NOTICE_RSP,    UpdateGulidNotice_SS,   m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_SET_GUILD_MEM_JOB_RSP,      SetGuildMemJob_SS,      m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_KICK_PLAYER_RSP,      GuildKickPlayerRsp_SS,  m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_SEARCH_GUILD_RSP,           SearchGuildRsp_SS,      m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_LEVELUP_RSP,          GuildLvUpRsp_SS,        m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_GET_GUILD_RANK_RSP,   GuildGetGuildRankRsp_SS,        m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_GET_GFIGHT_RANK_RSP,   GuildGetGFightRankRsp_SS,        m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_BOSS_ENTER_FIGHT_RSP,   GuildBossEnterFightRsp_SS,        m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_GUILD_SET_LEVEL_LIMIT_RSP,  GuildSetLevelLimitRsp_SS,       m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_GUILD_DRAW_SALARY_RSP,		GuildDrawSalaryRsp_SS	,m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_BOSS_COMPETITOR_INFO_RSP, GuildBossCompetitorInfoRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_BOSS_DAMAGE_RANK_IN_GUILD_RSP, GuildBossDamageRankInGuildRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_GET_MEMBER_LIST_RSP,      GuildGetMemberListRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_BOSS_PASSED_GUILD_LIST_RSP, GuildBossGetPassedListRsp_SS, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_SPEED_PARTNER_RSP,			GuildSpeedPartnerRsp_SS,		m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_GET_BE_SPEED_INFO_RSP,      GuildGetBeSpeededInfoRsp_SS,	m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_HANG_STAR_RSP,        GuildHangStarRsp_SS,	m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_BOSS_GET_MEM_FIGHT_TIMES_RSP, GuildBossGetMemFightTimesRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_BOSS_GET_MEM_WHO_GET_SUB_RWD_RSP, GuildBossGetMemWhoGetSubRwdRsp_SS, m_oSsMsgHandlerMap);



    REGISTER_MSG_HANDLER(SS_MSG_GUILD_FIGHT_MSG_BROADCAST,              GuildFightMsgBroad_SS,          m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_FIGHT_APPLY_RSP,				    GuildFightApplyRsp_SS,			m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_FIGHT_APPLY_LIST_NTF,			    GuildFightApplyListNtf_SS,		m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_FIGHT_AGAINST_INFO_NTF,	    	GuildFightAgainstNtf_SS,		m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_FIGHT_ARENA_JOIN_RSP,			    GuildFightArenaJoinRsp_SS,		m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_FIGHT_ARENA_QUIT_RSP,			    GuildFightArenaQuitRsp_SS,		m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_FIGHT_ARENA_MOVE_RSP,             GuildFightArenaMoveRsp_SS,      m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_FIGHT_PVP_MATCH_REQ,				GuildFightPvpMatchReq_SS,		m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_FIGHT_SETTLE_NTF,                 GuildFightSettleNtf_SS,         m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_FIGHT_QUIT_MATCH_RSP,             GuildFightQuitMatchRsp_SS,         m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_GUILD_FIGHT_CANCLE_GOD_RSP,             GuildFightCancleGodRsp_SS,          m_oSsMsgHandlerMap);

    REGISTER_MSG_HANDLER(SS_MSG_MESSAGE_SEND_RSP,              MessageSendRsp_SS,      m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MESSAGE_GET_BOX_RSP,           MessageGetBoxRsp_SS,    m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MESSAGE_DEL_BOX_RSP,           MessageDelBoxRsp_SS,    m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MESSAGE_AUTO_SEND_NTF,            MessageAutoSendNtf_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MAIL_SYNC_SERVER_DATA_NTF,  MailSyncServerDataNtf_SS,   m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MAIL_SYNC_PLAYER_DATA_NTF,  MailSyncPlayerDataNtf_SS,   m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MAIL_STAT_NTF,                   MailStatNtf_SS,        m_oSsMsgHandlerMap);

    REGISTER_MSG_HANDLER(SS_MSG_CHECK_SERIAL_NUM_RSP,          CheckSerialNum_SS,        m_oSsMsgHandlerMap);

    REGISTER_MSG_HANDLER(SS_MSG_FRIEND_HANDLE_RSP,          FriendHandleRsp_SS,     m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_FRIEND_GET_LIST_RSP,        FriendGetListRsp_SS,    m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_FRIEND_EVENT_NTF,           FriendEventNtf_SS,      m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_FRIEND_SEARCH_RSP, FriendSearchRsp_SS, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_FRIEND_SEND_AP_RSP,         FriendSendApRsp_SS,      m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_FRIEND_GET_AP_RSP,          FriendGetApRsp_SS,      m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_FRIEND_CHANGE_NAME_RSP,		FriendNameChangeRsp_SS, m_oSsMsgHandlerMap);

    REGISTER_MSG_HANDLER(SS_MSG_ROLE_GM_UPDATE_INFO_RSP,          GmRoleUpdateInfoRsp_SS,     m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_SDK_GET_ORDERID_RSP,              SdkGetOrderRsp_SS,          m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_SDK_PAY_CB_NTF,              SdkPayCbNtf_SS,          m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_LEVEL_RECORD_READ_RSP,      LevelRecordReadRsp_SS,  m_oSsMsgHandlerMap);

    REGISTER_MSG_HANDLER(SS_MSG_DAILY_CHALLENGE_GET_TOPLIST_RSP, DailyChallengeGetTopListRsp_SS,     m_oSsMsgHandlerMap);

    REGISTER_MSG_HANDLER(SS_MSG_ASYNC_PVP_START_RSP,                    AsyncpvpStartRsp_SS,        m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_ASYNC_PVP_SETTLE_RSP,                   AsyncpvpSettleRsp_SS,       m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_ASYNC_PVP_REFRESH_OPPONENT_RSP,         AsyncpvpRefreshOpponentRsp_SS, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_ASYNC_PVP_GET_DATA_RSP,                 AsyncpvpGetDataRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_ASYNC_PVP_REFRESH_TOPLIST_NTF,          AsyncpvpRefreshTopListNtf_SS, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_ASYNC_PVP_GET_WORSHIP_GOLD_RSP,         AsyncpvpGetWorshipGoldRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_ASYNC_PVP_GET_PLAYER_INFO_RSP,         AsyncpvpGetPlayerInfoRsp_SS, m_oSsMsgHandlerMap);

    REGISTER_MSG_HANDLER(SS_MSG_CLONE_BATTLE_GET_INFO_RSP,              CloneBattleGetInfoRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_CLONE_BATTLE_BROADCAST_NTF,             CloneBattleBroadcastNtf_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_CLONE_BATTLE_FIGHT_RSP,                 CloneBattleFightRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_CLONE_BATTLE_JOIN_TEAM_RSP,             CloneBattleJoinTeamRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_CLONE_BATTLE_SET_TEAM_RSP,              CloneBattleSetTeamRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_CLONE_BATTLE_QUIT_TEAM_RSP,             CloneBattleQuitTeamRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_CLONE_BATTLE_UPT_SYSINFO_NTF,           CloneBattleUptSysInfoNtf_SS, m_oSsMsgHandlerMap);

	REGISTER_MSG_HANDLER(SS_MSG_NAME_CHANGE_QUERY_RSP,       NameChangeQueryRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MINE_GET_INFO_RSP, MineGetInfoRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MINE_EXPLORE_RSP, MineExploreRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MINE_DEAL_ORE_RSP, MineDealOreRsp_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MINE_GET_AWARD_RSP, MineGetAwardRsp_SS, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_MINE_GET_REVENGER_INFO_RSP, MineGetRevengerInfoRsp_SS, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_MINE_FIGHT_RESULT_RSP, MineFightResultRsp_SS, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_MINE_SEASON_SETTLE_NTF, MineSeasonSettleNtf_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MINE_INFO_NTF, MineInfoNtf_SS, m_oSsMsgHandlerMap);

    REGISTER_MSG_HANDLER(SS_MSG_GM_REQ,                     GmReq_SS,               m_oSsMsgHandlerMap);

    REGISTER_MSG_HANDLER(SS_MSG_FIGHT_PLAYER_CHEAT_NTF,     FightPlayerCheatNtf_SS, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_GUILD_EXPEDITION_GET_ALL_INFO_RSP, GuildExpeditionGetAllInfoRsp_SS, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_GUILD_EXPEDITION_FIGHT_REQUEST_RSP, GuildExpeditionGetFightRequestRsp_SS, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_GUILD_EXPEDITION_FIGHT_RESULT_RSP, GuildExpeditionGetFightResultRsp_SS, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_GUILD_EXPEDITION_MATCH_RSP, GuildExpeditionMatchRsp_SS, m_oSsMsgHandlerMap);

	REGISTER_MSG_HANDLER(SS_MSG_ROLE_WEB_TASK_INFO_REQ, RoleWebTaskReq_SS, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_WEB_TASK_GET_RWD_REQ, RoleWebTaskRwdReq_SS, m_oSsMsgHandlerMap);
}

bool ZoneSvrMsgLayer::Init()
{
    ZONESVRCFG& rConfig = ZoneSvr::Instance().GetConfig();
    if (!this->_Init(rConfig.m_iBusGCIMKey, rConfig.m_iProcID))
    {
        return false;
    }
    m_iConnID = rConfig.m_iConnID;

    return true;
}

int ZoneSvrMsgLayer::DealPkg()
{
    ZONESVRCFG& rConfig = ZoneSvr::Instance().GetConfig();

    int iSrc = 0;
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

        iSrc = m_oCommBusLayer.GetRecvMsgSrc();
        if (rConfig.m_iConnID == iSrc)
        {
            //处理客户端消息CS_PKG
            this->_DealConnPkg();
        }
        else
        {
            //处理其他Svr消息SS_PKG
            this->_DealSvrPkg();
        }
    }

    return i;
}

void ZoneSvrMsgLayer::_DealConnPkg()
{
    MyTdrBuf* pstTdrBuf = m_oCommBusLayer.GetRecvTdrBuf();

    PKGMETA::CONNSESSION stSession;
    size_t uUsedLen = 0;
    Player* poPlayer = NULL;

    TdrError::ErrorType iRet = stSession.unpack(pstTdrBuf->m_szTdrBuf, pstTdrBuf->m_uPackLen, &uUsedLen);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("Unpack ConnSession failed!");
        return;
    }

    if (PKGMETA::CONNSESSION_CMD_STOP == stSession.m_chCmd)
    {
        poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(stSession.m_dwSessionId);
        if (poPlayer)
        {
            PlayerLogic::Instance().OnSessionStop(poPlayer, stSession.m_stCmdData.m_stConnSessStop.m_chStopReason);
        }
    }

    if (pstTdrBuf->m_uPackLen > uUsedLen)
    {
        char* pszPkg = pstTdrBuf->m_szTdrBuf + uUsedLen;
        TdrError::ErrorType iRet = m_stCsRecvPkg.unpack((const char*) pszPkg, pstTdrBuf->m_uPackLen - uUsedLen);
        if (iRet != TdrError::TDR_NO_ERROR)
        {
            LOGERR("Unpack pkg failed, Ret=%d", iRet);
            return;
        }

        IMsgBase* poMsgHandler = this->GetClientMsgHandler(m_stCsRecvPkg.m_stHead.m_wMsgId);
        if (!poMsgHandler)
        {
            LOGERR("Can not find msg handler. id <%u>", m_stCsRecvPkg.m_stHead.m_wMsgId);
            return;
        }

        if (PKGMETA::CONNSESSION_CMD_START == stSession.m_chCmd)
        {
            CpuSampleStats::Instance().BeginSample("ZoneSvrMsgLayer::_DealConnPkg::%hu", m_stCsRecvPkg.m_stHead.m_wMsgId);
            poMsgHandler->HandleClientMsg(&stSession, m_stCsRecvPkg);
            CpuSampleStats::Instance().EndSample();

        }
        else if (PKGMETA::CONNSESSION_CMD_INPROC == stSession.m_chCmd)
        {
            poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(stSession.m_dwSessionId);
            if (!poPlayer)
            {
                this->SendToClient(&stSession, NULL, PKGMETA::CONNSESSION_CMD_STOP);
            }
            else
            {
                /*LOGWARN("Recv Player(%s) Uin(%lu) pkg, Seq=%u, MsgId=%u", poPlayer->GetRoleName(), poPlayer->GetUin(),
                          m_stCsRecvPkg.m_stHead.m_dwSeqNum, m_stCsRecvPkg.m_stHead.m_wMsgId);*/

                poPlayer->m_dwRecvClientPkgSeq = m_stCsRecvPkg.m_stHead.m_dwSeqNum;

                CpuSampleStats::Instance().BeginSample("ZoneSvrMsgLayer::_DealConnPkg::%hu", m_stCsRecvPkg.m_stHead.m_wMsgId);
                poMsgHandler->HandleClientMsg(&stSession, m_stCsRecvPkg, poPlayer);
                CpuSampleStats::Instance().EndSample();
            }
        }
        else
        {
            assert(false);
        }
    }
}

int ZoneSvrMsgLayer::SendToClient(Player* poPlayer, PKGMETA::SCPKG* pstScPkg, char cSessCmd)
{
    assert(poPlayer);

    //先Pack Session部分
    size_t uUsedSize = 0;
    m_stScSendBuf.Reset();
    TdrError::ErrorType iRet = poPlayer->GetConnSession()->pack(m_stScSendBuf.m_szTdrBuf, m_stScSendBuf.m_uSize, &uUsedSize);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack CONNSESSION error! Ret=%d, SessionCmd=%c", iRet, cSessCmd);
        return -1;
    }
    m_stScSendBuf.m_uPackLen = uUsedSize;

    //再Pack ScPkg
    if( pstScPkg )
    {
        //设置当前包序号
        pstScPkg->m_stHead.m_dwSeqNum = poPlayer->GetPkgSeqNo();

        char* pszBuffer = m_stScSendBuf.m_szTdrBuf + uUsedSize;
        iRet = pstScPkg->pack(pszBuffer, m_stScSendBuf.m_uSize-uUsedSize, &uUsedSize, poPlayer->GetProtocolVersion());
        if( iRet != TdrError::TDR_NO_ERROR)
        {
            LOGERR_r("pack ScPkg error! MsgId=%u, iRet-%d", pstScPkg->m_stHead.m_wMsgId, iRet);
            return -1;
        }
        m_stScSendBuf.m_uPackLen += uUsedSize;

        //将发送的包缓存起来
        poPlayer->SaveSendPkg(pszBuffer, uUsedSize);

        //包序号每次加1
        poPlayer->SetPkgSeqNo(poPlayer->GetPkgSeqNo()+1);
    }

    return m_oCommBusLayer.Send(m_iConnID, m_stScSendBuf);
}

int ZoneSvrMsgLayer::SendCachePkgToClient(Player* poPlayer, char* pszBuffer,  uint32_t dwLen)
{
    assert(poPlayer);

    //先Pack Session部分
    size_t uUsedSize = 0;
    m_stScSendBuf.Reset();
    TdrError::ErrorType iRet = poPlayer->GetConnSession()->pack(m_stScSendBuf.m_szTdrBuf, m_stScSendBuf.m_uSize, &uUsedSize);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack CONNSESSION error! Ret=%d", iRet);
        return -1;
    }
    m_stScSendBuf.m_uPackLen = uUsedSize;

    memcpy(m_stScSendBuf.m_szTdrBuf + uUsedSize, pszBuffer, dwLen);

    m_stScSendBuf.m_uPackLen += dwLen;

    poPlayer->SaveSendPkg(pszBuffer, dwLen);

    return m_oCommBusLayer.Send(m_iConnID, m_stScSendBuf);
}

int ZoneSvrMsgLayer::SendToClient(const PKGMETA::CONNSESSION* pstConnSess, PKGMETA::SCPKG* pstScPkg, char cSessCmd, uint16_t wVersion)
{
    if (pstScPkg)
    {
        pstScPkg->m_stHead.m_dwSeqNum = 0;
    }

    return this->_SendToClient(m_iConnID, pstConnSess, pstScPkg, cSessCmd, wVersion);
}

// 这儿组播使用时最好注意限制下，人数不要太多
int ZoneSvrMsgLayer::MultiCastToClient(uint64_t UinList[], int iUinCnt, PKGMETA::SCPKG* pstScPkg, char cSessCmd)
{
    for (int i=0; i<iUinCnt; i++)
    {
        Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(UinList[i]);
        if (poPlayer != NULL)
        {
            this->SendToClient(poPlayer, pstScPkg, cSessCmd);
        }
        else
        {
            LOGERR("MultiCastToClient : Player Uin(%lu) is not exist.", UinList[i]);
        }
    }

    return 0;
}

int ZoneSvrMsgLayer::BroadcastToClient(PKGMETA::SCPKG* pstScPkg)
{
    if (pstScPkg)
    {
        pstScPkg->m_stHead.m_dwSeqNum = 0;
    }

    PKGMETA::CONNSESSION stSendSess;
    bzero(&stSendSess, sizeof(stSendSess));

    stSendSess.m_chCmd = CONNSESSION_CMD_BROADCAST;
    stSendSess.m_chConnType = PROTO_TYPE_TCP;

    return this->_SendToClient(m_iConnID, &stSendSess, pstScPkg, CONNSESSION_CMD_BROADCAST, 0);
}

int ZoneSvrMsgLayer::SendToAccountSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iAccountSvrID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToRoleSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iRoleSvrID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToRankSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iRankSvrID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToGuildSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iGuildSvrID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToFriendSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iFriendSvrID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToMessageSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iMessageSvrID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToMailSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iMailSvrID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToClusterGate(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iClusterGateID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToReplaySvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iReplaySvrID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToXiYouSDKSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iXiYouSDKSvrID, rstSsPkg);
}
int ZoneSvrMsgLayer::SendToSdkDMSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iSdkDMSvrID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToSerialNumSvr(PKGMETA::SSPKG& rstSsPkg)
{
	rstSsPkg.m_stHead.m_ullReservId |= (uint64_t)PROC_SERIAL_NUM_SVR << 32;
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iClusterGateID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToMiscSvr(PKGMETA::SSPKG& rstSsPkg)
{
	return SendToServer(ZoneSvr::Instance().GetConfig().m_iMiscID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToAsyncPvpSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iAsyncPvpSvrID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToCloneBattleSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iCloneBattleSvrID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToIdipAgentSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iIdipAgentSvrID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToMineSvr(PKGMETA::SSPKG& rstSsPkg)
{
	//构造 符合 ReservId
	rstSsPkg.m_stHead.m_ullReservId |=  (uint64_t) PROC_MINE_SVR << 32;
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iClusterGateID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToMatchSvr(PKGMETA::SSPKG& rstSsPkg)
{
	//构造 符合 ReservId
	rstSsPkg.m_stHead.m_ullReservId |= (uint64_t)PROC_MATCH_SVR << 32;
	return SendToServer(ZoneSvr::Instance().GetConfig().m_iClusterGateID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToFightSvr(PKGMETA::SSPKG& rstSsPkg)
{
	//构造 符合 ReservId
	rstSsPkg.m_stHead.m_ullReservId |= (uint64_t)PROC_FIGHT_SVR << 32;
	return SendToServer(ZoneSvr::Instance().GetConfig().m_iClusterGateID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToClusterAccSvr(PKGMETA::SSPKG& rstSsPkg)
{
    //构造 符合 ReservId
    rstSsPkg.m_stHead.m_ullReservId |= (uint64_t)PROC_CLUSTER_ACCOUNT_SVR << 32;
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iClusterGateID, rstSsPkg);
}

//m_ullReservId 字段需要在外面需要置0
int ZoneSvrMsgLayer::SendToGuildExpeditionSvr(PKGMETA::SSPKG& rstSsPkg)
{
	//构造 符合 ReservId
	rstSsPkg.m_stHead.m_ullReservId |= (uint64_t)PROC_GUILD_EXPEDITION_SVR << 32;
	return SendToServer(ZoneSvr::Instance().GetConfig().m_iClusterGateID, rstSsPkg);
}

int ZoneSvrMsgLayer::SendToZoneHttpConn(PKGMETA::SSPKG & rstSsPkg)
{
    return SendToServer(ZoneSvr::Instance().GetConfig().m_iZoneHttpConnID, rstSsPkg);
}
