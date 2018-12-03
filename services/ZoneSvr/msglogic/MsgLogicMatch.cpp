#include <string.h>
#include "strutil.h"
#include "ObjectUpdatorMgr.h"
#include "LogMacros.h"
#include "MsgLogicMatch.h"
#include "common_proto.h"
#include "../gamedata/GameDataMgr.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../module/player/PlayerMgr.h"
#include "../module/player/PlayerLogic.h"
#include "../module/FightPVP.h"
#include "../module/Equip.h"
#include "../module/GeneralCard.h"
#include "../module/Consume.h"
#include "../module/Majesty.h"
#include "../module/GeneralCard.h"
#include "../module/Task.h"
#include "../module/MasterSkill.h"
#include "../module/RankMgr.h"
#include "../module/Item.h"
#include "../module/Match.h"
#include "../module/Guild.h"
#include "../module/PvpRoomMgr.h"
#include "ov_res_keywords.h"
#include "hash_func.h"
#include "dwlog_def.h"
#include "../module/ZoneLog.h"
#include "dwlog_svr.h"
#include "../module/Marquee.h"
#include "../module/GloryItemsMgr.h"
#include "../module/DailyChallenge.h"
#include "../module/PeakArena.h"

using namespace PKGMETA;
using namespace DWLOG;

int MatchStart_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_MATCH_START_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stMatchStartReq;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (poPlayer == NULL)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    // settle player
    PlayerData& roPlayerData = poPlayer->GetPlayerData();
    if (roPlayerData.GetRoleBaseInfo().m_llBlackRoomTime > CGameTime::Instance().GetCurrSecond())
    {
        LOGERR("the role is in the back room");
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MATCH_START_RSP;
        SC_PKG_MATCH_START_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stMatchStartRsp;
        rstScPkgBodyRsp.m_nErrNo = ERR_BACK_ROOM;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return ERR_BACK_ROOM;
    }

    //每日挑战赛检查上阵武将合法性
    if (rstCsPkgBodyReq.m_bMatchType == MATCH_TYPE_DAILY_CHALLENGE)
    {
        int iRet = DailyChallenge::Instance().CheckGenerals(&roPlayerData);
        if (iRet != ERR_NONE)
        {
            m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MATCH_START_RSP;
            SC_PKG_MATCH_START_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stMatchStartRsp;
            rstScPkgBodyRsp.m_nErrNo = iRet;
            ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
            return iRet;
        }

    }

    // 开始匹配
    int16_t nErrNo = Match::Instance().Start(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_bMatchType);

    if (nErrNo != ERR_NONE)
    {
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MATCH_START_RSP;
        SC_PKG_MATCH_START_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stMatchStartRsp;
        rstScPkgBodyRsp.m_nErrNo = nErrNo;

        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }

    return 0;
}

int MatchCancel_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    CS_PKG_MATCH_CANCEL_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stMatchCancelReq;
    Match::Instance().Cancel(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_bMatchType);

    return 0;
}

int DungeonCreateRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_DUNGEON_CREATE_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stDungeonCreateRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MATCH_START_RSP;
    SC_PKG_MATCH_START_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stMatchStartRsp;

    // rsp to client
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkgBodyRsp.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer is NULL");
        return 0;
    }

    //取消玩家的匹配状态
    poPlayer->GetPlayerData().m_bMatchState = 0;

    uint8_t bSelfGroup = rstSsPkgBodyRsp.m_stDungeonInfo.m_astFightPlayerList[0].m_ullUin == poPlayer->GetUin() ? 0 : 1;
    //uint8_t bMatchType = rstSsPkgBodyRsp.m_stDungeonInfo.m_astFightPlayerList[bSelfGroup].m_bMatchType;

    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;
    if (rstSsPkgBodyRsp.m_nErrNo != ERR_NONE)
    {
        LOGRUN("Player(%s) Uin(%lu) dungeon create failed.", poPlayer->GetRoleName(), poPlayer->GetUin());
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return -1;
    }
    rstScPkgBodyRsp.m_stDungeonInfo = rstSsPkgBodyRsp.m_stDungeonInfo;
    rstScPkgBodyRsp.m_iFightSvrIp = rstSsPkgBodyRsp.m_iFightSvrIp;
    rstScPkgBodyRsp.m_wFightSvrPort = rstSsPkgBodyRsp.m_wFightSvrPort;;

    // 记录PVP分组信息
    for (int i=0; i<rstSsPkgBodyRsp.m_stDungeonInfo.m_bFightPlayerNum /*pvp 2players fixed*/; i++)
    {
        if (rstSsPkgBodyRsp.m_stDungeonInfo.m_astFightPlayerList[i].m_ullUin == poPlayer->GetUin())
        {
            poPlayer->GetPlayerData().m_oSelfInfo = rstSsPkgBodyRsp.m_stDungeonInfo.m_astFightPlayerList[i];
        }
        else
        {
            poPlayer->GetPlayerData().m_oOpponentInfo = rstSsPkgBodyRsp.m_stDungeonInfo.m_astFightPlayerList[i];
        }
    }

    PlayerData& roPlayerData = poPlayer->GetPlayerData();
    roPlayerData.m_ullDungeonTimeMs = rstSsPkgBodyRsp.m_stDungeonInfo.m_ullTimeStamp;

    //如果是周末挑战赛假匹配，需要构造相应的AI数据
    if (rstSsPkgBodyRsp.m_stDungeonInfo.m_bFakeType == MATCH_FAKE_OTHER)
    {
        rstScPkgBodyRsp.m_nErrNo = DailyChallenge::Instance().GenFakePlayer(&poPlayer->GetPlayerData(), rstScPkgBodyRsp.m_stDungeonInfo.m_astFightPlayerList[(bSelfGroup + 1) % 2]);
    }

    if (rstSsPkgBodyRsp.m_stDungeonInfo.m_bFakeType == MATCH_FAKE_NONE)
    {
        ZoneLog::Instance().WriteMatchLog(&poPlayer->GetPlayerData(), 2/*匹配到玩家*/, rstSsPkgBodyRsp.m_stDungeonInfo.m_bMatchType);
        poPlayer->SetFightState(FIGHT_STATE_IN_PVP);
    }
    else
    {
        ZoneLog::Instance().WriteMatchLog(&poPlayer->GetPlayerData(), 1/*匹配到AI*/, rstSsPkgBodyRsp.m_stDungeonInfo.m_bMatchType);
        poPlayer->SetFightState(FIGHT_STATE_IN_FAKE_PVP);
    }

    //如果今日已打过每日挑战赛，则用上次的战场副本信息
    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChallengeInfo = roPlayerData.GetELOInfo().m_stDailyChallengeInfo;
    if (rstScPkgBodyRsp.m_stDungeonInfo.m_bMatchType == MATCH_TYPE_DAILY_CHALLENGE)
    {
        if (rstDailyChallengeInfo.m_bIsGotFightDungeonInfo != 0)
        {
            rstScPkgBodyRsp.m_nErrNo = DailyChallenge::Instance().LoadSelfFightPlayerInfo(&poPlayer->GetPlayerData());
            if (rstScPkgBodyRsp.m_nErrNo == ERR_NONE)
            {
                memcpy(&rstScPkgBodyRsp.m_stDungeonInfo, &rstDailyChallengeInfo.m_stFightDungeonInfo, sizeof(DT_FIGHT_DUNGEON_INFO));
            }
        }
        else
        {
            //今日首次，保存战场副本信息
            memcpy(&rstDailyChallengeInfo.m_stFightDungeonInfo, &rstScPkgBodyRsp.m_stDungeonInfo, sizeof(DT_FIGHT_DUNGEON_INFO));
        }
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    LOGRUN("DungeonCreateRsp_SS uin-%lu, name-%s", poPlayer->GetUin(), poPlayer->GetAccountName());

    return 0;
}

int MatchCancelRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_MATCH_CANCEL_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stMatchCancelRsp;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkgBodyRsp.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MATCH_CANCEL_RSP;
    SC_PKG_MATCH_CANCEL_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stMatchCancelRsp;
    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;
    rstScPkgBodyRsp.m_bMatchType = rstSsPkgBodyRsp.m_bMatchType;

    //取消玩家的匹配状态
    poPlayer->GetPlayerData().m_bMatchState = 0;

    if (rstSsPkgBodyRsp.m_nErrNo == ERR_NONE)
    {
	    //记录Log
	    ZoneLog::Instance().WriteMatchLog(&poPlayer->GetPlayerData(), 3/*取消匹配*/);
        LOGRUN("MatchCancelRsp_SS Uin-%lu, Name-%s success", poPlayer->GetUin(), poPlayer->GetAccountName());
    }

    if( ERR_NOT_EXIST_PLAYER == rstSsPkgBodyRsp.m_nErrNo )
    {
        // 检查是否已经进入匹配，如果进入匹配，则返回另外的错误号，客户端不取消
        if( poPlayer->GetFightState() == FIGHT_STATE_IN_PVP )
        {
            rstScPkgBodyRsp.m_nErrNo = ERR_IN_PREPARE_PHASE;
        }
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int FightSettleNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_FIGHT_SETTLE_NTF& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stFightSettleNtf;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkgBodyRsp.m_stMyInfo.m_ullUin);
    if (poPlayer == NULL)
    {
        LOGERR("poPlayer is NULL.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_SETTLE_NTF;
    SC_PKG_FIGHT_SETTLE_NTF& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stFightSettleNtf;

    rstScPkgBodyRsp.m_bMatchType = rstSsPkgBodyRsp.m_bMatchType;
    rstScPkgBodyRsp.m_bResult = (rstSsPkgBodyRsp.m_chGroup == rstSsPkgBodyRsp.m_stMyInfo.m_chGroup) ? 1 : 2;
    rstScPkgBodyRsp.m_ullTimestamp = rstSsPkgBodyRsp.m_ullTimeStamp;
    rstScPkgBodyRsp.m_dwDungeonId = rstSsPkgBodyRsp.m_dwDungeonId;
    rstScPkgBodyRsp.m_bReason = rstSsPkgBodyRsp.m_chReason;

    switch (rstSsPkgBodyRsp.m_bMatchType)
    {
    case MATCH_TYPE_GUILD_FIGHT:
        this->_HandleGFightSettleNtf(rstSsPkg, m_stScPkg, &poPlayer->GetPlayerData());
        break;
    case MATCH_TYPE_WEEKEND_LEAGUE:
        this->_HandleWeekLeagueSettleNtf(rstSsPkg, m_stScPkg, &poPlayer->GetPlayerData());
        break;
    case MATCH_TYPE_LEISURE:
        this->_HandleLeisureSettleNtf(rstSsPkg, m_stScPkg, &poPlayer->GetPlayerData());
        break;
    case MATCH_TYPE_GUILD_PVP:
        this->_HandleRoomFightSettleNtf(rstSsPkg, m_stScPkg, &poPlayer->GetPlayerData());
        break;
    case MATCH_TYPE_DAILY_CHALLENGE:
        this->_HandleDailyChallengeSettleNtf(rstSsPkg, m_stScPkg, &poPlayer->GetPlayerData());
        break;
    case MATCH_TYPE_PEAK_ARENA:
        this->_HandlePeakArenaSettleNtf(rstSsPkg, m_stScPkg, &poPlayer->GetPlayerData());
        break;
    }

    poPlayer->SetFightState(FIGHT_STATE_NONE);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    LOGRUN("Player(%s) Uin(%lu) FightSettleNtf_SS MatchType(%d), Result(%d), DungeonId(%u)",
            poPlayer->GetRoleName(), poPlayer->GetUin(), rstSsPkgBodyRsp.m_bMatchType, rstScPkgBodyRsp.m_bResult, rstSsPkgBodyRsp.m_dwDungeonId);

    return 0;
}

int FightSettleNtf_SS::_HandleFight6v6SettleNtf(PKGMETA::SSPKG& rstSsPkg, PKGMETA::SCPKG& rstScPkg, PlayerData* pstData)
{
    SS_PKG_FIGHT_SETTLE_NTF& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stFightSettleNtf;

    DT_FIGHT6V6_SETTLE_EXTRA_INFO& rstSettleInfo = rstScPkg.m_stBody.m_stFightSettleNtf.m_stExtraInfo.m_stFight6v6Info;

    rstSettleInfo.m_dwRaceNumber = this->_GenRaceNumber(rstSsPkgBodyRsp);

    DT_ROLE_PVP_6V6_INFO& rstPvp6v6Info = pstData->GetELOInfo().m_stPvp6v6Info;

    uint8_t bResult = (rstSsPkgBodyRsp.m_chGroup == rstSsPkgBodyRsp.m_stMyInfo.m_chGroup) ? 1 : 0;
    Fight6V6::Instance().HandleELOSettle(pstData, bResult);

    //添加战斗记录历史
    Fight6V6::Instance().AddFightHistory(pstData, &rstSsPkgBodyRsp.m_stMyInfo, rstSsPkgBodyRsp.m_chGroup, rstSsPkgBodyRsp.m_ullTimeStamp);
    rstSettleInfo.m_stCurPvpInfo = rstPvp6v6Info.m_stBaseInfo;

    //天梯排名流水日志
    ZoneLog::Instance().WritePvPLog(pstData, "Rank", rstPvp6v6Info.m_stBaseInfo.m_wWinCount);

    // 任务
    Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_RANK, rstSettleInfo.m_stCurPvpInfo.m_bELOLvId/*value*/, 1/*段位*/);
    _HandlePvpTask(*pstData, rstSsPkgBodyRsp, Task::CONST_PVP_6V6);
	
    //更新公会成员信息
    Guild::Instance().RefreshMemberInfo(pstData);

	// 更新主公物品开启
	GloryItemsMgr::Instance().AddMajestyItems(pstData, MAJESTY_ITEM_ACCESS_WIN, rstPvp6v6Info.m_stBaseInfo.m_wWinCount);
	GloryItemsMgr::Instance().AddMajestyItems(pstData, MAJESTY_ITEM_ACCESS_LOSE, rstPvp6v6Info.m_stBaseInfo.m_wLoseCount);
	GloryItemsMgr::Instance().AddMajestyItems(pstData, MAJESTY_ITEM_ACCESS_WIN_STREAK, rstPvp6v6Info.m_stBaseInfo.m_bStreakTimes);

	if (rstSsPkgBodyRsp.m_stMyInfo.m_chGroup == rstSsPkgBodyRsp.m_chGroup)
	{
		//匹配结果为胜利
		_StatGeneralInfo(pstData, rstSsPkgBodyRsp);
        rstPvp6v6Info.m_stDailyInfo.m_wWinCount++;
	}

    rstSettleInfo.m_stCurPvpInfo = rstPvp6v6Info.m_stBaseInfo;

    //更新排名
    Fight6V6::Instance().SendRankInfoToRankSvr(pstData, rstSsPkgBodyRsp);

    return ERR_NONE;
}

int FightSettleNtf_SS::_HandleGFightSettleNtf(PKGMETA::SSPKG& rstSsPkg, PKGMETA::SCPKG& rstScPkg, PlayerData* pstData)
{
    SS_PKG_FIGHT_SETTLE_NTF& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stFightSettleNtf;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_PVP_SETTLE_NTF;
    SS_PKG_GUILD_FIGHT_PVP_SETTLE_NTF& rstSsPkgNtf = m_stSsPkg.m_stBody.m_stGuildFightPvpSettleNtf;

    //通知GuildSvr,军团战匹配打完了
    if (rstSsPkgBodyRsp.m_chGroup == PLAYER_GROUP_NONE)
    {
        rstSsPkgNtf.m_ullWinerUin = 0;
    }
    else if (rstSsPkgBodyRsp.m_chGroup == pstData->m_oSelfInfo.m_chGroup)
    {
        rstSsPkgNtf.m_ullWinerUin = rstSsPkgBodyRsp.m_stMyInfo.m_ullUin;
		Player* pstPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkgBodyRsp.m_stMyInfo.m_ullUin);
		if (pstPlayer != NULL)
		{
			//军团战 战斗胜利
			Task::Instance().ModifyData(&pstPlayer->GetPlayerData(), TASK_VALUE_TYPE_PVP, 1, Task::CONST_PVP_GUILD_FIGHT, 1);
		}
		
    }
    else
    {
        rstSsPkgNtf.m_ullWinerUin = rstSsPkgBodyRsp.m_stOpponentInfo.m_ullUin;
    }

    rstSsPkgNtf.m_stPlayerInfo1.m_ullUin = rstSsPkgBodyRsp.m_stMyInfo.m_ullUin;
    rstSsPkgNtf.m_stPlayerInfo1.m_ullGuildId = pstData->GetGuildInfo().m_ullGuildId;
    rstSsPkgNtf.m_stPlayerInfo2.m_ullUin = rstSsPkgBodyRsp.m_stOpponentInfo.m_ullUin;

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);

    return 0;
}

int FightSettleNtf_SS::_HandleWeekLeagueSettleNtf(PKGMETA::SSPKG& rstSsPkg, PKGMETA::SCPKG& rstScPkg, PlayerData* pstData)
{
#if 0
    SS_PKG_FIGHT_SETTLE_NTF& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stFightSettleNtf;
    //结算胜负场
    DT_ROLE_WEEK_LEAGUE_INFO& rstLeagueInfo = pstData->GetELOInfo().m_stWeekLeagueInfo;
    if ( rstSsPkgBodyRsp.m_chGroup == pstData->m_oSelfInfo.m_chGroup)
    {
        rstLeagueInfo.m_bWinCount++;
		if (rstLeagueInfo.m_bWinCount == 12/*周末赛12场全部胜利*/)
		{
			//主公获得称号
			GloryItemsMgr::Instance().AddMajestyItems(pstData, MAJESTY_ITEM_ACCESS_WIN_WEEKLEAGUE, 1);
		}
        rstLeagueInfo.m_ullTotalWinCount++;
    }
    else
    {
        rstLeagueInfo.m_bLoseCount++;
    }

    ZoneLog::Instance().WritePvPLog(pstData, "WeekLeague", rstLeagueInfo.m_ullTotalWinCount);

    _HandlePvpTask(*pstData, rstSsPkgBodyRsp, Task::CONST_PVP_WEKK_LEAGUE);

    pstData->m_pOwner->SetFightState(FIGHT_STATE_NONE);
#endif
    return 0;
}

int FightSettleNtf_SS::_HandleLeisureSettleNtf(PKGMETA::SSPKG& rstSsPkg, PKGMETA::SCPKG& rstScPkg, PlayerData* pstData)
{
    SS_PKG_FIGHT_SETTLE_NTF& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stFightSettleNtf;

    DT_ROLE_ELO_INFO& rstELOInfo = pstData->GetELOInfo();
    if (rstSsPkgBodyRsp.m_chGroup == pstData->m_oSelfInfo.m_chGroup)
    {
        rstELOInfo.m_stPvp6v6Info.m_stDailyInfo.m_wWinCount++;
    }

    _HandlePvpTask(*pstData, rstSsPkgBodyRsp, Task::CONST_PVP_LEISURE);

    ZoneLog::Instance().WritePvPLog(pstData, "Leisure", rstELOInfo.m_stPvp6v6Info.m_stDailyInfo.m_wWinCount);

    return ERR_NONE;
}

int FightSettleNtf_SS::_HandleRoomFightSettleNtf(PKGMETA::SSPKG& rstSsPkg, PKGMETA::SCPKG& rstScPkg, PlayerData* pstData)
{
    DT_GUILD_PVP_SETTLE_EXTRA_INFO& rstSettleInfo = rstScPkg.m_stBody.m_stFightSettleNtf.m_stExtraInfo.m_stGuildPvpInfo;
    rstSettleInfo.m_dwRaceNumber = this->_GenRaceNumber(rstSsPkg.m_stBody.m_stFightSettleNtf);

    if (pstData->m_ullRoomNo != 0)
    {
        //删除房间
        PvpRoomMgr::Instance().DestroyRoom(pstData->m_ullRoomNo);
    }

    return ERR_NONE;
}


int FightSettleNtf_SS::_HandleDailyChallengeSettleNtf(PKGMETA::SSPKG& rstSsPkg, PKGMETA::SCPKG& rstScPkg, PlayerData* pstData)
{
    SS_PKG_FIGHT_SETTLE_NTF& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stFightSettleNtf;

    DT_DAILY_CHALLENGE_SETTLE_EXTRA_INFO& rstSettleInfo = rstScPkg.m_stBody.m_stFightSettleNtf.m_stExtraInfo.m_stDailyChallengeInfo;
    rstSettleInfo.m_stSyncItemInfo.m_bSyncItemCount = 0;

    uint8_t bResult = (rstSsPkgBodyRsp.m_chGroup == rstSsPkgBodyRsp.m_stMyInfo.m_chGroup) ? 1 : 0;
    DailyChallenge::Instance().FightSettle(pstData, bResult, rstSettleInfo.m_stSyncItemInfo);

	DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChallengeInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;
	memcpy(&rstSettleInfo.m_stSelfGeneralsInfo, &rstDailyChallengeInfo.m_stGeneralsInfo, sizeof(DT_DAILY_CHALLENGE_GENERALS_INFO));
	memcpy(&rstSettleInfo.m_stEnemyTroopInfo, &rstDailyChallengeInfo.m_stEnemyTroopInfo, sizeof(DT_DAILY_CHALLENGE_TROOP_INFO));
    memcpy(&rstSettleInfo.m_stBuff, &rstDailyChallengeInfo.m_stBuffs, sizeof(DT_DAILY_CHALLENGE_BUFFS));

    rstSettleInfo.m_dwScore = rstDailyChallengeInfo.m_dwScore;

    _HandlePvpTask(*pstData, rstSsPkgBodyRsp, Task::CONST_PVP_DAILY_CHALLENGE);

    return ERR_NONE;
}


int FightSettleNtf_SS::_HandlePeakArenaSettleNtf(PKGMETA::SSPKG& rstSsPkg, PKGMETA::SCPKG& rstScPkg, PlayerData* pstData)
{
    SS_PKG_FIGHT_SETTLE_NTF& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stFightSettleNtf;

    DT_PEAK_ARENA_SETTLE_EXTRA_INFO& rstSettleInfo = rstScPkg.m_stBody.m_stFightSettleNtf.m_stExtraInfo.m_stPeakArenaInfo;
    rstSettleInfo.m_stSyncItemInfo.m_bSyncItemCount = 0;

    //进行段位，积分等结算
    uint8_t bIsWin = (rstSsPkgBodyRsp.m_chGroup == rstSsPkgBodyRsp.m_stMyInfo.m_chGroup) ? 1 : 0;
    bIsWin = (rstSsPkgBodyRsp.m_chGroup == PLAYER_GROUP_NONE) ? 2 : bIsWin;

    PeakArena::Instance().HandleELOSettle(pstData, bIsWin);
    //进行单局奖励结算
    PeakArena::Instance().HandleRewardSettle(pstData, bIsWin, rstSettleInfo.m_stSyncItemInfo);

    DT_ROLE_PEAK_ARENA_INFO& rstPeakArenaInfo = pstData->GetELOInfo().m_stPeakArenaInfo;
    rstSettleInfo.m_stCurPvpInfo.m_bELOLvId = rstPeakArenaInfo.m_bELOLvId;
    rstSettleInfo.m_stCurPvpInfo.m_dwScore = rstPeakArenaInfo.m_dwScore;
    rstSettleInfo.m_stCurPvpInfo.m_wWinCount = rstPeakArenaInfo.m_wWinCount;
    rstSettleInfo.m_stCurPvpInfo.m_wLoseCount = rstPeakArenaInfo.m_wLoseCount;
    rstSettleInfo.m_stCurPvpInfo.m_bStreakTimes = rstPeakArenaInfo.m_bStreakTimes;

    //更新排名
    PeakArena::Instance().UpdateRank(pstData);

	//  巅峰击杀武将
	Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_KILL, rstSsPkgBodyRsp.m_stDungeonTaskInfo.m_dwKillGeneral, 3, 1, 10);

	_HandlePvpTask(*pstData, rstSsPkgBodyRsp, Task::CONST_PVP_PEAK_ARENA);
    ZoneLog::Instance().WritePvPLog(pstData, "PeakArena", rstSettleInfo.m_stCurPvpInfo.m_wWinCount);

    return ERR_NONE  ;
}

//根据匹配的武将阵容获得称号
void FightSettleNtf_SS::_StatGeneralInfo(PlayerData* pstData, SS_PKG_FIGHT_SETTLE_NTF& rstSsPkgBodyRsp)
{
	uint8_t astGeneralPower[4];
	uint8_t astGeneralGender[2];
	for (int i = 0; i < 4; i++)
	{
		astGeneralPower[i] = 0;
	}
	for (int i = 0; i < 2; i++)
	{
		astGeneralGender[i] = 0;
	}

	for (int i = 0; i < rstSsPkgBodyRsp.m_stMyInfo.m_bTroopNum; i++)
	{
		DT_ITEM_GCARD& rstGeneralCard = rstSsPkgBodyRsp.m_stMyInfo.m_astTroopList[i].m_stGeneralInfo;
		ResGeneralMgr_t& rstGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
		RESGENERAL* poGeneral = rstGeneralMgr.Find(rstGeneralCard.m_dwId);
		if (poGeneral)
		{
			astGeneralGender[poGeneral->m_bSex]++;
			astGeneralPower[poGeneral->m_bCountryId]++;
		}
	}

	DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();

	if (astGeneralPower[1] == MAX_TROOP_NUM_PVP)
	{
		rstMiscInfo.m_wWinOfWeiCnt++;
		GloryItemsMgr::Instance().AddMajestyItems(pstData, MAJESTY_ITEM_ACCESS_WIN_WEI, rstMiscInfo.m_wWinOfWeiCnt);
	}

	if (astGeneralPower[2] == MAX_TROOP_NUM_PVP)
	{
		rstMiscInfo.m_wWinOfShuCnt++;
		GloryItemsMgr::Instance().AddMajestyItems(pstData, MAJESTY_ITEM_ACCESS_WIN_SHU, rstMiscInfo.m_wWinOfShuCnt);
	}

	if (astGeneralPower[3] == MAX_TROOP_NUM_PVP)
	{
		rstMiscInfo.m_wWinOfWuCnt++;
		GloryItemsMgr::Instance().AddMajestyItems(pstData, MAJESTY_ITEM_ACCESS_WIN_SHU, rstMiscInfo.m_wWinOfShuCnt);
	}

	if (astGeneralGender[1] == MAX_TROOP_NUM_PVP)
	{
		rstMiscInfo.m_wWinOfFemaleCnt++;
		GloryItemsMgr::Instance().AddMajestyItems(pstData, MAJESTY_ITEM_ACCESS_WIN_FEMALE, rstMiscInfo.m_wWinOfFemaleCnt);
	}
}

uint32_t FightSettleNtf_SS::_GenRaceNumber(PKGMETA::SS_PKG_FIGHT_SETTLE_NTF& rstNtf)
{
    uint64_t ullUin1, ullUin2, ullTimestamp = 0;
    char szRaceNumber[60];
    if (rstNtf.m_stMyInfo.m_ullUin < rstNtf.m_stOpponentInfo.m_ullUin)
    {
        ullUin1 = rstNtf.m_stMyInfo.m_ullUin;
        ullUin2 = rstNtf.m_stOpponentInfo.m_ullUin;
    }
    else
    {
        ullUin2 = rstNtf.m_stMyInfo.m_ullUin;
        ullUin1 = rstNtf.m_stOpponentInfo.m_ullUin;
    }
    ullTimestamp = rstNtf.m_ullTimeStamp;
    snprintf(szRaceNumber, sizeof(szRaceNumber), "%lu%lu%lu", ullUin1, ullUin2, ullTimestamp);
    return zend_inline_hash_func(szRaceNumber, strlen(szRaceNumber));
}

int FightSettleNtf_SS::_HandlePvpTask(PlayerData& roPlayerData, SS_PKG_FIGHT_SETTLE_NTF& rstSsPkgBodyRsp, uint32_t dwPvpType)
{

    Task::Instance().ModifyData(&roPlayerData, TASK_VALUE_TYPE_PVP, 1/*value*/, dwPvpType, 3/*参与*/);
    Task::Instance().ModifyData(&roPlayerData, TASK_VALUE_TYPE_PVP, 1/*value*/, 3/*PVP*/, 3/*参与*/);
    Task::Instance().ModifyData(&roPlayerData, TASK_VALUE_TYPE_KILL, rstSsPkgBodyRsp.m_stDungeonTaskInfo.m_dwKillGeneral, 1, 1, 10);
    Task::Instance().ModifyData(&roPlayerData, TASK_VALUE_TYPE_KILL, rstSsPkgBodyRsp.m_stDungeonTaskInfo.m_KillTypeList[ARMY_CAVALRY], 1, 1, 1);
    Task::Instance().ModifyData(&roPlayerData, TASK_VALUE_TYPE_KILL, rstSsPkgBodyRsp.m_stDungeonTaskInfo.m_KillTypeList[ARMY_SPEARMAN], 1, 1, 3);
    Task::Instance().ModifyData(&roPlayerData, TASK_VALUE_TYPE_KILL, rstSsPkgBodyRsp.m_stDungeonTaskInfo.m_KillTypeList[ARMY_ARCHER], 1, 1, 2);

    if (rstSsPkgBodyRsp.m_chGroup == roPlayerData.m_oSelfInfo.m_chGroup)
    {
        // 胜利
        Task::Instance().ModifyData(&roPlayerData, TASK_VALUE_TYPE_PVP, 1/*value*/, dwPvpType, 1/*胜利*/);
        Task::Instance().ModifyData(&roPlayerData, TASK_VALUE_TYPE_PVP, 1/*value*/, 3/*PVP*/, 1/*胜利*/);

        if (rstSsPkgBodyRsp.m_stDungeonTaskInfo.m_dwEnemySettleHP == 0)
        {
            Task::Instance().ModifyData(&roPlayerData, TASK_VALUE_TYPE_WINCITY, 1, 1, 2, 2);
        }

        Task::Instance().ModifyData(&roPlayerData, TASK_VALUE_TYPE_WINCITY, rstSsPkgBodyRsp.m_stDungeonTaskInfo.m_bSelfSettleHPPer, 1, 1, 1);
    }

    return 0;
}

int PvpFakeSettle_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGRUN("PvpFakeSettle : player not exist.");
        return -1;
    }
    // 假PVP，用PVE信息构造PVP结算信息
    CS_PKG_FIGHT_FAKE_SETTLE_REQ& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightFakeSettleReq;

	//更新敌我双方战场信息
    if (rstCsPkgBody.m_stMyInfo.m_bMatchType == MATCH_TYPE_DAILY_CHALLENGE)
    {
        uint8_t bResult = (rstCsPkgBody.m_bGroup == rstCsPkgBody.m_stMyInfo.m_chGroup ? 1 : 0);
        DailyChallenge::Instance().UpdateTroopInfo(&poPlayer->GetPlayerData(), bResult, rstCsPkgBody.m_stSelfTroop, rstCsPkgBody.m_stEnemyTroop);
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_FIGHT_SETTLE_NTF;
    SS_PKG_FIGHT_SETTLE_NTF& rstSsPkgBody = m_stSsPkg.m_stBody.m_stFightSettleNtf;

    rstSsPkgBody.m_bMatchType = rstCsPkgBody.m_stMyInfo.m_bMatchType;		// 匹配类型

    rstSsPkgBody.m_chGroup = rstCsPkgBody.m_bGroup;				// 胜利阵营
    rstSsPkgBody.m_chReason = rstCsPkgBody.m_bReason;				// 胜利原因
    rstSsPkgBody.m_ullTimeStamp = rstCsPkgBody.m_ullTimeStamp;		// 时间戳
    rstSsPkgBody.m_stMyInfo = rstCsPkgBody.m_stMyInfo;				// 自己的匹配信息
    rstSsPkgBody.m_stOpponentInfo = rstCsPkgBody.m_stOpponentInfo;	// 对手的匹配信息
    rstSsPkgBody.m_stOpponentInfo.m_bMatchType = rstCsPkgBody.m_stMyInfo.m_bMatchType;

    // 副本任务信息
    rstSsPkgBody.m_stDungeonTaskInfo.m_dwKillGeneral = rstCsPkgBody.m_bKillTroopNum;			// 击杀武将
    rstSsPkgBody.m_stDungeonTaskInfo.m_dwDestoryObstacle = rstCsPkgBody.m_bKillBarrierNum;		// 摧毁障碍物
    rstSsPkgBody.m_stDungeonTaskInfo.m_dwGeneralSkill = rstCsPkgBody.m_bGSkillUseNum;			// 武将技释放
    rstSsPkgBody.m_stDungeonTaskInfo.m_dwMasterSkill = rstCsPkgBody.m_bMSkillUseNum;			// 军师技释放
    memcpy(rstSsPkgBody.m_stDungeonTaskInfo.m_KillTypeList, rstCsPkgBody.m_KillTypeList, sizeof(rstSsPkgBody.m_stDungeonTaskInfo.m_KillTypeList));
    rstSsPkgBody.m_stDungeonTaskInfo.m_dwEnemySettleHP = rstCsPkgBody.m_wCityHpEnemy;
    rstSsPkgBody.m_stDungeonTaskInfo.m_dwSelfSettleHP = rstCsPkgBody.m_wCityHpSelf;
    rstSsPkgBody.m_stDungeonTaskInfo.m_bSelfSettleHPPer = (uint32_t)rstCsPkgBody.m_wCityHpSelf * 100 /rstSsPkgBody.m_stMyInfo.m_dwCityHp;

    LOGRUN("Handle PvpFakeSettle Req, Uin<%lu>, Name<%s>, MatchType<%d>", poPlayer->GetUin(), poPlayer->GetAccountName(), rstSsPkgBody.m_stMyInfo.m_bMatchType);

    m_oSettleNtfHandle.HandleServerMsg(m_stSsPkg);

    return 0;
}


int TestSendMsg()
{
#if 0
    int iRet = 0;
    SSPKG m_stSsPkg;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_RANK_UPDATE_REQ;
    ZoneSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);
    return iRet;
#endif
    return 0;
}
