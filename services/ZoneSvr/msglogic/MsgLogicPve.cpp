#include "MsgLogicPve.h"
#include "common_proto.h"
#include "dwlog_svr.h"
#include "strutil.h"
#include "ObjectUpdatorMgr.h"
#include "LogMacros.h"
#include "ov_res_public.h"
#include "../gamedata/GameDataMgr.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../module/player/Player.h"
#include "../module/player/PlayerMgr.h"
#include "../module/player/PlayerLogic.h"
#include "../module/GeneralCard.h"
#include "../module/FightPVE.h"
#include "../module/FightStatistics.h"
#include "../module/Task.h"
#include "../module/AP.h"
#include "../module/Props.h"
#include "../module/Consume.h"
#include "../module/ZoneLog.h"
#include "../module/Item.h"
#include "../module/Guild.h"
#include "../module/Task.h"
#include "../module/ActivityMgr.h"
#include "../module/Majesty.h"
#include "../module/LevelRank.h"

using namespace PKGMETA;
using namespace DWLOG;

int PveEnterDungeon_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    CS_PKG_FIGHT_PVE_ENTER_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stFightPveEnterReq;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (poPlayer == NULL)
    {
        LOGERR("poPlayer -> null, sessionid:%d", pstSession->m_dwSessionId);
        return ERR_DEFAULT;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_PVE_ENTER_RSP;
    SC_PKG_FIGHT_PVE_ENTER_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stFightPveEnterRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    int iErrNo = ERR_NONE;
    PlayerData& roPlayerData = poPlayer->GetPlayerData();

    do
    {
        // black room
        if (roPlayerData.GetRoleBaseInfo().m_llBlackRoomTime > CGameTime::Instance().GetCurrSecond())
        {
            LOGERR("the role is in the back room");
            iErrNo = ERR_BACK_ROOM;
            break;
        }

        RESFIGHTLEVEL *pstResLevel = CGameDataMgr::Instance().GetResFightLevelMgr().Find(rstCsPkgBodyReq.m_dwLevelId);
        if (!pstResLevel)
        {
            LOGERR("dwPveLevelId:%d -> not exist.", rstCsPkgBodyReq.m_dwLevelId);
            iErrNo = ERR_NOT_FOUND;
            break;
        }

        // 等级校验
        if (pstResLevel->m_dwRequireMajestyLevel > roPlayerData.GetMajestyInfo().m_wLevel)
        {
            LOGERR("level<%d> is less than require level<%d>.", roPlayerData.GetMajestyInfo().m_wLevel, pstResLevel->m_dwRequireMajestyLevel);
            iErrNo = ERR_MAJESTY_UN_SATISFY;
            break;
        }

        if (!AP::Instance().IsEnough(&roPlayerData, (uint32_t)pstResLevel->m_bAPConsume))
        {
            DT_ITEM& rstItem = rstScPkgBodyRsp.m_stSyncItemInfo.m_astSyncItemList[rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount++];
            rstItem.m_bItemType = ITEM_TYPE_AP;
            rstItem.m_dwValueAfterChg = roPlayerData.GetAP();
            iErrNo = ERR_NOT_ENOUGH_AP;
            break;
        }

        if (pstResLevel->m_bChapterType == CHAPTER_TYPE_ACTIVITY)
        {
            if (pstResLevel->m_bChapterId == 1)
            {
                iErrNo = Majesty::Instance().IsArriveLevel(&roPlayerData, LEVEL_LIMIT_ACTIVITY3);
            }
            else if (pstResLevel->m_bChapterId == 2)
            {
                iErrNo = Majesty::Instance().IsArriveLevel(&roPlayerData, LEVEL_LIMIT_ACTIVITY2);
            }
            else if (pstResLevel->m_bChapterId == 3)
            {
                //iErrNo = Majesty::Instance().IsArriveLevel(&roPlayerData, LEVEL_LIMIT_ACTIVITY1);//策划要求，去掉等级限制
            }

            if (iErrNo != ERR_NONE)
            {
                break;
            }
        }

		if (pstResLevel->m_bChapterType == CHAPTER_TYPE_ACTIVITY)
		{
			uint32_t dwActivityTypeId = pstResLevel->m_bChapterId;
			if(0!=ActivityMgr::Instance().CheckActivityPveCd(&roPlayerData, dwActivityTypeId))
			{
				iErrNo = ActivityMgr::Instance().ResetColdTime(&roPlayerData, dwActivityTypeId, rstScPkgBodyRsp.m_stSyncItemInfo);
				if(iErrNo<0)
				{
					break;
				}
			}
		}

        if (CHAPTER_TYPE_GUILD_BOSS == pstResLevel->m_bChapterType)
        {
            //要走公会拿boss血量,不在这里回复
            SSPKG stSsPkg ;
            stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
            stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_BOSS_ENTER_FIGHT_REQ;
            stSsPkg.m_stBody.m_stGuildBossEnterFightReq.m_dwFLevelId = rstCsPkgBodyReq.m_dwLevelId;
            stSsPkg.m_stBody.m_stGuildBossEnterFightReq.m_ullGuildId = poPlayer->GetPlayerData().GetGuildInfo().m_ullGuildId;
            ZoneSvrMsgLayer::Instance().SendToGuildSvr(stSsPkg);

            //Guild::Instance().FightBoss(&roPlayerData, pstResLevel->m_dwId, rstScPkgBodyRsp);
            return 0;
        }

    } while (false);

    // 是否能够进入 pve
    rstScPkgBodyRsp.m_nErrNo = iErrNo;

    if (iErrNo == ERR_NONE)
    {
        // settle player
        roPlayerData.m_ullPveTimeStamp = (uint64_t)CGameTime::Instance().GetCurrSecond();

        //时间戳, 简单放作弊
        rstScPkgBodyRsp.m_ullTimeStamp = roPlayerData.m_ullPveTimeStamp;
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int PveSettle_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stFightPveSettleReq;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (poPlayer == NULL)
    {
        LOGERR("poPlayer -> null, sessionid:%d", pstSession->m_dwSessionId);
        return ERR_DEFAULT;
    }

    LOGRUN("Handle PveSettle Req, Uin-%lu, Name-%s", poPlayer->GetUin(), poPlayer->GetRoleName());

    PlayerData& roPlayerData = poPlayer->GetPlayerData();

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_PVE_SETTLE_RSP;
    SC_PKG_FIGHT_PVE_SETTLE_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stFightPveSettleRsp;
    rstScPkgBodyRsp.m_dwFLevelID = rstCsPkgBodyReq.m_dwFLevelID;
    rstScPkgBodyRsp.m_bIsPass = rstCsPkgBodyReq.m_bIsPass;
    rstScPkgBodyRsp.m_bPassReason = rstCsPkgBodyReq.m_bPassReason;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

	rstScPkgBodyRsp.m_bStarEvalResult = 0;
    rstScPkgBodyRsp.m_nErrNo = ERR_NONE;

	//武将列表置零
	bzero(rstScPkgBodyRsp.m_GeneralList, sizeof(uint32_t)*MAX_TROOP_NUM_PVP);

    RESFIGHTLEVEL *pstResLevel = CGameDataMgr::Instance().GetResFightLevelMgr().Find(rstCsPkgBodyReq.m_dwFLevelID);
    if (pstResLevel == NULL)
    {
        LOGERR("dwPveLevelId:%d -> not exist.", rstCsPkgBodyReq.m_dwFLevelID);
        rstScPkgBodyRsp.m_nErrNo = ERR_NOT_FOUND;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return ERR_NOT_FOUND;
    }

    int IsFirstPass = 0;
    if (pstResLevel->m_bChapterType == CHAPTER_TYPE_GUILD)
    {
        //通过时记录
        if (rstCsPkgBodyReq.m_bIsPass != 0)
        {
            //公会关卡的结算
            rstScPkgBodyRsp.m_nErrNo = Guild::Instance().SettleGuilTask(&poPlayer->GetPlayerData(), rstCsPkgBodyReq, rstScPkgBodyRsp);
            if (rstScPkgBodyRsp.m_nErrNo == ERR_NONE)
            {
                LOGRUN("dwPveLevelId:%d is guild level.", rstCsPkgBodyReq.m_dwFLevelID);
                Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_GUILD_OTHER, 1, 1);
            }
        }
    }
    else if (pstResLevel->m_bChapterType == CHAPTER_TYPE_ACTIVITY)
    {
        //活动关卡结算
        rstScPkgBodyRsp.m_nErrNo = ActivityMgr::Instance().ActivityPveSettle(&roPlayerData, rstCsPkgBodyReq, rstScPkgBodyRsp);
    }
    else if(pstResLevel->m_bChapterType == CHAPTER_TYPE_GUILD_BOSS)
    {// 军团BOSS战斗结算
        rstScPkgBodyRsp.m_nErrNo = Guild::Instance().SettleBoss(&poPlayer->GetPlayerData(), rstCsPkgBodyReq, rstScPkgBodyRsp);
    }
    else if (pstResLevel->m_bChapterType == 0)
    {
        //不做处理
    }
    else
    {
        /*
        //普通关卡的结算
        RESFIGHTLEVEL *pstResLevel = CGameDataMgr::Instance().GetResFightLevelMgr().Find(rstCsPkgBodyReq.m_dwFLevelID);
        if (!pstResLevel)
        {
            LOGERR("dwPveLevelId:%d -> not exist.", rstCsPkgBodyReq.m_dwFLevelID);
            rstScPkgBodyRsp.m_nErrNo = ERR_NOT_FOUND;
            ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
            return ERR_NOT_FOUND;
        }
        */

        // 更新挑战记录
        IsFirstPass = FightPVE::Instance().UpdateRecord(&roPlayerData, rstCsPkgBodyReq, rstScPkgBodyRsp, pstResLevel->m_bChapterType);
        LevelRank::Instance().UpdateRecord(&roPlayerData, rstCsPkgBodyReq);

        if (rstCsPkgBodyReq.m_bIsPass != 0)
        {
            // 通关成功,更新玩家获得的普通奖励
            FightPVE::Instance().AddReward(&roPlayerData, pstResLevel->m_bChapterType, rstCsPkgBodyReq.m_dwFLevelID, rstScPkgBodyRsp, pstResLevel->m_bChapterType == CHAPTER_TYPE_TUTORIAL/*是否是新手*/);

            // 体力消耗，同步信息
            Item::Instance().ConsumeItem(&poPlayer->GetPlayerData(), ITEM_TYPE_AP, 0, -pstResLevel->m_bAPConsume, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_PEV_SETTLE);
        }
        else
        {
            // 失败体力消耗
            Item::Instance().ConsumeItem(&poPlayer->GetPlayerData(), ITEM_TYPE_AP, 0, -pstResLevel->m_bAPConsumeLose, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_PEV_SETTLE);
        }
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    //记录通关日志
    ZoneLog::Instance().WriteLevelPassLog(&poPlayer->GetPlayerData(), rstCsPkgBodyReq, IsFirstPass);

    return 0;
}

int CltDwLog_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    CS_PKG_DW_LOG& rstCSPkgBodyRsp = rstCsPkg.m_stBody.m_stDwLog;

    if (rstCSPkgBodyRsp.m_wType == CLT_FIGHT_STATS)
    {
        CltFightStatsArray stFightStatsArray;
        TdrError::ErrorType iRet = stFightStatsArray.unpack((char*)rstCSPkgBodyRsp.m_szBinLog, rstCSPkgBodyRsp.m_wLength);
        if( iRet != TdrError::TDR_NO_ERROR)
        {
           LOGERR("Unpack CltFightStats pkg failed!");
           return -1;
        }
        for(int i=0; i<stFightStatsArray.m_bCltFightStatsCNT; i++)
        {
            ZoneLog::Instance().WriteCltLog("CltFightStats", (char*)&stFightStatsArray.m_astCltFightStatsList[i], sizeof(CltFightStats));
        }
    }

    return 0;
}

int PveSkipFight_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);

    if (poPlayer == NULL)
    {
        LOGERR("poPlayer -> null, sessionid:%d", pstSession->m_dwSessionId);
        return ERR_DEFAULT;
    }

    CS_PKG_PVE_SKIP_FIGHT_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stPveSkipFightReq;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PVE_SKIP_FIGHT_RSP;
    SC_PKG_PVE_SKIP_FIGHT_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stPveSkipFightRsp;
    rstScPkgBodyRsp.m_dwPveLevelId = rstCsPkgBodyReq.m_dwPveLevelId;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    rstScPkgBodyRsp.m_bSkipCount = rstCsPkgBodyReq.m_bSkipCount;

    int iErrNo = Majesty::Instance().IsArriveLevel(&poPlayer->GetPlayerData(), LEVEL_LIMIT_SWEEP);

    if (ERR_NONE != iErrNo)
    {
        rstScPkgBodyRsp.m_nErrNo = iErrNo;
        LOGERR("Uin<%lu> PveId<%u> PveSkipFight lv limit iRet<%d>", poPlayer->GetUin(), rstCsPkgBodyReq.m_dwPveLevelId, iErrNo);
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return iErrNo;
    }

    RESFIGHTLEVEL *pstResLevel = CGameDataMgr::Instance().GetResFightLevelMgr().Find(rstCsPkgBodyReq.m_dwPveLevelId);

    if (!pstResLevel)
    {
        LOGERR("Uin<%lu> pstResLevel<%u>  is NULL", poPlayer->GetUin(), rstCsPkgBodyReq.m_dwPveLevelId);
        rstScPkgBodyRsp.m_nErrNo = ERR_NOT_FOUND;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return ERR_NOT_FOUND;
    }

    if (pstResLevel->m_bChapterType == CHAPTER_TYPE_ACTIVITY)
    {
        rstScPkgBodyRsp.m_nErrNo = ActivityMgr::Instance().ActivitySkipFight(&poPlayer->GetPlayerData(), rstCsPkgBodyReq, rstScPkgBodyRsp);
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

        return 0;
    }

    // settle player
    PlayerData& roPlayerData = poPlayer->GetPlayerData();
    roPlayerData.m_ullPveTimeStamp = CGameTime::Instance().GetCurrTimeMs();

	bool bCanSkipLevel = FightPVE::Instance().CanSkipLevel(&roPlayerData, rstCsPkgBodyReq.m_dwPveLevelId, rstCsPkgBodyReq.m_bSkipCount);
	if(CHAPTER_TYPE_NORMAL==pstResLevel->m_bChapterType)
	{
		bCanSkipLevel = true;
	}

    // 是否符合扫荡条件
    if(!bCanSkipLevel)
    {
        rstScPkgBodyRsp.m_nErrNo = ERR_PVE_LEVEL_SKIP_DISABLE;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        LOGERR("Uin<%lu> PveId<%u> can't skip", poPlayer->GetUin(), rstCsPkgBodyReq.m_dwPveLevelId);
        return ERR_PVE_LEVEL_SKIP_DISABLE;
    }

    if (!AP::Instance().IsEnough(&poPlayer->GetPlayerData(), (uint32_t)pstResLevel->m_bAPConsume * rstCsPkgBodyReq.m_bSkipCount))
    {
        rstScPkgBodyRsp.m_nErrNo = ERR_NOT_ENOUGH_AP;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        LOGERR("Uin<%lu> PveId<%u> ap limit", poPlayer->GetUin(), rstCsPkgBodyReq.m_dwPveLevelId);
        return ERR_NOT_ENOUGH_AP;
    }

    // 能够扫荡
    rstScPkgBodyRsp.m_nErrNo = ERR_NONE;

    // 更新 玩家 获得
    FightPVE::Instance().AddReward(&roPlayerData, pstResLevel->m_bChapterType, rstCsPkgBodyReq.m_dwPveLevelId, rstScPkgBodyRsp);

    // 体力消耗
    Item::Instance().ConsumeItem(&roPlayerData, ITEM_TYPE_AP, 0, -pstResLevel->m_bAPConsume * rstScPkgBodyRsp.m_bSkipCount, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_PEV_SETTLE_SKIP);

    // 更新关卡
    FightPVE::Instance().UpdateRecord(&roPlayerData, rstCsPkgBodyReq, rstScPkgBodyRsp, pstResLevel->m_bChapterType);

    //记扫荡日志
    ZoneLog::Instance().WriteLevelSweepLog(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwPveLevelId, rstScPkgBodyRsp.m_bSkipCount);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int PvePurchaseTimes_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (poPlayer == NULL)
    {
        LOGERR("poPlayer -> null, sessionid:%d", pstSession->m_dwSessionId);
        return ERR_DEFAULT;
    }
    //CHAPTER_TYPE_HERO = 3, 	/* 英雄章节 */
    //CHAPTER_TYPE_GUILD = 4, 	/* 公会章节 */
    //CHAPTER_TYPE_ACTIVITY = 5, 	/* 活动章节 */
    CS_PKG_PVE_PURCHASE_TIMES_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stPvePurchaseTimesReq;
    PlayerData& roPlayerData = poPlayer->GetPlayerData();

    SC_PKG_PVE_PURCHASE_TIMES_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stPvePurchaseTimesRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    int iRet = ERR_NONE;

    switch (rstCsPkgBodyReq.m_bType)
    {
    case CHAPTER_TYPE_HERO:
        iRet = FightPVE::Instance().ResetChallengeTimes(&roPlayerData, rstCsPkgBodyReq, rstScPkgBodyRsp);
        break;
    case CHAPTER_TYPE_ACTIVITY:
        iRet =  ActivityMgr::Instance().ResetCount(&roPlayerData, rstCsPkgBodyReq, rstScPkgBodyRsp);
        break;
    default:
        iRet = ERR_WRONG_PARA;
        break;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PVE_PURCHASE_TIMES_RSP;
    rstScPkgBodyRsp.m_nErrNo = iRet;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int PveChapterReward_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (poPlayer == NULL)
    {
        LOGERR("poPlayer -> null, sessionid:%d", pstSession->m_dwSessionId);
        return ERR_DEFAULT;
    }

    CS_PKG_PVE_CHAPTER_REWARD_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stPveChapterRewardReq;
    PlayerData& roPlayerData = poPlayer->GetPlayerData();

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PVE_CHAPTER_REWARD_RSP;
    SC_PKG_PVE_CHAPTER_REWARD_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stPveChapterRewardRsp;
    rstScPkgBodyRsp.m_nErrNo = FightPVE::Instance().HandleChapterRewardMsg(&roPlayerData, rstCsPkgBodyReq, rstScPkgBodyRsp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int PveFightLevelRecord_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */ )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (poPlayer == NULL)
    {
        LOGERR("poPlayer -> null, sessionid:%d", pstSession->m_dwSessionId);
        return ERR_DEFAULT;
    }

    CS_PKG_FIGHT_LEVEL_RECORD_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stFightLevelRecordReq;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_LEVEL_RECORD_RSP;
    SC_PKG_FIGHT_LEVEL_RECORD_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stFightLevelRecordRsp;

    int iRet = ERR_NONE;

    if(rstCsPkgBodyReq.m_bType == CHAPTER_TYPE_TUTORIAL || rstCsPkgBodyReq.m_bType == CHAPTER_TYPE_NORMAL || rstCsPkgBodyReq.m_bType == CHAPTER_TYPE_HERO)
    {

        iRet = LevelRank::Instance().GetRecord(rstCsPkgBodyReq, rstScPkgBodyRsp);//正常取值
    }
    else
    {
        iRet = ERR_LEVEL_TYPE_WRONG;  //关卡类型不对

    }
    rstScPkgBodyRsp.m_nErrNo = iRet;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}




int PveGetTreasure_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{

    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (poPlayer == NULL)
    {
        LOGERR("poPlayer -> null, sessionid:%d", pstSession->m_dwSessionId);
        return ERR_DEFAULT;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PVE_GET_TREASURE_RSP;
    SC_PKG_PVE_GET_TREASURE_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stPveGetTreasureRsp;
    rstScPkgBodyRsp.m_nErrNo = FightPVE::Instance().GetTreasure(&poPlayer->GetPlayerData(), rstCsPkg.m_stBody.m_stPveGetTreasureReq, rstScPkgBodyRsp);
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}


