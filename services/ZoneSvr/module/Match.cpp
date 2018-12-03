#include "Match.h"
#include "LogMacros.h"
#include "player/Player.h"
#include "MasterSkill.h"
#include "GeneralCard.h"
#include "Equip.h"
#include "Majesty.h"
#include "Tactics.h"
#include "../ZoneSvr.h"
#include "../gamedata/GameDataMgr.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "League.h"
#include "ZoneLog.h"
#include "DailyChallenge.h"
#include "TimeCycle.h"

using namespace std;
using namespace PKGMETA;

Match::Match()
{

}

Match::~Match()
{

}

int Match::Start(PlayerData* poData, uint8_t bMatchType)
{
    if (poData->GetRoleBaseInfo().m_llBlackRoomTime > CGameTime::Instance().GetCurrSecond())
    {
        LOGERR("Player(%s) Uin(%lu) the role is in the back room", poData->GetRoleBaseInfo().m_szRoleName, poData->m_ullUin);
        return ERR_BACK_ROOM;
    }

    int iRet = CheckStart(poData, bMatchType);
    if (ERR_NONE != iRet)
    {
        LOGERR("Player(%s) Uin(%lu) MatchType(%d) match check failed, Ret=%d",
                poData->GetRoleBaseInfo().m_szRoleName, poData->m_ullUin, bMatchType, iRet);
        return iRet;
    }

    // 初始化匹配玩家信息
    iRet = this->InitFightPlayerInfo(poData, bMatchType);
    if (iRet != ERR_NONE)
    {
        LOGERR("Player(%s) Uin(%lu) Init player info error", poData->GetRoleBaseInfo().m_szRoleName, poData->m_ullUin);
        return iRet;
    }

    //记录玩家的匹配状态
    poData->m_bMatchState = bMatchType;

    // 请求匹配系统匹配
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MATCH_START_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poData->m_ullUin;
	m_stSsPkg.m_stHead.m_ullReservId = 0;
    SS_PKG_MATCH_START_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stMatchStartReq;
    rstSsPkgBodyReq.m_stInfo = poData->m_oSelfInfo;
    rstSsPkgBodyReq.m_bMatchType = bMatchType;

    ZoneSvrMsgLayer::Instance().SendToMatchSvr(m_stSsPkg);

    LOGRUN("Player(%s) Uin(%lu) match start, type-%u", poData->GetRoleBaseInfo().m_szRoleName, poData->m_ullUin, (uint32_t)bMatchType);
    return ERR_NONE;
}

void Match::Cancel(PlayerData* poData, uint8_t bMatchType)
{
    DT_ROLE_ELO_INFO& rstRoleELOInfo= poData->GetELOInfo();
    DT_ROLE_MAJESTY_INFO& rstRoleMajestyInfo = poData->GetMajestyInfo();

    Player* poPlayer = poData->m_pOwner;
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return;
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MATCH_CANCEL_REQ;
    m_stSsPkg.m_stHead.m_ullReservId = 0;

    SS_PKG_MATCH_CANCEL_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stMatchCancelReq;

    rstSsPkgBodyReq.m_ullUin = poPlayer->GetUin();
    rstSsPkgBodyReq.m_bMatchType = bMatchType;

    switch(bMatchType)
    {
    case MATCH_TYPE_6V6:
    case MATCH_TYPE_LEISURE:
    case MATCH_TYPE_PEAK_ARENA:
        rstSsPkgBodyReq.m_dwEfficiency = rstRoleELOInfo.m_stPeakArenaInfo.m_dwScore;
        rstSsPkgBodyReq.m_dwScore = rstRoleELOInfo.m_stPeakArenaInfo.m_dwScore;
        break;
    case MATCH_TYPE_WEEKEND_LEAGUE:
        break;
    case MATCH_TYPE_DAILY_CHALLENGE:
        rstSsPkgBodyReq.m_dwScore = rstRoleELOInfo.m_stDailyChallengeInfo.m_bWinCount;
        rstSsPkgBodyReq.m_dwEfficiency = rstRoleMajestyInfo.m_dwHighestLi;
        break;
    default:
        break;
    }

    ZoneSvrMsgLayer::Instance().SendToMatchSvr(m_stSsPkg);
    LOGRUN("Uin<%lu> match cancel %s type %d", poData->m_ullUin, poPlayer->GetAccountName(), bMatchType);

    return;
}

int Match::InitFightPlayerInfo(PlayerData* poData, uint8_t bMatchType)
{
    DT_FIGHT_PLAYER_INFO& rstPlayerInfo = poData->m_oSelfInfo;

    rstPlayerInfo.m_iZoneSvrId = ZoneSvr::Instance().GetProcID();
    StrCpy(rstPlayerInfo.m_szName, poData->GetRoleBaseInfo().m_szRoleName, sizeof(rstPlayerInfo.m_szName));
    rstPlayerInfo.m_ullUin = poData->GetRoleBaseInfo().m_ullUin;
    rstPlayerInfo.m_bMajestyLevel = poData->GetMajestyInfo().m_wLevel;
    rstPlayerInfo.m_wHeadIcon = poData->GetMajestyInfo().m_wIconId;
	rstPlayerInfo.m_wHeadFrame = poData->GetMajestyInfo().m_wFrameId;
	rstPlayerInfo.m_wHeadTitle = poData->GetMajestyInfo().m_wTitleId;

    DT_ROLE_ELO_INFO & rstRoleELOInfo = poData->GetELOInfo();
    DT_ROLE_MAJESTY_INFO& rstRoleMajestyInfo = poData->GetMajestyInfo();
    rstPlayerInfo.m_bELOLvId = rstRoleELOInfo.m_stPeakArenaInfo.m_bELOLvId;
    rstPlayerInfo.m_bMatchType = bMatchType;
    rstPlayerInfo.m_bTroopNum = 0;
    rstPlayerInfo.m_dwMasterSkillId = 0;
    rstPlayerInfo.m_bGeneralCnt = 0;
    rstPlayerInfo.m_dwLeaderValue = 0;

    // 根据类型赋值匹配规则中的需要的值
    switch (bMatchType)
    {
    case MATCH_TYPE_6V6:
    case MATCH_TYPE_LEISURE:
    case MATCH_TYPE_PEAK_ARENA:
        rstPlayerInfo.m_stMatchInfo.m_dwEfficiency = rstRoleELOInfo.m_stPeakArenaInfo.m_dwScore;
        rstPlayerInfo.m_stMatchInfo.m_dwScore = rstRoleELOInfo.m_stPeakArenaInfo.m_dwScore;
        rstPlayerInfo.m_stMatchInfo.m_dwLosingStreak = 0;
        break;
    case MATCH_TYPE_WEEKEND_LEAGUE:
        break;
    case MATCH_TYPE_DAILY_CHALLENGE:
        rstPlayerInfo.m_stMatchInfo.m_dwEfficiency = rstRoleMajestyInfo.m_dwHighestLi;
        rstPlayerInfo.m_stMatchInfo.m_dwScore = rstRoleELOInfo.m_stDailyChallengeInfo.m_bWinCount;
        rstPlayerInfo.m_stMatchInfo.m_dwLosingStreak = rstRoleELOInfo.m_stDailyChallengeInfo.m_bLoseCount;
        break;
    default:
        break;
    }

    if (bMatchType != MATCH_TYPE_PEAK_ARENA)
    {
        rstPlayerInfo.m_dwLeaderValue = poData->m_dwLeaderValue;
        // 阵容信息
        DT_BATTLE_ARRAY_INFO* pstBattleArrayInfo;
        if (bMatchType == MATCH_TYPE_DAILY_CHALLENGE)
        {
            pstBattleArrayInfo = Majesty::Instance().GetBattleArrayInfo(poData, BATTLE_ARRAY_TYPE_DAILYCHALLENGE);
        }
        else
        {
            pstBattleArrayInfo = Majesty::Instance().GetBattleArrayInfo(poData, BATTLE_ARRAY_TYPE_NORMAL);
        }

        if (pstBattleArrayInfo == NULL)
        {
            LOGERR("Uin<%lu> the pstBattleArrayInfo<%hhu> is NULL! matchtype<%hhu>",
                poData->m_ullUin, BATTLE_ARRAY_TYPE_NORMAL, bMatchType);
            return ERR_DUNGEON_CREATE;
        }

        // 军师技
        	DT_ITEM_MSKILL* pstMSkillInfo = MasterSkill::Instance().GetMSkillInfo(poData, pstBattleArrayInfo->m_bMSId);
        	if (pstMSkillInfo == NULL)
        	{
                LOGERR("Uin<%lu> the pstMSkillInfo<%hhu> is NULL! matchtype<%hhu>", poData->m_ullUin,
                    pstBattleArrayInfo->m_bMSId, bMatchType);
                return ERR_DUNGEON_CREATE;
        	}
        	rstPlayerInfo.m_dwMasterSkillId = pstMSkillInfo->m_bId;
        	rstPlayerInfo.m_bMSkillLevel = pstMSkillInfo->m_bLevel;

        // 阵法
        DT_ITEM_TACTICS* pstTactics = Tactics::Instance().GetTacticsItem(poData, pstBattleArrayInfo->m_bTacticsType);
        if (pstTactics == NULL)
        {
            LOGRUN("Uin<%lu> the pstTacticsType<%hhu> is NULL! matchtype<%hhu>", poData->m_ullUin,
                pstBattleArrayInfo->m_bTacticsType, bMatchType);
            rstPlayerInfo.m_bTacticsType = 0;
            rstPlayerInfo.m_bTacticsLevel = 0;
        }
        else
        {
            rstPlayerInfo.m_bTacticsType = pstTactics->m_bType;
            rstPlayerInfo.m_bTacticsLevel = pstTactics->m_bLevel;
        }

        int iGCardNum = 0;
        for (int i = 0; i < pstBattleArrayInfo->m_bGeneralCnt; i++)
        {
           if (pstBattleArrayInfo->m_GeneralList[i] != 0)
           {
               iGCardNum++;
           }
        }

        if (iGCardNum < 3 && MATCH_TYPE_DAILY_CHALLENGE == bMatchType)
        {
            //是为了新手引导加入特殊处理
            LOGERR("Uin<%lu> the GCardNum<%d> is not enough! matchtype<%hhu>", poData->m_ullUin, iGCardNum, bMatchType);
            return ERR_DUNGEON_CREATE;
        }
        else if (iGCardNum < MAX_TROOP_NUM_PVP && MATCH_TYPE_DAILY_CHALLENGE != bMatchType)
        {
            LOGERR("Uin<%lu> the GCardNum<%d> is not enough! matchtype<%hhu>", poData->m_ullUin, iGCardNum, bMatchType);
            return ERR_DUNGEON_CREATE;
        }

        // 携带武将
    	    rstPlayerInfo.m_bTroopNum = pstBattleArrayInfo->m_bGeneralCnt;
        for (int i = 0; i < rstPlayerInfo.m_bTroopNum; i++)
        {
            if (pstBattleArrayInfo->m_GeneralList[i] == 0)
            {
                bzero( &rstPlayerInfo.m_astTroopList[i], sizeof(rstPlayerInfo.m_astTroopList[i]) );
                continue;
    		  }

            if (InitOneTroopInfo(poData, pstBattleArrayInfo->m_GeneralList[i], rstPlayerInfo.m_astTroopList[i]) != ERR_NONE)
            {
                return ERR_DUNGEON_CREATE;
            }
        }
    }
    else /*巅峰竞技需要把拥有的武将和皮肤传过去*/
    {
        DT_ROLE_GCARD_INFO& rstInfo = poData->GetGCardInfo();
        rstPlayerInfo.m_dwLeaderValue = 0;
        rstPlayerInfo.m_bGeneralCnt = rstInfo.m_iCount;
        for (int i=0; i<rstPlayerInfo.m_bGeneralCnt; i++)
        {
            rstPlayerInfo.m_astGeneralList[i].m_dwId = rstInfo.m_astData[i].m_dwId;
            rstPlayerInfo.m_astGeneralList[i].m_bStar = rstInfo.m_astData[i].m_bStar;
            rstPlayerInfo.m_astGeneralList[i].m_bIsAwake = 0;

            RESGENERAL* pResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(rstInfo.m_astData[i].m_dwId);
            if (!pResGeneral)
            {
                continue;
            }

            uint32_t dwEquipSeq = rstInfo.m_astData[i].m_astEquipList[pResGeneral->m_bSpecialEquipType-1].m_dwEquipSeq;
            DT_ITEM_EQUIP* pstEquip = Equip::Instance().Find(poData, dwEquipSeq);
            if (!pstEquip)
            {
                continue;
            }

            if (pstEquip->m_wStar >= pResGeneral->m_bSpecialEquipStar)
            {
                rstPlayerInfo.m_astGeneralList[i].m_bIsAwake = 1;
            }
        }

        uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();
        DT_ROLE_MISC_INFO& rstMiscInfo = poData->GetMiscInfo();
        rstPlayerInfo.m_wSkinCnt = 0;
        for (int i = 0; i < rstMiscInfo.m_wSkinCnt; i++)
        {
            if (rstMiscInfo.m_astSkinList[i].m_llEndTime == -1 ||
                rstMiscInfo.m_astSkinList[i].m_llEndTime < ullCurTime)
            {
                rstPlayerInfo.m_SkinList[rstPlayerInfo.m_wSkinCnt++] = rstMiscInfo.m_astSkinList[i].m_dwId;
            }
        }
    }

    // 初始化城墙血量 基础值+成长值
    int iCityHpBase = 0;
    RESBASIC* poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_HP_CITY);
    if (poResBasic != NULL)
    {
        iCityHpBase = (int)poResBasic->m_para[0];
    }

    ResMajestyLvMgr_t& rstResMajestyMgr = CGameDataMgr::Instance().GetResMajestyLvMgr();
    RESMAJESTYLV* pstResMajesty = rstResMajestyMgr.Find(rstPlayerInfo.m_bMajestyLevel);
    if (pstResMajesty == NULL)
    {
        LOGERR("Uin<%lu> pstResMajesty is null, majesty id-%d", poData->m_ullUin, rstPlayerInfo.m_bMajestyLevel);
        return ERR_DUNGEON_CREATE;
    }
    rstPlayerInfo.m_dwCityHp = iCityHpBase + pstResMajesty->m_dwWallHpGrow;

    return ERR_NONE;
}


int Match::InitOneTroopInfo(PlayerData* poData, DT_ITEM_GCARD* pstGCardInfo, OUT DT_TROOP_INFO& rstTroopInfo)
{

    // 武将信息
    rstTroopInfo.m_stGeneralInfo = *pstGCardInfo;
    // 武将缘分Buff
    GeneralCard::Instance().InitFateBuffIds(poData, &rstTroopInfo.m_stGeneralInfo);
    // 额外属性加成
    bzero(rstTroopInfo.m_AttrAddValue, sizeof(uint32_t)*MAX_ATTR_ADD_NUM);
    // 军师技加成
    GeneralCard::Instance().InitMSkillAttrs(poData, pstGCardInfo, rstTroopInfo);
    // 亲密度加成
    GeneralCard::Instance().InitFeedTrainAttrs(poData, pstGCardInfo, rstTroopInfo);

    // 武将装备
    for (int j = 0; j < EQUIP_TYPE_MAX_NUM; j++)
    {
        uint32_t dwSeq = pstGCardInfo->m_astEquipList[j].m_dwEquipSeq;
        DT_ITEM_EQUIP* pstEquipInfo = Equip::Instance().Find(poData, dwSeq);
        if (pstEquipInfo == NULL)
        {
            LOGERR("Player(%s) Uin(%lu) pstEquipInfo is null", poData->GetRoleBaseInfo().m_szRoleName, poData->m_ullUin);
            return ERR_DUNGEON_CREATE;
        }
        rstTroopInfo.m_astEquipInfoList[j] = *pstEquipInfo;
    }
    return ERR_NONE;
}
int Match::InitOneTroopInfo(PlayerData* poData, uint32_t dwGcardId, OUT DT_TROOP_INFO& rstTroopInfo)
{
    DT_ITEM_GCARD* pstGCardInfo = GeneralCard::Instance().Find(poData, dwGcardId);
    if (pstGCardInfo == NULL)
    {
        LOGERR("Player(%s) Uin(%lu) pstGCardInfo<%u> is null", poData->GetRoleBaseInfo().m_szRoleName, poData->m_ullUin, dwGcardId);
        return ERR_DUNGEON_CREATE;
    }
    return InitOneTroopInfo(poData, pstGCardInfo, rstTroopInfo);
}



int Match::InitBattleArrayFightInfo(PlayerData* poData, uint8_t bArrayType, OUT DT_BATTLE_ARRAY_FIGHT_INFO& rstArrayFightInfo )
{

	DT_BATTLE_ARRAY_INFO* pstBattleArrayInfo = Majesty::Instance().GetBattleArrayInfo(poData, bArrayType);
	if (!pstBattleArrayInfo)
	{
		LOGERR("Uin<%lu> the pstBattleArrayInfo<%d> is NULL", poData->m_ullUin, (int)bArrayType);
		return ERR_DUNGEON_CREATE;
	}
	DT_ITEM_MSKILL* pstMSkillInfo = MasterSkill::Instance().GetMSkillInfo(poData, pstBattleArrayInfo->m_bMSId);
	if (pstMSkillInfo == NULL)
	{
		LOGERR("Uin<%lu> the pstMSkillInfo<%hhu> is NULL! bArrayType<%hhu>", poData->m_ullUin,
			pstBattleArrayInfo->m_bMSId, bArrayType);
		rstArrayFightInfo.m_stMSkill.m_bId = MasterSkill::Instance().GetDefaultMSId();
		rstArrayFightInfo.m_stMSkill.m_bLevel = 1;
	}
	else
	{
		rstArrayFightInfo.m_stMSkill = *pstMSkillInfo;
	}

	// 阵法
	DT_ITEM_TACTICS* pstTactics = Tactics::Instance().GetTacticsItem(poData, pstBattleArrayInfo->m_bTacticsType);
	if (pstTactics == NULL)
	{
		LOGRUN("Uin<%lu> the pstTacticsType<%hhu> is NULL! bArrayType<%hhu>", poData->m_ullUin,
			pstBattleArrayInfo->m_bTacticsType, bArrayType);
		rstArrayFightInfo.m_stTactics.m_bType = 0;
		rstArrayFightInfo.m_stTactics.m_bLevel = 0;
	}
	else
	{
		rstArrayFightInfo.m_stTactics = *pstTactics;
	}
	for (int i = 0; i < pstBattleArrayInfo->m_bGeneralCnt; i++)
	{
		if (pstBattleArrayInfo->m_GeneralList[i] == 0)
		{
			bzero(&rstArrayFightInfo.m_astTroopList[i], sizeof(rstArrayFightInfo.m_astTroopList[i]));
			continue;
		}

		DT_ITEM_GCARD* pstGCardInfo = GeneralCard::Instance().Find(poData, pstBattleArrayInfo->m_GeneralList[i]);
		if (pstGCardInfo == NULL)
		{
			LOGERR("Player(%s) Uin(%lu) pstGCardInfo<%u> is null", poData->GetRoleBaseInfo().m_szRoleName, poData->m_ullUin, pstBattleArrayInfo->m_GeneralList[i]);
			return ERR_DUNGEON_CREATE;
		}
		rstArrayFightInfo.m_dwTroopLi += pstGCardInfo->m_dwLi;
		rstArrayFightInfo.m_dwLeaderValue += GeneralCard::Instance().GetGCardLeaderValue(poData, pstGCardInfo);
		if (InitOneTroopInfo(poData, pstGCardInfo, rstArrayFightInfo.m_astTroopList[i]) != ERR_NONE)
		{
			return ERR_DUNGEON_CREATE;
		}
	}
	rstArrayFightInfo.m_bTroopNum = pstBattleArrayInfo->m_bGeneralCnt;
	return ERR_NONE;
}

int Match::CheckStart(PlayerData* poData, uint8_t bMatchType)
{
    int iRet = ERR_NONE;
    switch (bMatchType)
    {
    case MATCH_TYPE_6V6:
        iRet = TimeCycle::Instance().CheckTime4Pvp6v6();
        break;
    case MATCH_TYPE_LEISURE:
    case MATCH_TYPE_PEAK_ARENA:
        break;
    case MATCH_TYPE_WEEKEND_LEAGUE:
        iRet = League::Instance().CheckMatch(poData);
        break;
    case MATCH_TYPE_DAILY_CHALLENGE:
        iRet = DailyChallenge::Instance().CheckMatch(poData);
        break;
    default:
        LOGERR("Player(%s) Uin(%lu) MatchType(%d) err!", poData->GetRoleBaseInfo().m_szRoleName, poData->m_ullUin, bMatchType);
        iRet = ERR_WRONG_PARA;
        break;
    }
    return iRet;
}


