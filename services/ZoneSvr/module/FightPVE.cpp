#include "FightPVE.h"
#include "LogMacros.h"
#include "Consume.h"
#include "Majesty.h"
#include "FakeRandom.h"
#include "Equip.h"
#include "Props.h"
#include "Task.h"
#include "Item.h"
#include "strutil.h"
#include "ZoneLog.h"
#include "dwlog_def.h"
#include "player/Player.h"
#include "GeneralCard.h"
#include "RankMgr.h"
#include "TaskAct.h"

#define PVE_LEVEL_UPDATE_HOUR (5)

using namespace DWLOG;
using namespace PKGMETA;

int PveLevelCmp(const void *pstFirst, const void *pstSecond)
{
	DT_PVE_LEVEL_INFO* pstItemFirst = (DT_PVE_LEVEL_INFO*)pstFirst;
	DT_PVE_LEVEL_INFO* pstItemSecond = (DT_PVE_LEVEL_INFO*)pstSecond;

	int iResult = (int)pstItemFirst->m_dwId - (int)pstItemSecond->m_dwId;

	return iResult;
}

int PveChapterCmp(const void *pstFirst, const void *pstSecond)
{
	DT_PVE_CHAPTER_INFO* pstItemFirst = (DT_PVE_CHAPTER_INFO*)pstFirst;
	DT_PVE_CHAPTER_INFO* pstItemSecond = (DT_PVE_CHAPTER_INFO*)pstSecond;

	int iResult =  ((int)pstItemFirst->m_bType * 256 + (int)pstItemFirst->m_bId) - ((int)pstItemSecond->m_bType * 256 + (int)pstItemSecond->m_bId);
	return iResult;
}

FightPVE::FightPVE()
{
	m_ullUpdateLastTime = 0;

	bzero(&m_stPveLevlData, sizeof(m_stPveLevlData));
	bzero(&m_stPveChapterData, sizeof(m_stPveChapterData));
}

int FightPVE::Init(PlayerData* pstData)
{
	DT_ROLE_PVE_INFO& rstInfo = pstData->GetPveInfo();

	// 一关没打
	rstInfo.m_dwPveLevelRecordCount = 0;

	rstInfo.m_dwPveChapterRecordCount = 0;

	return ERR_NONE;
}

int FightPVE::UpdateServer()
{
    ResBasicMgr_t& rResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* poBasic = rResBasicMgr.Find(COMMON_UPDATE_TIME);
    int m_iRefreshTime = 5;
    if (NULL == poBasic)
    {
        LOGERR("FATAL　ERROR :poBasic is null");
        m_iRefreshTime =  PVE_LEVEL_UPDATE_HOUR;
    }
    else
    {
        m_iRefreshTime = (int)poBasic->m_para[0];
    }

	// 每日副本次数限制
	uint64_t ullUpdateTime = 0;
	if (CGameTime::Instance().IsNeedUpdateByHour(m_ullUpdateLastTime, m_iRefreshTime, ullUpdateTime))
	{
		m_ullUpdateLastTime = ullUpdateTime;
	}

	return 0;
}

int FightPVE::UpdatePlayerData(PlayerData* pstData)
{
	DT_ROLE_PVE_INFO& rstInfo = pstData->GetPveInfo();
	if (rstInfo.m_ullPveUpdateTime < m_ullUpdateLastTime)
	{
		rstInfo.m_ullPveUpdateTime = m_ullUpdateLastTime;

		// 刷新每日可打PVE次数
		for (int i= 0; i< (int)rstInfo.m_dwPveLevelRecordCount; ++i)
		{
			DT_PVE_LEVEL_INFO& rstPveLevelInfo = rstInfo.m_astPveLevelRecordList[i];
			RESFIGHTLEVEL *pLevel = CGameDataMgr::Instance().GetResFightLevelMgr().Find(rstPveLevelInfo.m_dwId);
			if(pLevel == NULL)
			{
				LOGERR("not exist -> %d", rstPveLevelInfo.m_dwId);
				rstPveLevelInfo.m_bChallengeTimesLeft = 0;
			}
			else
			{
				rstPveLevelInfo.m_bChallengeTimesLeft = pLevel->m_bFreeTimes;
			}
            rstPveLevelInfo.m_bResetTimes = 0;
		}
	}

	return ERR_NONE;
}

bool FightPVE::AddReward(PlayerData* pstData, uint8_t bChapterType, uint32_t dwPveLevelId, SC_PKG_FIGHT_PVE_SETTLE_RSP &rSettleRsp, uint8_t bTutorial)
{
	RESFIGHTLEVELPL *pLevelPL = CGameDataMgr::Instance().GetResFightLevelPLMgr().Find(dwPveLevelId);

	if(NULL == pLevelPL)
	{
		LOGERR("dwPveLevelId:%d -> not exist.", dwPveLevelId);
		return false;
	}

	DT_PVE_LEVEL_INFO* pstPveLvInfo = this->FindLevel(pstData, dwPveLevelId);

	if (pstPveLvInfo == NULL)
	{
		LOGERR("pstPveLvInfo:%d -> not exist.", dwPveLevelId);
		return false;
	}
    int iDouble = 1;
    if ((bChapterType == CHAPTER_TYPE_NORMAL     //pve双倍模式
          && TaskAct::Instance().IsActTypeOpen(pstData, ACT_TYPE_DOUBLE_PVE_NORMAL))
        || (bChapterType == CHAPTER_TYPE_HERO        //精英双倍模式
            && TaskAct::Instance().IsActTypeOpen(pstData, ACT_TYPE_DOUBLE_PVE_HERO)))
    {
        iDouble = 2;
    }

	// 判断该关卡是否可以重复领取奖励
	if (pLevelPL->m_bIsRewardRepeat == 0)
	{
		if (pstPveLvInfo->m_bIsRewardDrew != 0)
		{
			// 已领取
			return true;
		}
	}

	//VIP等级会提高金币的收入
	RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(pstData->GetVIPLv()+1);
	uint32_t dwGold = pLevelPL->m_dwGold + (uint32_t)(pLevelPL->m_dwGold * pResVip->m_dwGoldenBunos / 100.0);
	if (pstPveLvInfo->m_bIsRewardDrew == 0 && bTutorial != 1)
	{
		//第一次通关时，经验和金币10倍
		RESBASIC* pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find((int)FIRST_PASS_PVE_MULTIPLE);

		if (NULL == pResBasic)
		{
			LOGERR("pResBasic is null");
			return false;
		}

		Item::Instance().RewardItem(pstData, ITEM_TYPE_EXP, 0, pResBasic->m_para[1] * pLevelPL->m_dwExp, rSettleRsp.m_stSyncItemInfo, METHOD_PEV_SETTLE, Item::CONST_SHOW_PROPERTY_NO);
		Item::Instance().RewardItem(pstData, ITEM_TYPE_GOLD, 0, pResBasic->m_para[0] * dwGold, rSettleRsp.m_stSyncItemInfo, METHOD_PEV_SETTLE, Item::CONST_SHOW_PROPERTY_NO);
	}
	else
	{
		Item::Instance().RewardItem(pstData, ITEM_TYPE_EXP, 0, pLevelPL->m_dwExp, rSettleRsp.m_stSyncItemInfo, METHOD_PEV_SETTLE, Item::CONST_SHOW_PROPERTY_NO);
		Item::Instance().RewardItem(pstData, ITEM_TYPE_GOLD, 0, dwGold, rSettleRsp.m_stSyncItemInfo, METHOD_PEV_SETTLE, Item::CONST_SHOW_PROPERTY_NO);
	}

	Item::Instance().RewardItem(pstData, ITEM_TYPE_DIAMOND, 0, pLevelPL->m_dwDiamond, rSettleRsp.m_stSyncItemInfo, METHOD_PEV_SETTLE, Item::CONST_SHOW_PROPERTY_NO);

	LOGRUN("dwPveLevelId:%d -> diamon:%d, gold:%d, exp:%d", dwPveLevelId, pLevelPL->m_dwDiamond, pLevelPL->m_dwGold, pLevelPL->m_dwExp);

	// 添加随机掉落关卡物品
	this->_AddRandomDropItem(pstData, pLevelPL, rSettleRsp.m_stSyncItemInfo, iDouble);

	//给上阵武将添加经验
	GeneralCard::Instance().AddGeneralExp(pstData, dwPveLevelId, rSettleRsp.m_GeneralList);

	if (0 == pstPveLvInfo->m_bIsRewardDrew)
	{
		//第一次通关后,概率掉落物品
		pstPveLvInfo->m_bIsRewardDrew = 1;
		rSettleRsp.m_bIsFirstPass = 1;
		GeneralCard::Instance().HandleGroupCard(pstData, 2/*关卡类型组卡开孔*/, dwPveLevelId);
	}
	else
	{
		rSettleRsp.m_bIsFirstPass = 0;
	}

	return true;
}

bool FightPVE::AddReward(PlayerData* pstData, uint8_t bChapterType, uint32_t dwPveLevelId, SC_PKG_PVE_SKIP_FIGHT_RSP &rSkipFightRsp)
{
	RESFIGHTLEVELPL *pLevelPL = CGameDataMgr::Instance().GetResFightLevelPLMgr().Find(dwPveLevelId);
	if (pLevelPL == NULL)
	{
		LOGERR("dwPveLevelId:%d -> not exist.", dwPveLevelId);
		return false;
	}
	// 是否合理关卡
	// 比如跳关
    int iDouble = 1;
    if ((bChapterType == CHAPTER_TYPE_NORMAL     //pve双倍模式
        && TaskAct::Instance().IsActTypeOpen(pstData, ACT_TYPE_DOUBLE_PVE_NORMAL))
        || (bChapterType == CHAPTER_TYPE_HERO        //精英双倍模式
            && TaskAct::Instance().IsActTypeOpen(pstData, ACT_TYPE_DOUBLE_PVE_HERO)))
    {
        iDouble = 2;
    }
	//VIP等级会提高金币的收入
	RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(pstData->GetVIPLv()+1);
	uint32_t dwGold = pLevelPL->m_dwGold + (uint32_t)(pLevelPL->m_dwGold * pResVip->m_dwGoldenBunos / 100.0);

	// 多次扫荡，分别获得每次扫荡获得的奖品
	for (int i= 0 ; i< rSkipFightRsp.m_bSkipCount; ++i)
	{
        DT_SYNC_ITEM_INFO& rRewardItemInfo = rSkipFightRsp.m_astRewardItemInfo[i];
 		rRewardItemInfo.m_bSyncItemCount = 0;
		this->_AddRandomDropItem(pstData, pLevelPL, rRewardItemInfo, iDouble);

		//经验，金币，钻石产出
		Item::Instance().RewardItem(pstData, ITEM_TYPE_EXP, 0, pLevelPL->m_dwExp, rRewardItemInfo, METHOD_PEV_SETTLE);
		Item::Instance().RewardItem(pstData, ITEM_TYPE_GOLD, 0, dwGold, rRewardItemInfo, METHOD_PEV_SETTLE);
		Item::Instance().RewardItem(pstData, ITEM_TYPE_DIAMOND, 0, pLevelPL->m_dwDiamond, rRewardItemInfo, METHOD_PEV_SETTLE);

		//额外物品产出
		for (int j=0; j<pLevelPL->m_bPropsExtraCount; j++)
		{
			Item::Instance().RewardItem(pstData, pLevelPL->m_szPropsExtraTypeList[j], pLevelPL->m_propsExtraIdList[j], pLevelPL->m_szPropsExtraCountList[j],
												rRewardItemInfo, METHOD_PEV_SETTLE, Item::CONST_SHOW_PROPERTY_EXTRA);
		}
	}

	return true;
}

void FightPVE::_InitRewardArr(RESPVECHAPTERREWARD* pResReward, int iIdx, uint8_t* arrType, uint32_t* arrId, uint32_t* arrNum)
{
	arrType[0] = pResReward->m_szReward1Type[iIdx];
	arrId[0] = pResReward->m_reward1Id[iIdx];
	arrNum[0] = pResReward->m_reward1Num[iIdx];

	arrType[1] = pResReward->m_szReward2Type[iIdx];
	arrId[1] = pResReward->m_reward2Id[iIdx];
	arrNum[1] = pResReward->m_reward2Num[iIdx];

	arrType[2] = pResReward->m_szReward3Type[iIdx];
	arrId[2] = pResReward->m_reward3Id[iIdx];
	arrNum[2] = pResReward->m_reward3Num[iIdx];

	arrType[3] = pResReward->m_szReward4Type[iIdx];
	arrId[3] = pResReward->m_reward4Id[iIdx];
	arrNum[3] = pResReward->m_reward4Num[iIdx];

	arrType[4] = pResReward->m_szReward5Type[iIdx];
	arrId[4] = pResReward->m_reward5Id[iIdx];
	arrNum[4] = pResReward->m_reward5Num[iIdx];
}

int FightPVE::HandleChapterRewardMsg(PlayerData* pstData, CS_PKG_PVE_CHAPTER_REWARD_REQ& rstCsPkgBodyReq, SC_PKG_PVE_CHAPTER_REWARD_RSP& rstScPkgBodyRsp)
{
	rstScPkgBodyRsp.m_bChapterType = rstCsPkgBodyReq.m_bChapterType;
	rstScPkgBodyRsp.m_bChapterId = rstCsPkgBodyReq.m_bChapterId;

	rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

	int iIdx = rstCsPkgBodyReq.m_bChapterRewardIdx;

	DT_PVE_CHAPTER_INFO* pstChapterInfo = this->FindChapter(pstData, rstCsPkgBodyReq.m_bChapterType, rstCsPkgBodyReq.m_bChapterId);
	if (pstChapterInfo == NULL)
	{
		LOGERR("pstChapterInfo not exist -> chapterType = %d, chapterId = %d", rstCsPkgBodyReq.m_bChapterType, rstCsPkgBodyReq.m_bChapterId);
		return ERR_NOT_FOUND;
	}

	if ((pstChapterInfo->m_bRewardFlag & (1 << iIdx)) != 0)
	{
		// 已领取
		LOGERR("Already drew.");
		return ERR_WRONG_STATE;
	}

	RESPVECHAPTERREWARD *pResReward = CGameDataMgr::Instance().GetResPveChapterRewardMgr().Find(pstChapterInfo->m_dwRewardId);

	if (pResReward == NULL)
	{
		LOGERR("pResReward not exist -> %d", pstChapterInfo->m_dwRewardId);
		return ERR_NOT_FOUND;
	}

	if (iIdx >= pResReward->m_bRewardStepCount)
	{
		return ERR_NOT_FOUND;
	}

	// 一次只领取一个章节奖励
	if (pstChapterInfo->m_bStar < pResReward->m_szRequireStarNum[iIdx])
	{
		// 星星不足，不能领奖
		return ERR_NOT_SATISFY_COND;
	}

	pstChapterInfo->m_bRewardFlag |= 1 << iIdx;
	rstScPkgBodyRsp.m_bChapterRewardFlag = pstChapterInfo->m_bRewardFlag;

	uint8_t arrType[RES_MAX_CHAPTER_REWARD_STEP_CNT];
	uint32_t arrId[RES_MAX_CHAPTER_REWARD_STEP_CNT];
	uint32_t arrNum[RES_MAX_CHAPTER_REWARD_STEP_CNT];
	_InitRewardArr(pResReward, iIdx, arrType, arrId, arrNum);

	for (int i=0; i<pResReward->m_szRewardCnt[iIdx]; i++)
	{
		Item::Instance().RewardItem(pstData, arrType[i], arrId[i], arrNum[i], rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_PEV_SETTLE_CHAPTER);
	}

	return ERR_NONE;
}

int FightPVE::CanPlayLevel(PlayerData* pstData, uint32_t dwPveLevelId)
{
	//DT_ROLE_PVE_INFO& rstInfo = pstData->GetPveInfo();
	RESFIGHTLEVEL *pLevel = CGameDataMgr::Instance().GetResFightLevelMgr().Find(dwPveLevelId);
	if(NULL == pLevel)
	{
		LOGERR("dwPveLevelId:%d -> not exist.", dwPveLevelId);
		return ERR_NOT_FOUND;
	}

	if(pLevel->m_dwRequireMajestyLevel > Majesty::Instance().GetLevel(pstData))
	{
		// 未解锁，主公等级不够
		return ERR_MAJESTY_UN_SATISFY;
	}

	if (pLevel->m_dwRequireFightLevel1 > 0)
	{
		DT_PVE_LEVEL_INFO* pstPveLvInfo = this->FindLevel(pstData, pLevel->m_dwRequireFightLevel1);
		if (pstPveLvInfo == NULL)
		{
			LOGERR("pstPveLvInfo:%d -> not exist.", pLevel->m_dwRequireFightLevel1);
			return ERR_NOT_FOUND;
		}

		// 是否开启
		if (pstPveLvInfo->m_bIsPassed == 0)
		{
			// 前置关卡1未通关
			// 未解锁，未开启
			return ERR_PVE_LEVEL_DISABLE;
		}
	}

	if (pLevel->m_dwRequireFightLevel2 > 0)
	{
		DT_PVE_LEVEL_INFO* pstPveLvInfo = this->FindLevel(pstData, pLevel->m_dwRequireFightLevel2);
		if (pstPveLvInfo == NULL)
		{
			LOGERR("pstPveLvInfo:%d -> not exist.", pLevel->m_dwRequireFightLevel2);
			return ERR_NOT_FOUND;
		}

		// 是否开启
		if (pstPveLvInfo->m_bIsPassed == 0)
		{
			// 前置关卡2未通关
			// 未解锁，未开启
			return ERR_PVE_LEVEL_DISABLE;
		}
	}

	if (this->GetPveChallengeTimesLeft(pstData, dwPveLevelId) <= 0)
	{
		// 挑战次数不足
		return ERR_PVE_LEVEL_NOT_ENOUGH;
	}

	return ERR_NONE;
}

bool FightPVE::CanSkipLevel(PlayerData* pstData, uint32_t dwPveLevelId, uint32_t dwSkipTimes)
{
	DT_PVE_LEVEL_INFO *pLevelInfo = this->FindLevel(pstData, dwPveLevelId);
	if (pLevelInfo == NULL)
	{
		LOGERR("pLevelInfo is NULL.");
		return false;
	}

	if (!pLevelInfo->m_bIsPassed)
	{
		// 未通关
		return false;
	}

    //由PKGMETA::MAX_PVE_STAR_COND（3星）可扫荡改为1星
	if (pLevelInfo->m_bEvaluateStar < RES_SKIP_PVE_STAR)
	{
		// 评星不足
		return false;
	}

	if (pLevelInfo->m_bChallengeTimesLeft < dwSkipTimes)
	{
		return false;
	}

	return true;
}

int FightPVE::UpdateRecord(PlayerData* pstData, CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsSettleReq, SC_PKG_FIGHT_PVE_SETTLE_RSP& rstScSettleRsp, uint8_t bChapterType)
{
	bool bIsNew = false;

	DT_PVE_LEVEL_INFO *pLevelInfo = this->FindLevel(pstData, rstCsSettleReq.m_dwFLevelID);
	if (pLevelInfo == NULL)
	{
		this->AddLevel(pstData, rstCsSettleReq.m_dwFLevelID);
		pLevelInfo = this->FindLevel(pstData, rstCsSettleReq.m_dwFLevelID);

		if (pLevelInfo == NULL)
		{
			LOGERR("pLevelInfo is NULL.");
			return 0;
		}

		bIsNew = true;
	}

	// 挑战次数更新
	if(CHAPTER_TYPE_NORMAL!=bChapterType)
	{
	    if(pLevelInfo->m_bChallengeTimesLeft > 0) pLevelInfo->m_bChallengeTimesLeft--;
	}
	pLevelInfo->m_bChallengeTimesTotal++;

	// 星级新增数量
	uint8_t bIncreaseStarNum = 0;

	if (rstCsSettleReq.m_bIsPass != 0)
	{
		pLevelInfo->m_bIsPassed = 1;

		// 评星规则变为累计评星
		for (int i=0; i<rstCsSettleReq.m_bStarEvalResult; i++)
		{
			uint32_t dwStarEvalId = rstCsSettleReq.m_StarEvalIDList[i];

			int j=0;
			for (; j<pLevelInfo->m_bEvaluateStar; j++)
			{
				if (pLevelInfo->m_EvaluateList[j] == dwStarEvalId)
				{
					break;
				}
			}

			if (j == pLevelInfo->m_bEvaluateStar && j < RES_MAX_PVE_STAR)
			{
				// 拥有新评星
				pLevelInfo->m_EvaluateList[j] = dwStarEvalId;

				pLevelInfo->m_bEvaluateStar++;

				bIncreaseStarNum++;
			}
		}
#if 0
		// 评星结果可能和评星条件个数不一致
		if(pLevelInfo->m_bEvaluateStar < rstCsSettleReq.m_bStarEvalResult)
		{
			// 新增数量
			bIncreaseStarNum = rstCsSettleReq.m_bStarEvalResult - pLevelInfo->m_bEvaluateStar;

			pLevelInfo->m_bEvaluateStar = rstCsSettleReq.m_bStarEvalResult;
			for (int i= 0; i< rstCsSettleReq.m_bStarEvalResult; ++i)
			{
				pLevelInfo->m_EvaluateList[i] = rstCsSettleReq.m_StarEvalIDList[i];
			}
		}
		else
		{
			// 新增为零
			bIncreaseStarNum = 0;
		}
#endif

	}

	// 同步给客户端
	rstScSettleRsp.m_bStarEvalResult = pLevelInfo->m_bEvaluateStar;
	for (int i=0; i<pLevelInfo->m_bEvaluateStar; i++)
	{
		rstScSettleRsp.m_StarEvalIDList[i] = pLevelInfo->m_EvaluateList[i];
	}

	RESFIGHTLEVEL *pstResLevel = CGameDataMgr::Instance().GetResFightLevelMgr().Find(rstCsSettleReq.m_dwFLevelID);
	if (pstResLevel != NULL)
	{
        if (0 == rstCsSettleReq.m_bIsPass && CHAPTER_TYPE_HERO == pstResLevel->m_bChapterType)
        {
			//精英副本失败,返回挑战次数
			pLevelInfo->m_bChallengeTimesLeft++;
            pLevelInfo->m_bChallengeTimesTotal--;
        }

		// 普通任务才记录
		if(CHAPTER_TYPE_TUTORIAL == pstResLevel->m_bChapterType
			|| CHAPTER_TYPE_NORMAL == pstResLevel->m_bChapterType
			|| CHAPTER_TYPE_HERO == pstResLevel->m_bChapterType)
		{
			// 任务计数
			Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_STAR, bIncreaseStarNum, 0, 1);
			// 任务记数修改
			Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_PVE, 1/*value*/, 3);
		}

		int iHardType = 0;
		if (pstResLevel->m_bChapterType == CHAPTER_TYPE_NORMAL
            || pstResLevel->m_bChapterType == CHAPTER_TYPE_TUTORIAL)
		{
			iHardType = 1;
		}
		else if (pstResLevel->m_bChapterType == CHAPTER_TYPE_HERO)
		{
			iHardType = 2;
		}

		if (iHardType > 0 &&  rstCsSettleReq.m_bIsPass)
		{
			Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_PVE, 1/*value*/, iHardType, rstCsSettleReq.m_dwFLevelID);
		}
	}

	// 挑战次数同步
    rstScSettleRsp.m_bChallengeTimesLeft = pLevelInfo->m_bChallengeTimesLeft;
    rstScSettleRsp.m_bChallengeTimesTotal = pLevelInfo->m_bChallengeTimesTotal;

	// 章节更新
	rstScSettleRsp.m_dwChapterStar = this->UpdateRecordChapter(pstData, rstCsSettleReq.m_dwFLevelID, bIncreaseStarNum);

	//记首次通关日志
	if (bIsNew)
	{
	    return 1;
	}
    else
    {
        return 0;
    }
}

// 扫荡只更新次数
void FightPVE::UpdateRecord(PlayerData* pstData, CS_PKG_PVE_SKIP_FIGHT_REQ& rstCsSkipReq, SC_PKG_PVE_SKIP_FIGHT_RSP& rstScSkipRsp, uint8_t bChapterType)
{
	DT_PVE_LEVEL_INFO *pLevelInfo = this->FindLevel(pstData, rstCsSkipReq.m_dwPveLevelId);
	if (pLevelInfo == NULL)
	{
		LOGERR("not exist -> %d, skip count:%d", rstCsSkipReq.m_dwPveLevelId, rstCsSkipReq.m_bSkipCount);
		return;
	}

	//普通关卡不判定次数
	if(CHAPTER_TYPE_NORMAL!=bChapterType)
	{
		if (pLevelInfo->m_bChallengeTimesLeft < rstCsSkipReq.m_bSkipCount)
		{
			LOGERR("invalid -> curd:%d, count:%d", rstCsSkipReq.m_dwPveLevelId, rstCsSkipReq.m_bSkipCount);
			return;
		}

		pLevelInfo->m_bChallengeTimesLeft -= rstCsSkipReq.m_bSkipCount;
	}

	rstScSkipRsp.m_bChallengeTimesLeft = pLevelInfo->m_bChallengeTimesLeft;

	// 任务记数修改
	Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_PVE, rstCsSkipReq.m_bSkipCount/*value*/, 3);
	RESFIGHTLEVEL *pstResLevel = CGameDataMgr::Instance().GetResFightLevelMgr().Find(rstCsSkipReq.m_dwPveLevelId);
	if (pstResLevel != NULL)
	{
		int iHardType = 0;
		if (pstResLevel->m_bChapterType == CHAPTER_TYPE_NORMAL)
		{
			iHardType = 1;
		}
		else if (pstResLevel->m_bChapterType == CHAPTER_TYPE_HERO)
		{
			iHardType = 2;
		}

		if (iHardType > 0)
		{
			Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_PVE, rstCsSkipReq.m_bSkipCount/*value*/, iHardType, rstCsSkipReq.m_dwPveLevelId);
		}
	}
}


int FightPVE::GetTreasure(PlayerData* pstData, CS_PKG_PVE_GET_TREASURE_REQ& rstCsReq, SC_PKG_PVE_GET_TREASURE_RSP& rstScRsp)
{
    uint32_t dwPveLevelId = rstCsReq.m_dwPveLevelId;
    int iRet = ERR_NONE;
    do
    {
        RESFIGHTLEVEL *pstResLevel = CGameDataMgr::Instance().GetResFightLevelMgr().Find(dwPveLevelId);
        if (!pstResLevel)
        {
            LOGERR("Uin<%lu> dwPveLevelId<%d>  not exist.", pstData->m_ullUin, dwPveLevelId);
            iRet = ERR_NOT_FOUND;
            break;
        }


        DT_PVE_LEVEL_INFO *pLevelInfo = FightPVE::Instance().FindLevel(pstData, dwPveLevelId);
        if (pLevelInfo == NULL)
        {
            LOGERR("Uin<%lu> pLevelInfo is NULL.", pstData->m_ullUin);
            iRet =  ERR_DEFAULT;
            break;
        }

        if ( !(pLevelInfo->m_bIsPassed == 1 && pLevelInfo->m_bIsTreasureDraw == COMMON_DO_SOME_STATE_NONE) )
        {
            LOGERR("Uin<%lu> not satisy cond", pstData->m_ullUin);
            iRet =  ERR_NOT_SATISFY_COND;
            break;
        }
        pLevelInfo->m_bIsTreasureDraw = COMMON_DO_SOME_STATE_FINISHED;

		rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
        iRet = Props::Instance().GetCommonAward(pstData, pstResLevel->m_dwGiftId, rstScRsp.m_stSyncItemInfo, METHOD_PVE_GET_TREASURE);
        if (iRet == ERR_NONE)
        {
            rstScRsp.m_dwPveLevelId = dwPveLevelId;
        }

    } while (0);
    return iRet;
}

void FightPVE::UpdateRecord4Debug(PlayerData* pstData, uint32_t dwPveLevelId)
{
	DT_PVE_LEVEL_INFO *pLevelInfo = this->FindLevel(pstData, dwPveLevelId);
	if (pLevelInfo == NULL)
	{
		this->AddLevel(pstData, dwPveLevelId, true);
	}
}

int FightPVE::UpdateRecordChapter(PlayerData* pstData, uint32_t dwPveLevelId, uint8_t bStarAdd/*新增数量*/)
{
	RESFIGHTLEVEL *pLevel = CGameDataMgr::Instance().GetResFightLevelMgr().Find(dwPveLevelId);
	if (NULL == pLevel)
	{
		LOGERR("not exist -> %d", dwPveLevelId);
		return 0;
	}

	DT_PVE_CHAPTER_INFO* pChapterInfo = this->FindChapter(pstData, pLevel->m_bChapterType, pLevel->m_bChapterId);
	if (pChapterInfo == NULL)
	{
		this->AddChapter(pstData, pLevel->m_bChapterType, pLevel->m_bChapterId, pLevel->m_dwChapterRewardId);
		pChapterInfo = this->FindChapter(pstData, pLevel->m_bChapterType, pLevel->m_bChapterId);
	}

	if (pChapterInfo != NULL)
	{
		// 正常关卡才参与评星
		if (pLevel->m_bSection != 0)
		{
			pChapterInfo->m_bStar += bStarAdd;
            pstData->GetPveInfo().m_dwPveTotalStar += bStarAdd;
            RankMgr::Instance().UpdatePveStar(pstData);
		}

		return pChapterInfo->m_bStar;
	}

	return 0;
}

int FightPVE::UpdatePveChallengeTimes(PlayerData* pstData, uint32_t dwPveLevelId, uint32_t dwPurchaseTimes)
{
	DT_PVE_LEVEL_INFO *pLevelInfo = this->FindLevel(pstData, dwPveLevelId);
	if (NULL == pLevelInfo)
	{
		LOGERR("not exist -> %d", dwPveLevelId);
		return ERR_DEFAULT;
	}
	else
	{
		pLevelInfo->m_bChallengeTimesLeft += dwPurchaseTimes;
		return ERR_NONE;
	}
}

int FightPVE::GetPveChallengeTimesLeft(PlayerData* pstData, uint32_t dwPveLevelId)
{
	DT_PVE_LEVEL_INFO *pLevelInfo = this->FindLevel(pstData, dwPveLevelId);
	if (pLevelInfo == NULL)
	{
		RESFIGHTLEVEL *pLevel = CGameDataMgr::Instance().GetResFightLevelMgr().Find(dwPveLevelId);

		if (pLevel == NULL)
		{
			LOGERR("not exist -> %d", dwPveLevelId);
		}
		else
		{
			return pLevel->m_bFreeTimes;
		}
	}
	else
	{
		return pLevelInfo->m_bChallengeTimesLeft;
	}

	return 0;
}

PKGMETA::DT_PVE_LEVEL_INFO* FightPVE::FindLevel(PlayerData* pstData, uint32_t dwPveLevelId)
{
	m_stPveLevlData.m_dwId = dwPveLevelId;
	DT_ROLE_PVE_INFO& rstInfo = pstData->GetPveInfo();
	int iEqual = 0;
	int iIndex = MyBSearch(&m_stPveLevlData, rstInfo.m_astPveLevelRecordList, rstInfo.m_dwPveLevelRecordCount, sizeof(DT_PVE_LEVEL_INFO), &iEqual, PveLevelCmp);
	if (!iEqual)
	{
		return NULL;
	}

	return &rstInfo.m_astPveLevelRecordList[iIndex];
}

int FightPVE::AddLevel(PlayerData* pstData, uint32_t dwPveLevelId, uint8_t bIsPassed /*= 0*/)
{
	DT_ROLE_PVE_INFO& rstInfo = pstData->GetPveInfo();

	bzero(&m_stPveLevlData, sizeof(m_stPveLevlData));
	m_stPveLevlData.m_dwId = dwPveLevelId;
	m_stPveLevlData.m_bIsPassed = bIsPassed;

	RESFIGHTLEVEL *pLevel = CGameDataMgr::Instance().GetResFightLevelMgr().Find(dwPveLevelId);
	if(pLevel != NULL)
	{
		m_stPveLevlData.m_bChallengeTimesLeft = pLevel->m_bFreeTimes;
	}
	else
	{
		LOGERR("pLevel not exist -> %d", dwPveLevelId);
	}

	size_t nmemb = (size_t)rstInfo.m_dwPveLevelRecordCount;
	if (nmemb >= MAX_NUM_PVE_LEVEL)
	{
		LOGERR("rstInfo.m_dwPveLevelRecordCount<%d> is larger than MAX_NUM_PVE_LEVEL", rstInfo.m_dwPveLevelRecordCount);
		return -1;
	}
	if (MyBInsert(&m_stPveLevlData, rstInfo.m_astPveLevelRecordList, &nmemb, sizeof(DT_PVE_LEVEL_INFO), 1, PveLevelCmp))
	{
		rstInfo.m_dwPveLevelRecordCount = (int32_t)nmemb;
	}

	return 0;
}

DT_PVE_CHAPTER_INFO* FightPVE::FindChapter(PlayerData* pstData, uint8_t bChapterType, uint8_t bChapterId)
{
	m_stPveChapterData.m_bType = bChapterType;
	m_stPveChapterData.m_bId = bChapterId;
	DT_ROLE_PVE_INFO& rstInfo = pstData->GetPveInfo();
	int iEqual = 0;
	int iIndex = MyBSearch(&m_stPveChapterData, rstInfo.m_astPveChapterRecordList, rstInfo.m_dwPveChapterRecordCount, sizeof(DT_PVE_CHAPTER_INFO), &iEqual, PveChapterCmp);
	if (!iEqual)
	{
		return NULL;
	}

	return &rstInfo.m_astPveChapterRecordList[iIndex];
}

int FightPVE::AddChapter(PlayerData* pstData, uint8_t bChapterType, uint8_t bChapterId, uint32_t dwRewardId)
{
	DT_ROLE_PVE_INFO& rstInfo = pstData->GetPveInfo();

	m_stPveChapterData.m_bType = bChapterType;
	m_stPveChapterData.m_bId = bChapterId;
	m_stPveChapterData.m_dwRewardId = dwRewardId;

	m_stPveChapterData.m_bStar = 0;
	m_stPveChapterData.m_bRewardFlag = 0;

	size_t nmemb = (size_t)rstInfo.m_dwPveChapterRecordCount;
	if (nmemb >= MAX_NUM_PVE_CHAPTER)
	{
		LOGERR("rstInfo.m_dwPveChapterRecordCount<%d> is larger than MAX_NUM_PVE_CHAPTER", rstInfo.m_dwPveChapterRecordCount);
		return ERR_DEFAULT;
	}
	if (MyBInsert(&m_stPveChapterData, rstInfo.m_astPveChapterRecordList, &nmemb, sizeof(m_stPveChapterData), 1, PveChapterCmp))
	{
		rstInfo.m_dwPveChapterRecordCount = (int32_t)nmemb;
	}

	return ERR_NONE;
}

// 随机值 大于 掉落几率 则 不掉落,概率以10000基数计算
int FightPVE::_IsDropItem(uint32_t dwProbability, int dwMinCount, int dwMaxCount)
{
	int dwDropCount = 0;

	if(dwProbability >= 10000)
	{
		dwDropCount = dwMaxCount;
	}
	else if (dwProbability > 0)
	{
		uint32_t dwRandValue = CFakeRandom::Instance().Random(10000);
		if(dwRandValue < dwProbability)
		{
			dwDropCount = dwMaxCount;
		}
		else
		{
			dwDropCount = dwMinCount;
		}
		/*
		while(dwMaxCount--)
		{
			uint32_t dwRandValue = CFakeRandom::Instance().Random(10000);
			if(dwRandValue < dwProbability)
			{
				++dwDropCount;
			}
		}
		*/
	}

	return (int)dwDropCount;
}

void FightPVE::_AddRandomDropItem(PlayerData* pstData, RESFIGHTLEVELPL *pLevelPL, DT_SYNC_ITEM_INFO& rstSyncItemInfo, int iDouble)
{
	int iId = this->_FindIdByRes(pLevelPL);
	DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();
	for (int i= 0; i< pLevelPL->m_bPropsCount; ++i)
	{
		int iRealDropCount = 0;
		uint32_t dwDroped = rstMiscInfo.m_PveGetPropList[iId] & (1 << i);
		if (dwDroped > 0)
		{
			iRealDropCount = this->_IsDropItem(pLevelPL->m_propsProbabilityList[i], pLevelPL->m_szPropsCountList[i], pLevelPL->m_szPropsCountMaxList[i]);
		}
		else
		{
			iRealDropCount = this->_IsDropItem(pLevelPL->m_propsIsFirstGetList[i], pLevelPL->m_szPropsCountList[i], pLevelPL->m_szPropsCountMaxList[i]);
			if (iRealDropCount == pLevelPL->m_szPropsCountMaxList[i])
			{
				rstMiscInfo.m_PveGetPropList[iId] |= (1 << i);
			}
		}

		// 数量为 0, 继续下一个物品
		if (iRealDropCount == 0)
		{
			continue;
		}

		uint8_t bItemType = pLevelPL->m_szPropsTypeList[i];
		uint32_t dwId = pLevelPL->m_propsIdList[i];

		Item::Instance().RewardItem(pstData, bItemType, dwId, iRealDropCount * iDouble, rstSyncItemInfo, METHOD_PEV_SETTLE);
	}
}

int FightPVE::_FindIdByRes(RESFIGHTLEVELPL *pLevelPL)
{
	int iRet = 0;
	RESFIGHTLEVELPL* pTmpFightLevelPl = NULL;
	int iResCnt = CGameDataMgr::Instance().GetResFightLevelPLMgr().GetResNum();
	for ( ; iRet < iResCnt; iRet++)
	{
		pTmpFightLevelPl = CGameDataMgr::Instance().GetResFightLevelPLMgr().GetResByPos(iRet);
		if (pTmpFightLevelPl == NULL)
		{
			continue;
		}
		if (pTmpFightLevelPl->m_dwId == pLevelPL->m_dwId)
		{
			break;
		}
	}

	return iRet;
}

#define RESCOMSUME_HERE_PVE 901

int FightPVE::ResetChallengeTimes(PlayerData* pstData, CS_PKG_PVE_PURCHASE_TIMES_REQ& rstPurchaseTimesReq, SC_PKG_PVE_PURCHASE_TIMES_RSP& rstPurchaseTimesRsp)
{
    DT_PVE_LEVEL_INFO *pLevelInfo = this->FindLevel(pstData, rstPurchaseTimesReq.m_dwPveLevelId);
    if (pLevelInfo == NULL)
    {
        LOGERR("the param error, Uin<%lu>,PvelevelId<%d>",pstData->m_pOwner->GetUin(), rstPurchaseTimesReq.m_dwPveLevelId);
        return ERR_WRONG_PARA;
    }
    RESFIGHTLEVEL *pLevel = CGameDataMgr::Instance().GetResFightLevelMgr().Find(rstPurchaseTimesReq.m_dwPveLevelId);
    if (NULL == pLevel)
    {
        LOGERR("RESFIGHTLEVEL not find  Uin<%lu>,PvelevelId<%d>",pstData->m_pOwner->GetUin(), rstPurchaseTimesReq.m_dwPveLevelId);
        return ERR_NOT_FOUND;
    }
    if (pLevel->m_bChapterType != CHAPTER_TYPE_HERO)
    {
		//不是精英副本
        LOGERR("reset charpter tpyer error  Uin<%lu>,PvelevelId<%d>",pstData->m_pOwner->GetUin(), rstPurchaseTimesReq.m_dwPveLevelId);
        return ERR_WRONG_PARA;
    }

    RESCONSUME* pResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(RESCOMSUME_HERE_PVE);
    if (NULL == pResConsume)
    {
        LOGERR("RESCONSUME not find  Uin<%lu>,PvelevelId<%d>",pstData->m_pOwner->GetUin(), rstPurchaseTimesReq.m_dwPveLevelId);
        return ERR_NOT_FOUND;
    }
    if (0 != pLevelInfo->m_bChallengeTimesLeft)
    {
        //次数不为0 不能重置
        return ERR_PVE_PURCHASE_NOT_ZERO;
    }

    //要判断Vip 重置次数上限, 目前还有设计 先填着 3次
    RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(pstData->GetVIPLv()+1);
    if (pLevelInfo->m_bResetTimes >= pResVip->m_dwResetChapterTimes)
    {
        //重置超出上限
        return ERR_PVE_RESET_TIMES_NOT_ENOUGH;
    }
    uint32_t dwConsumeDia = pResConsume->m_lvList[pLevelInfo->m_bResetTimes];
    if (!Consume::Instance().IsEnoughDiamond(pstData, dwConsumeDia))
    {
        return ERR_NOT_ENOUGH_DIAMOND;
    }
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -dwConsumeDia, rstPurchaseTimesRsp.m_stSyncItemInfo, METHOD_PVE_HERO_BUY_COUNT);
    pLevelInfo->m_bResetTimes++;
    pLevelInfo->m_bChallengeTimesLeft = pLevel->m_bFreeTimes;
    rstPurchaseTimesRsp.m_bChallengeTimesLeft = pLevelInfo->m_bChallengeTimesLeft;
    rstPurchaseTimesRsp.m_dwPveLevelId = rstPurchaseTimesReq.m_dwPveLevelId;
    rstPurchaseTimesRsp.m_bType = rstPurchaseTimesReq.m_bType;
    LOGRUN("Player<%lu> ResetPVEHero PVEID<%u> consume dia<%u>, ResetTimes<%u>", pstData->m_pOwner->GetUin(), rstPurchaseTimesReq.m_dwPveLevelId,  dwConsumeDia, pLevelInfo->m_bResetTimes);
    return ERR_NONE;
}

