#include "ActivityMgr.h"
#include "LogMacros.h"
#include "Item.h"
#include "Lottery.h"
#include "Gm/Gm.h"
#include "Consume.h"
#include "dwlog_def.h"
#include "player/Player.h"
#include "AP.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "Task.h"
#include "ov_res_public.h"
#include "ZoneLog.h"
#include "Majesty.h"
#include "GloryItemsMgr.h"
#include "GeneralCard.h"
#include "TaskAct.h"

#define PVE_ACTIVITY_UPDATE_HOUR (0)

#define ACTIVITY_OPEN 1

using namespace PKGMETA;
using namespace DWLOG;

bool ActivityMgr::Init()
{
    RESBASIC* poBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(COMMON_UPDATE_TIME);
    if (NULL == poBasic)
    {
       LOGERR("poBasic is null");
       return false;
    }
    m_iDailyResetTime = (int)poBasic->m_para[0];

    m_ullUptTimeStamp = 0;

    poBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(1721);
    if (NULL == poBasic)
    {
       LOGERR("poBasic is null");
       return false;
    }
    m_iGuildVitality = (int)poBasic->m_para[0];

    uint8_t bWeekDay = (uint8_t)CGameTime::Instance().GetCurDayOfWeek();   // 1-7

    for (int i = 0; i < MAX_ACTIVITY_PVE_NUM; i++)
    {
       m_szActPveFinishCnt[i] = 0;
       RESACTIVITY* poResActivity = CGameDataMgr::Instance().GetResActivityMgr().Find(i + 1);
       if (poResActivity == NULL)
       {
         continue;
       }

       if (poResActivity->m_szOpenDay[bWeekDay-1] == ACTIVITY_OPEN)
       {
         // 该天开放活动，重置
         m_szActPveFinishCnt[i] = 0;
       }
       else
       {
         // 没有开放，相当于 已完成
         m_szActPveFinishCnt[i] = poResActivity->m_dwTimesLimit;
       }
    }

    ResVIPMgr_t& rResVipMgr = CGameDataMgr::Instance().GetResVIPMgr();

    RESBASIC* poBasicGold = CGameDataMgr::Instance().GetResBasicMgr().Find(CD_ACTIVITY_TYPE_GOLD);
    if (NULL == poBasicGold)
    {
       LOGERR("poBasicGold is null");
       return false;
    }
    m_iColdResetCostDiaGold = (int)poBasicGold->m_para[1];

    RESBASIC* poBasicGrain = CGameDataMgr::Instance().GetResBasicMgr().Find(CD_ACTIVITY_TYPE_GRAIN);
    if (NULL == poBasicGrain)
    {
       LOGERR("poBasicGrain is null");
       return false;
    }
    m_iColdResetCostDiaGrain = (int)poBasicGrain->m_para[1];

    RESBASIC* poBasicRound = CGameDataMgr::Instance().GetResBasicMgr().Find(CD_ACTIVITY_TYPE_ROUND);
    if (NULL == poBasicRound)
    {
       LOGERR("poBasicRound is null");
       return false;
    }
    m_iColdResetCostDiaRound = (int)poBasicRound->m_para[1];

    for (int i=0; i<MAX_VIP_LEVEL; i++)
    {
       RESVIP* pResVip = rResVipMgr.Find(i+1);
       if (NULL == pResVip)
       {
         LOGERR("pResVip is null");
         return false;
       }
       m_ullColdTimeGoldList[i] = poBasicGold->m_para[0] - pResVip->m_wActivityTimeReduce;
       m_ullColdTimeGrainList[i] = poBasicGrain->m_para[0] - pResVip->m_wActivityTimeReduce;
       m_ullColdTimeRoundList[i] = poBasicRound->m_para[0] - pResVip->m_wActivityTimeReduce;
    }

    return true;
}

int ActivityMgr::UpdateServer()
{
    uint64_t ullUpdateTimeMs = 0;
    if (CGameTime::Instance().IsNeedUpdateByHour(m_ullUptTimeStamp, m_iDailyResetTime, ullUpdateTimeMs))
    {
       // 重置活动
       m_ullUptTimeStamp = ullUpdateTimeMs;
    }

    return 0;
}

int ActivityMgr::UpdatePlayerData(PlayerData* poPlayerData)
{
    DT_ROLE_MISC_INFO& rstMiscInfo = poPlayerData->GetMiscInfo();

#if 1
    if (rstMiscInfo.m_ullActivityLastUptTime < m_ullUptTimeStamp)
    {
       LOGRUN("Uin<%lu> ActivityMgr UpdatePlayerData 1=<%lu>, 2=<%lu>", poPlayerData->m_ullUin, rstMiscInfo.m_ullActivityLastUptTime, m_ullUptTimeStamp);
        rstMiscInfo.m_ullActivityLastUptTime = m_ullUptTimeStamp;

        // 更新活动情况
        for (int i = 0; i < MAX_ACTIVITY_PVE_NUM; ++i)
        {
            rstMiscInfo.m_astActivityInfo[i].m_bFinishCnt = m_szActPveFinishCnt[i];
         rstMiscInfo.m_astActivityInfo[i].m_bResetCnt = 0;
        }

       // 通知客户端更新
       _SendSynMsg(poPlayerData, 0, rstMiscInfo.m_astActivityInfo[0]);
    }
#endif

    return ERR_NONE;
}

//活动副本奖励结算
int ActivityMgr::ActivityPveSettle(PlayerData* poPlayerData, CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsPkgBodyReq, SC_PKG_FIGHT_PVE_SETTLE_RSP& rstScPkgBodyRsp)
{
    if (0 == rstCsPkgBodyReq.m_bIsPass)
    {
       //失败不管
       rstScPkgBodyRsp.m_ullTimeStampColdDown = 0;
       return ERR_NONE;
    }

    uint32_t dwLevelId = rstCsPkgBodyReq.m_dwFLevelID;
    RESFIGHTLEVEL* poResFightLevel = CGameDataMgr::Instance().GetResFightLevelMgr().Find(dwLevelId);

    if (poResFightLevel == NULL)
    {
        LOGERR("Uin<%lu>, poResFightLevel is null dwLevelId<%u> ",poPlayerData->m_ullUin, dwLevelId);
        return ERR_SYS;
    }

    uint8_t bActivityType = poResFightLevel->m_bChapterId;

    uint32_t dwActivityRewardId = poResFightLevel->m_dwChapterRewardId;

    uint8_t bHardType = poResFightLevel->m_bSection - 1;

    if (bActivityType < 1 || bActivityType > MAX_ACTIVITY_PVE_NUM)
    {
       LOGERR("Uin<%lu>, bActivityType<%d> is out limit.",poPlayerData->m_ullUin, bActivityType);
       return ERR_SYS;
    }

    DT_ROLE_GUILD_INFO& rstGuildInfo = poPlayerData->GetGuildInfo();
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = poPlayerData->GetMajestyInfo();
    DT_ROLE_MISC_INFO& rstMiscInfo = poPlayerData->GetMiscInfo();
    DT_ACTIVITY_INFO& rstActivityInfo = rstMiscInfo.m_astActivityInfo[bActivityType - 1];

    // 活动
    RESACTIVITY* poResActivity = CGameDataMgr::Instance().GetResActivityMgr().Find(bActivityType);
    if (poResActivity == NULL)
    {
        LOGERR("Uin<%lu> poResActivity is null SeqId<%u> ", poPlayerData->m_ullUin, bActivityType);
        return ERR_SYS;
    }

    // 活动奖励
    RESACTIVITYREWARD* poResActivityReward = CGameDataMgr::Instance().GetResActivityRewardMgr().Find(dwActivityRewardId);

    if (NULL == poResActivityReward)
    {
       LOGERR("Uin<%lu>, poResActivityReward is null, SeqId<%u>", poPlayerData->m_ullUin, dwActivityRewardId);
       return ERR_SYS;
    }

    if (rstMajestyInfo.m_wLevel < poResActivity->m_wLevelLimit)
    {
        LOGERR("Uin<%lu>,ActivityMgr: Lv limit, CondiLv<%d>, PlayerLv<%d>", poPlayerData->m_ullUin, poResActivity->m_wLevelLimit, poPlayerData->GetMajestyInfo().m_wLevel);
        return ERR_LEVEL_LIMIT;
    }

    if (bHardType >= RES_MAX_ACTIVITY_HARD_LEVEL || rstMajestyInfo.m_wLevel < poResActivity->m_szHardLevelOpenLv[bHardType])
    {
       // 挑战该难度等级不足
       return ERR_LEVEL_LIMIT;
    }

    if (bHardType > 0 &&  rstActivityInfo.m_szEvaluateLevelList[bHardType - 1] < poResActivityReward->m_bUnlockEvaluateLv)
    {
        // 难度未解锁
        return ERR_LEVEL_LIMIT;
    }

    if (!AP::Instance().IsEnough(poPlayerData, poResFightLevel->m_bAPConsume))
    {
       LOGERR("Uin<%lu> AP is not enought", poPlayerData->m_ullUin);
       return ERR_NOT_ENOUGH_AP;
    }

    // 这里开始和结束时间 +/- 1秒,
    uint64_t ulBeginDayTimeSec = (uint64_t)CGameTime::Instance().GetBeginOfDay();
    uint64_t ulCurTimeSec = (uint64_t)CGameTime::Instance().GetCurrSecond();

    uint64_t ulBeginTimeSec = ulBeginDayTimeSec + poResActivity->m_wBeginTime * 3600;
    uint64_t ulEndTimeSec = ulBeginDayTimeSec + poResActivity->m_wEndTime * 3600 - 1;

    uint8_t bWeekDay = (uint8_t)CGameTime::Instance().GetCurDayOfWeek();// 1-7
    //  先检查活动开启时间是否满足,再检查次数是否满足

    int iTimelimit = bActivityType==ACTIVITY_TYPE_GOLD ? poResActivity->m_dwTimesLimit+rstGuildInfo.m_bGuildGoldTime : poResActivity->m_dwTimesLimit;

    if ( !( ACTIVITY_OPEN == poResActivity->m_szOpenDay[bWeekDay-1] &&          /*一周的开放时间检测*/
         ulCurTimeSec >= ulBeginTimeSec && ulCurTimeSec <= ulEndTimeSec &&       /*当天具体开放时间检测*/
         rstActivityInfo.m_bFinishCnt < iTimelimit)      /*完成次数检测*/
         )
    {
        LOGERR("Uin<%lu>, Condition limit, bActivityType<%d>, iTimelimit<%d> ", poPlayerData->m_ullUin, bActivityType, iTimelimit);
        return ERR_SYS;
    }

    int iDouble = 1;
    if ((bActivityType == ACTIVITY_TYPE_GRAIN && TaskAct::Instance().IsActTypeOpen(poPlayerData, ACT_TYPE_DOUBLE_GRAIN))
        || (bActivityType == ACTIVITY_TYPE_ROUND && TaskAct::Instance().IsActTypeOpen(poPlayerData, ACT_TYPE_DOUBLE_ROUND)))
    {
        iDouble = 2;
    }

    Task::Instance().ModifyData(poPlayerData, TASK_VALUE_TYPE_ACTIVITY, 1/*value*/, 1/*para1*/, bActivityType/*活动类型*/);

    // 体力消耗，同步信息
    Item::Instance().ConsumeItem(poPlayerData, ITEM_TYPE_AP, 0, -poResFightLevel->m_bAPConsume, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_ACTIVITY_PEV_SETTLE);

    // 固定奖励
    Item::Instance().ConsumeItem(poPlayerData, ITEM_TYPE_EXP, 0, poResActivityReward->m_dwExp, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_ACTIVITY_PEV_SETTLE);

    //科技加成
    if (bActivityType == ACTIVITY_TYPE_GOLD)
    {
        rstCsPkgBodyReq.m_dwBaseRewardPara += rstCsPkgBodyReq.m_dwBaseRewardPara*rstGuildInfo.m_bGuildGoldScoreRatio/100;
        rstCsPkgBodyReq.m_dwBoxRewardPara += rstCsPkgBodyReq.m_dwBoxRewardPara*rstGuildInfo.m_bGuildGoldScoreRatio/100;

        Item::Instance().ConsumeItem(poPlayerData, ITEM_TYPE_GUILD_VITALITY, 0, m_iGuildVitality, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_ACTIVITY_PEV_SETTLE);
    }

    // 基础奖励
    uint32_t dwBaseReward = poResActivityReward->m_dwBaseRewardNum * rstCsPkgBodyReq.m_dwBaseRewardPara;

    if (rstActivityInfo.m_HistoryInfoList[0] < dwBaseReward)
    {
       rstActivityInfo.m_HistoryInfoList[0] = dwBaseReward;
    }

    if (poResActivityReward->m_bBaseRewardType != 0)
    {
       if (poResActivityReward->m_bBaseRewardType == ITEM_TYPE_GOLD)
       {
         // 基础奖励如果是金币奖励

         //VIP等级会提高金币的收入，金币通过同步协议发送
         RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(poPlayerData->GetVIPLv() + 1);
         dwBaseReward += (uint32_t)(dwBaseReward * pResVip->m_dwGoldenBunos / 100.0);

         Item::Instance().RewardItem(poPlayerData, poResActivityReward->m_bBaseRewardType, poResActivityReward->m_dwBaseRewardId, dwBaseReward * iDouble,
          rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_ACTIVITY_PEV_SETTLE, Item::CONST_SHOW_PROPERTY_NORMAL);
       }
       else
       {
         Item::Instance().RewardItem(poPlayerData, poResActivityReward->m_bBaseRewardType, poResActivityReward->m_dwBaseRewardId, dwBaseReward * iDouble,
                rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_ACTIVITY_PEV_SETTLE, Item::CONST_SHOW_PROPERTY_NORMAL);
       }
    }

    // 宝箱奖励
    RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(poPlayerData->GetVIPLv() + 1);
    if (poResActivityReward->m_bBoxRewardType == ACTIVITY_BOXREWARD_LIST)
    {

    }
    else if (poResActivityReward->m_bBoxRewardType == ACTIVITY_BOXREWARD_SCORE)
    {
       uint8_t bCnt = 0;

       for (int i=0; i<poResActivityReward->m_bBoxRewardNum; i++)
       {
         if (rstCsPkgBodyReq.m_dwBoxRewardPara >= poResActivityReward->m_rewardLimit[i])
         {
          uint32_t dwPondId = poResActivityReward->m_reward[i];
          if (dwPondId == 0)
          {
              // 没有奖励
              continue;
          }

          bCnt++;

          uint8_t bIndexStart = rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount;
          Lottery::Instance().DrawLotteryByPond(poPlayerData, dwPondId, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_ACTIVITY_PEV_SETTLE);
				if(iDouble > 1)
				{
					//双倍奖励
					Lottery::Instance().DrawLotteryByPond(poPlayerData, dwPondId, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_ACTIVITY_PEV_SETTLE);
				}
          uint8_t bIndexEnd = rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount;

          if (pResVip != NULL && pResVip->m_dwGoldenBunos > 0)
          {
              for (; bIndexStart<bIndexEnd; bIndexStart++)
              {
                 if (rstScPkgBodyRsp.m_stSyncItemInfo.m_astSyncItemList[bIndexStart].m_bItemType == ITEM_TYPE_GOLD)
                 {
                   uint32_t dwGoldNum = rstScPkgBodyRsp.m_stSyncItemInfo.m_astSyncItemList[bIndexStart].m_iValueChg * pResVip->m_dwGoldenBunos / 100.0;
                   Item::Instance().RewardItem(poPlayerData, ITEM_TYPE_GOLD, 0, dwGoldNum,
                    rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_ACTIVITY_PEV_SETTLE, Item::CONST_SHOW_PROPERTY_NORMAL);
                 }
              }
          }
         }
       }

       LOGRUN("bActivityType=<%d>, uin=<%lu>, BoxRewardScore=<%d>, cnt=<%d>", bActivityType, poPlayerData->m_ullUin, rstCsPkgBodyReq.m_dwBoxRewardPara, bCnt);
    }

    // 活动通过评价
    // 难度解锁
    // 统计信息
    uint32_t dwEvaluateValue = 0; // 评价值
    uint8_t bEvaluateLevel = 0;    // 评价等级
    uint32_t dwEvaluateScore = 0;  // 评价分数
    switch (bActivityType)
    {
    case ACTIVITY_TYPE_ROUND:
       {
         // 波次活动
         // rstCsPkgBodyReq.m_bKillRoundNum;  // 击杀波次
         // rstCsPkgBodyReq.m_bTreasureBoxNum; // 宝箱数

         // 评价值
         dwEvaluateValue = (uint32_t)(rstCsPkgBodyReq.m_bKillRoundNum * poResActivityReward->m_evaluatePara[0]);
            // 活动评价结果
            for (int i=0; i<RES_MAX_ACTIVITY_EVALUATE_LEVEL; i++)
            {
                // 逃跑越多，评价越低
                if (dwEvaluateValue < poResActivityReward->m_evaluateLimit[i])
                {
                    break;
                }

                bEvaluateLevel = poResActivityReward->m_szEvaluateLevel[i];
            }

            dwEvaluateScore = (uint32_t)(dwEvaluateValue * poResActivityReward->m_fScorePara);
         //LOGWARN("评价等级：%d", dwEvaluateValue);
         //LOGWARN("活动难度：%d", bHardType);
            ZoneLog::Instance().WriteActivityLog(poPlayerData, rstCsPkgBodyReq, rstCsPkgBodyReq.m_bKillRoundNum, rstCsPkgBodyReq.m_bTreasureBoxNum, 0,
                bEvaluateLevel, bHardType);

         break;
       }
    case ACTIVITY_TYPE_GRAIN:
       {
         // 粮车活动
         // rstCsPkgBodyReq.m_bKillRoundNum;  // 击杀波次
         // rstCsPkgBodyReq.m_bKillTowerNum;  // 逃逸个数
         // rstCsPkgBodyReq.m_dwBoxRewardPara; // 积分

         // 评价值
         float fValue = rstCsPkgBodyReq.m_bKillRoundNum * poResActivityReward->m_evaluatePara[0] - rstCsPkgBodyReq.m_bKillTowerNum * poResActivityReward->m_evaluatePara[1];
         fValue = fValue > 0 ? fValue : 0;

         dwEvaluateValue = (uint32_t)fValue;
            // 活动评价结果
            for (int i=0; i<RES_MAX_ACTIVITY_EVALUATE_LEVEL; i++)
            {
                // 逃跑越多，评价越低
                if (dwEvaluateValue < poResActivityReward->m_evaluateLimit[i])
                {
                    break;
                }

                bEvaluateLevel = poResActivityReward->m_szEvaluateLevel[i];
            }

            dwEvaluateScore = (uint32_t)(dwEvaluateValue * poResActivityReward->m_fScorePara);
         //LOGWARN("评价等级：%d", bEvaluateLevel);
         //LOGWARN("活动难度：%d", bHardType);
            ZoneLog::Instance().WriteActivityLog(poPlayerData, rstCsPkgBodyReq, rstCsPkgBodyReq.m_bKillRoundNum, rstCsPkgBodyReq.m_bKillTowerNum, rstCsPkgBodyReq.m_dwBoxRewardPara,
                bEvaluateLevel, bHardType);
         break;
       }
    case ACTIVITY_TYPE_GOLD:
       {
         // 金币活动，保存活动数据
         // rstCsPkgBodyReq.m_bKillTowerNum; // 击杀敌人
         // rstCsPkgBodyReq.m_bKillBarrierNum; // 击杀难民
         // rstCsPkgBodyReq.m_dwDamageOut; // 连击
         // rstCsPkgBodyReq.m_dwBaseRewardPara; // 积分

         // dwBaseReward; // 金币获得

         if (rstActivityInfo.m_HistoryInfoList[1] < rstCsPkgBodyReq.m_bKillTowerNum)
          rstActivityInfo.m_HistoryInfoList[1] = rstCsPkgBodyReq.m_bKillTowerNum;     // 最大击杀敌人

         if (rstActivityInfo.m_HistoryInfoList[2] < rstCsPkgBodyReq.m_dwDamageOut)
         {
          rstActivityInfo.m_HistoryInfoList[2] = rstCsPkgBodyReq.m_dwDamageOut;     // 最大连击
          GloryItemsMgr::Instance().AddMajestyItems(poPlayerData, MAJESTY_ITEM_ACCESS_GOLD_HIT, rstCsPkgBodyReq.m_dwDamageOut);
         }

         // 评价值
         dwEvaluateValue = (uint32_t)(rstCsPkgBodyReq.m_dwBoxRewardPara * poResActivityReward->m_evaluatePara[0]);
		 // 活动评价结果
		 for (int i = 0; i < RES_MAX_ACTIVITY_EVALUATE_LEVEL; i++)
		 {
			 // 逃跑越多，评价越低
			 if (dwEvaluateValue < poResActivityReward->m_evaluateLimit[i])
			 {
				 break;
			 }

			 bEvaluateLevel = poResActivityReward->m_szEvaluateLevel[i];
		 }

		 dwEvaluateScore = (uint32_t)(dwEvaluateValue * poResActivityReward->m_fScorePara);
		 //LOGWARN("评价等级：%d", dwEvaluateValue);
		 //LOGWARN("活动难度：%d", bHardType);
		 //LOGWARN("EvaluateScore = %d, EvaluateValue = %d", dwEvaluateScore, dwEvaluateValue);
		 ZoneLog::Instance().WriteActivityLog(poPlayerData, rstCsPkgBodyReq, dwEvaluateScore, rstCsPkgBodyReq.m_bKillTowerNum, rstCsPkgBodyReq.m_bKillBarrierNum,
			 bEvaluateLevel, bHardType);
		 //参与保卫军团次数
		 Task::Instance().ModifyData(poPlayerData, TASK_VALUE_TYPE_GUILD_OTHER, 1, 7, 1);
		 //获取积分
		 Task::Instance().ModifyData(poPlayerData, TASK_VALUE_TYPE_GUILD_OTHER, rstCsPkgBodyReq.m_dwBaseRewardPara, 7, 2);
		 //击杀难民数低于N
		 Task::Instance().ModifyData(poPlayerData, TASK_VALUE_TYPE_GUILD_OTHER, rstCsPkgBodyReq.m_bKillBarrierNum, 7, 3);
         break;
       }
    default:
       break;
    }

    // 评价等级
    rstScPkgBodyRsp.m_bTreasureBoxNum = bEvaluateLevel;
    // 评价分数
    rstScPkgBodyRsp.m_dwChapterStar = dwEvaluateScore;

    // 保存
    rstActivityInfo.m_bFinishCnt++;
    rstActivityInfo.m_szEvaluateLevelList[bHardType] = bEvaluateLevel;

    //更新CD时间戳
    _UpdateColdDownTime(poPlayerData, bActivityType, rstActivityInfo, rstCsPkgBodyReq.m_ullTimeStampStart, rstScPkgBodyRsp.m_ullTimeStampColdDown);

    _SendSynMsg(poPlayerData, bActivityType, rstActivityInfo);

    if (bActivityType == ACTIVITY_TYPE_GOLD)
    {
       _SendLevelEvent(poPlayerData, dwLevelId, CHAPTER_TYPE_ACTIVITY, dwEvaluateScore);
    }

    return ERR_NONE;
}

int ActivityMgr::ActivitySkipFight(PlayerData* poPlayerData, CS_PKG_PVE_SKIP_FIGHT_REQ& rstCsPkgReq, SC_PKG_PVE_SKIP_FIGHT_RSP& rstScPkgRsp)
{
    RESFIGHTLEVEL* poResFightLevel = CGameDataMgr::Instance().GetResFightLevelMgr().Find(rstCsPkgReq.m_dwPveLevelId);
    if (poResFightLevel == NULL)
    {
       LOGERR("Uin<%lu>, poResFightLevel is null dwLevelId<%d> ", poPlayerData->m_ullUin, rstCsPkgReq.m_dwPveLevelId);
       return ERR_SYS;
    }

    uint8_t bActivityType = poResFightLevel->m_bChapterId;
    uint32_t dwActivityRewardId = poResFightLevel->m_dwChapterRewardId;
    uint8_t bHardType = poResFightLevel->m_bSection - 1;

    if (bActivityType < 1 || bActivityType > MAX_ACTIVITY_PVE_NUM)
    {
       LOGERR("Uin<%lu>, bActivityType<%d> is out limit.",poPlayerData->m_ullUin, bActivityType);
       return ERR_SYS;
    }

    DT_ROLE_GUILD_INFO& rstGuildInfo = poPlayerData->GetGuildInfo();
    //DT_ROLE_MAJESTY_INFO& rstMajestyInfo = poPlayerData->GetMajestyInfo();
    DT_ROLE_MISC_INFO& rstMiscInfo = poPlayerData->GetMiscInfo();
    DT_ACTIVITY_INFO& rstActivityInfo = rstMiscInfo.m_astActivityInfo[bActivityType - 1];

    int iDouble = 1;
    if ((bActivityType == ACTIVITY_TYPE_GRAIN && TaskAct::Instance().IsActTypeOpen(poPlayerData, ACT_TYPE_DOUBLE_GRAIN))
        || (bActivityType == ACTIVITY_TYPE_ROUND && TaskAct::Instance().IsActTypeOpen(poPlayerData, ACT_TYPE_DOUBLE_ROUND)))
    {
        iDouble = 2;
    }

    // 活动
    RESACTIVITY* poResActivity = CGameDataMgr::Instance().GetResActivityMgr().Find(bActivityType);
    if (poResActivity == NULL)
    {
       LOGERR("Uin<%lu> poResActivity is null SeqId<%u> ", poPlayerData->m_ullUin, bActivityType);
       return ERR_SYS;
    }

    // 活动奖励
    RESACTIVITYREWARD* poResActivityReward = CGameDataMgr::Instance().GetResActivityRewardMgr().Find(dwActivityRewardId);

    if (NULL == poResActivityReward)
    {
       LOGERR("Uin<%lu>, poResActivityReward is null, SeqId<%u>", poPlayerData->m_ullUin, dwActivityRewardId);
       return ERR_SYS;
    }

    // 是否符合扫荡条件
    if (rstActivityInfo.m_szEvaluateLevelList[bHardType] < poResActivityReward->m_szEvaluateLevel[RES_MAX_ACTIVITY_EVALUATE_LEVEL - 1])
    {
       // 未完美通关，不可扫荡
       return ERR_NOT_SATISFY_COND;
    }

    int iTimelimit = bActivityType==ACTIVITY_TYPE_GOLD ? poResActivity->m_dwTimesLimit+rstGuildInfo.m_bGuildGoldTime : poResActivity->m_dwTimesLimit;
    if (rstActivityInfo.m_bFinishCnt + rstCsPkgReq.m_bSkipCount > iTimelimit)
    {
       return ERR_NOT_ENOUGH_SKIP_TIMES;
    }

    if (!AP::Instance().IsEnough(poPlayerData, (uint32_t)poResFightLevel->m_bAPConsume * rstCsPkgReq.m_bSkipCount))
    {
       return ERR_NOT_ENOUGH_AP;
    }

    //是否有足够的钻石
    if (!Consume::Instance().IsEnoughDiamond(poPlayerData, poResActivityReward->m_dwDiamondConsumCnt * rstCsPkgReq.m_bSkipCount))
    {
       return ERR_NOT_ENOUGH_DIAMOND;
    }

    //检查冷却时间，并重置
    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();
    if (ERR_NONE != CheckActivityPveCd(poPlayerData, bActivityType))
    {
       int iRet = ResetColdTime(poPlayerData, bActivityType, rstScPkgRsp.m_stSyncItemInfo);
       if (ERR_NONE!=iRet)
       {
         return iRet;
       }
    }

    for (int i=0; i<rstCsPkgReq.m_bSkipCount; i++)
    {
       DT_SYNC_ITEM_INFO& rstRewardItemInfo = rstScPkgRsp.m_astRewardItemInfo[i];
       rstRewardItemInfo.m_bSyncItemCount = 0;

       if (poResActivityReward->m_bBaseRewardType != 0)
       {
         // 基础奖励
         uint32_t dwBaseReward = rstActivityInfo.m_HistoryInfoList[0];

         // 基础奖励如果是金币奖励
         if (poResActivityReward->m_bBaseRewardType == ITEM_TYPE_GOLD)
         {
          //VIP等级会提高金币的收入，金币通过同步协议发送
          RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(poPlayerData->GetVIPLv() + 1);
          dwBaseReward += (uint32_t)(dwBaseReward * pResVip->m_dwGoldenBunos / 100.0);

          Item::Instance().RewardItem(poPlayerData, poResActivityReward->m_bBaseRewardType, poResActivityReward->m_dwBaseRewardId, dwBaseReward * iDouble,
              rstRewardItemInfo, METHOD_ACTIVITY_SKIP, Item::CONST_SHOW_PROPERTY_NORMAL);
         }
         else
         {
          Item::Instance().RewardItem(poPlayerData, poResActivityReward->m_bBaseRewardType, poResActivityReward->m_dwBaseRewardId, dwBaseReward * iDouble,
              rstRewardItemInfo, METHOD_ACTIVITY_SKIP, Item::CONST_SHOW_PROPERTY_NORMAL);
         }
       }

       // 宝箱奖励
       RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(poPlayerData->GetVIPLv() + 1);
       if (poResActivityReward->m_bBoxRewardType == ACTIVITY_BOXREWARD_LIST)
       {

       }
       else if (poResActivityReward->m_bBoxRewardType == ACTIVITY_BOXREWARD_SCORE)
       {
         for (uint32_t i = 0; i < poResActivityReward->m_bBoxRewardNum && i < RES_MAX_ACTIVITY_REWARD; i++)
         {
          uint32_t dwPondId = poResActivityReward->m_reward[i];
          if (dwPondId == 0)
          {
              // 没有奖励
              continue;
          }

          uint8_t bIndexStart = rstRewardItemInfo.m_bSyncItemCount;
          Lottery::Instance().DrawLotteryByPond(poPlayerData, dwPondId, rstRewardItemInfo, METHOD_ACTIVITY_SKIP);
          if (iDouble > 1)
          {
              //双倍奖励
              Lottery::Instance().DrawLotteryByPond(poPlayerData, dwPondId, rstRewardItemInfo, METHOD_ACTIVITY_PEV_SETTLE);
          }
          uint8_t bIndexEnd = rstRewardItemInfo.m_bSyncItemCount;

          if (pResVip != NULL && pResVip->m_dwGoldenBunos > 0)
          {
              for (; bIndexStart<bIndexEnd; bIndexStart++)
              {
                 if (rstRewardItemInfo.m_astSyncItemList[bIndexStart].m_bItemType == ITEM_TYPE_GOLD)
                 {
                   uint32_t dwGoldNum = rstRewardItemInfo.m_astSyncItemList[bIndexStart].m_iValueChg * pResVip->m_dwGoldenBunos / 100.0;
                   Item::Instance().RewardItem(poPlayerData, ITEM_TYPE_GOLD, 0, dwGoldNum,
                    rstRewardItemInfo, METHOD_ACTIVITY_SKIP, Item::CONST_SHOW_PROPERTY_NORMAL);
                 }
              }
          }
         }
       }

       // 经验奖励
       Item::Instance().RewardItem(poPlayerData, ITEM_TYPE_EXP, 0, poResActivityReward->m_dwExp, rstRewardItemInfo, METHOD_ACTIVITY_SKIP);
    }

    // 体力消耗
    Item::Instance().ConsumeItem(poPlayerData, ITEM_TYPE_AP, 0, -poResFightLevel->m_bAPConsume * rstCsPkgReq.m_bSkipCount, rstScPkgRsp.m_stSyncItemInfo, METHOD_ACTIVITY_SKIP);

    // 钻石消耗
    Item::Instance().ConsumeItem(poPlayerData, ITEM_TYPE_DIAMOND, 0, -poResActivityReward->m_dwDiamondConsumCnt * rstCsPkgReq.m_bSkipCount, rstScPkgRsp.m_stSyncItemInfo, METHOD_ACTIVITY_SKIP);

    // 任务计数
    Task::Instance().ModifyData(poPlayerData, TASK_VALUE_TYPE_ACTIVITY, rstCsPkgReq.m_bSkipCount/*value*/, 1/*para1*/, bActivityType/*活动Id*/);

    //记扫荡日志
    ZoneLog::Instance().WriteLevelSweepLog(poPlayerData, rstCsPkgReq.m_dwPveLevelId, rstCsPkgReq.m_bSkipCount);

    //次数更新
    rstActivityInfo.m_bFinishCnt += rstCsPkgReq.m_bSkipCount;

    rstScPkgRsp.m_bChallengeTimesTotal = rstActivityInfo.m_bFinishCnt;
    rstScPkgRsp.m_bChallengeTimesLeft = poResActivity->m_dwTimesLimit - rstScPkgRsp.m_bChallengeTimesTotal;

    _UpdateColdDownTime(poPlayerData, bActivityType, rstActivityInfo, ullCurTime, rstScPkgRsp.m_ullTimeStampColdDown);

    _SendSynMsg(poPlayerData, bActivityType, rstActivityInfo);

    return ERR_NONE;
}

int ActivityMgr::HandleMajestLvUp(PlayerData* poPlayerData)
{


    return 0;
}

int ActivityMgr::ResetCount(PlayerData* poPlayerData, CS_PKG_PVE_PURCHASE_TIMES_REQ& rstPurchaseTimesReq, SC_PKG_PVE_PURCHASE_TIMES_RSP& rstPurchaseTimesRsp)
{
    uint8_t bActivityType = rstPurchaseTimesReq.m_dwPveLevelId;
    if (bActivityType < 1 || bActivityType > MAX_ACTIVITY_PVE_NUM)
    {
       LOGERR("Uin<%lu>, bActivityType<%d> is out limit.",poPlayerData->m_ullUin, bActivityType);
       return ERR_SYS;
    }

    //DT_ROLE_MAJESTY_INFO& rstMajestyInfo = poPlayerData->GetMajestyInfo();
    DT_ROLE_MISC_INFO& rstMiscInfo = poPlayerData->GetMiscInfo();
    DT_ACTIVITY_INFO& rstActivityInfo = rstMiscInfo.m_astActivityInfo[bActivityType - 1];

    RESACTIVITY* poResActivity = CGameDataMgr::Instance().GetResActivityMgr().Find(bActivityType);
    if (poResActivity == NULL)
    {
        LOGERR("Uin<%lu> poResActivity is null!  SeqId<%u> ", poPlayerData->m_ullUin, bActivityType);
        return ERR_NOT_FOUND;
    }

    RESCONSUME* pResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(poResActivity->m_dwResetConsumeId);
    if (NULL == pResConsume)
    {
        LOGERR("Uin<%lu> RESCONSUME not find  ,bActivityType<%d>",poPlayerData->m_ullUin, bActivityType);
        return ERR_NOT_FOUND;
    }

    if (rstActivityInfo.m_bFinishCnt < poResActivity->m_dwTimesLimit)
    {
       //挑战次数不为0
        return ERR_PVE_PURCHASE_NOT_ZERO;
    }

    RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(poPlayerData->GetVIPLv() + 1);

    if (rstActivityInfo.m_bResetCnt >= pResVip->m_dwResetActivityTimes)
    {
       //重置次数已用完
        return ERR_PVE_RESET_TIMES_NOT_ENOUGH;
    }

    uint32_t dwConsumeDia = pResConsume->m_lvList[rstActivityInfo.m_bResetCnt];

    if (!Consume::Instance().IsEnoughDiamond(poPlayerData, dwConsumeDia))
    {
       //钻石不够
        return ERR_NOT_ENOUGH_DIAMOND;
    }

    Item::Instance().ConsumeItem(poPlayerData, ITEM_TYPE_DIAMOND, 0, -dwConsumeDia, rstPurchaseTimesRsp.m_stSyncItemInfo, DWLOG::METHOD_ACTIVITY_PEV_RESET_COUNT);

    rstActivityInfo.m_bResetCnt++;
    rstActivityInfo.m_bFinishCnt = 0;

    rstPurchaseTimesRsp.m_bType = rstPurchaseTimesReq.m_bType;
    rstPurchaseTimesRsp.m_dwPveLevelId = rstPurchaseTimesReq.m_dwPveLevelId;
    rstPurchaseTimesRsp.m_bChallengeTimesLeft = poResActivity->m_dwTimesLimit;

    LOGRUN("Uin<%lu> reset PVEACTIVITY ActId<%d>, consume dia<%u>, ResetCnt<%u>", poPlayerData->m_ullUin, bActivityType, dwConsumeDia, rstActivityInfo.m_bResetCnt);

    return ERR_NONE;
}

int ActivityMgr::CheckActivityPveCd(PlayerData* poPlayerData, uint32_t dwActivityType)
{
    uint8_t bActivityType = dwActivityType;
    if (bActivityType < 1 || bActivityType > MAX_ACTIVITY_PVE_NUM)
    {
       LOGERR("Uin<%lu>, bActivityType<%d> is out limit.",poPlayerData->m_ullUin, bActivityType);
       return ERR_SYS;
    }

    //DT_ROLE_MAJESTY_INFO& rstMajestyInfo = poPlayerData->GetMajestyInfo();
    DT_ROLE_MISC_INFO& rstMiscInfo = poPlayerData->GetMiscInfo();
    DT_ACTIVITY_INFO& rstActivityInfo = rstMiscInfo.m_astActivityInfo[bActivityType - 1];

    uint64_t ullCurTimeSec = CGameTime::Instance().GetCurrSecond();
    if (ullCurTimeSec < rstActivityInfo.m_ullCdTimeStamp)
    {
       return ERR_IN_COLD;
    }

    return ERR_NONE;
}

int ActivityMgr::ResetColdTime(PlayerData* poPlayerData, uint32_t dwActivityType, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    uint8_t bActivityType = dwActivityType;
    if (bActivityType < 1 || bActivityType > MAX_ACTIVITY_PVE_NUM)
    {
       LOGERR("Uin<%lu>, bActivityType<%d> is out limit.",poPlayerData->m_ullUin, bActivityType);
       return ERR_SYS;
    }

    //DT_ROLE_MAJESTY_INFO& rstMajestyInfo = poPlayerData->GetMajestyInfo();
    DT_ROLE_MISC_INFO& rstMiscInfo = poPlayerData->GetMiscInfo();
    DT_ACTIVITY_INFO& rstActivityInfo = rstMiscInfo.m_astActivityInfo[bActivityType - 1];

    switch (bActivityType)
    {
    case ACTIVITY_TYPE_ROUND:
       if(!Consume::Instance().IsEnough(poPlayerData, ITEM_TYPE_DIAMOND, m_iColdResetCostDiaRound))
       {
         return ERR_NOT_ENOUGH_DIAMOND;
       }
       Item::Instance().ConsumeItem(poPlayerData, ITEM_TYPE_DIAMOND, 0, -m_iColdResetCostDiaRound, rstSyncItemInfo, METHOD_RESET_CD_TIME);
       break;

    case ACTIVITY_TYPE_GRAIN:
       if(!Consume::Instance().IsEnough(poPlayerData, ITEM_TYPE_DIAMOND, m_iColdResetCostDiaGrain))
       {
         return ERR_NOT_ENOUGH_DIAMOND;
       }
       Item::Instance().ConsumeItem(poPlayerData, ITEM_TYPE_DIAMOND, 0, -m_iColdResetCostDiaGrain, rstSyncItemInfo, METHOD_RESET_CD_TIME);
       break;

    case ACTIVITY_TYPE_GOLD:
       if(!Consume::Instance().IsEnough(poPlayerData, ITEM_TYPE_DIAMOND, m_iColdResetCostDiaGold))
       {
         return ERR_NOT_ENOUGH_DIAMOND;
       }
       Item::Instance().ConsumeItem(poPlayerData, ITEM_TYPE_DIAMOND, 0, -m_iColdResetCostDiaGold, rstSyncItemInfo, METHOD_RESET_CD_TIME);
       break;

    default:
       return ERR_WRONG_PARA;
    }

    rstActivityInfo.m_ullCdTimeStamp = 0;
    return ERR_NONE;
}

void ActivityMgr::_UpdateColdDownTime(PlayerData* poPlayerData, int iActivityType, DT_ACTIVITY_INFO& rstActivityInfo, uint64_t m_ullCdStartTimeStamp, uint64_t& m_ullColdDowmTimeStamp)
{
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = poPlayerData->GetMajestyInfo();

    switch (iActivityType)
    {
    case ACTIVITY_TYPE_ROUND:
       m_ullColdDowmTimeStamp = m_ullCdStartTimeStamp + m_ullColdTimeRoundList[rstMajestyInfo.m_bVipLv];
       rstActivityInfo.m_ullCdTimeStamp = m_ullColdDowmTimeStamp;
       break;
    case ACTIVITY_TYPE_GRAIN:
       m_ullColdDowmTimeStamp = m_ullCdStartTimeStamp + m_ullColdTimeGrainList[rstMajestyInfo.m_bVipLv];
       rstActivityInfo.m_ullCdTimeStamp = m_ullColdDowmTimeStamp;
       break;
    case ACTIVITY_TYPE_GOLD:
       //m_ullColdDowmTimeStamp = m_ullCdStartTimeStamp + m_ullColdTimeGoldList[rstMajestyInfo.m_bVipLv];
       //rstActivityInfo.m_ullCdTimeStamp = m_ullColdDowmTimeStamp;
       m_ullColdDowmTimeStamp = 0;
       rstActivityInfo.m_ullCdTimeStamp = 0; //策划表示炮夺钱饷不要cd了
       break;
    default:
       break;
    }
}

void ActivityMgr::_SendSynMsg(PlayerData* poPlayerData, int iActivityType, DT_ACTIVITY_INFO& rstActivityInfo)
{
    //DT_ROLE_MISC_INFO& rstMiscInfo = poPlayerData->GetMiscInfo();

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PVE_ACTIVITY_SYN;
    SC_PKG_PVE_ACTIVITY_SYN& rstPveActivitySyn = m_stScPkg.m_stBody.m_stPveActivitySyn;
    rstPveActivitySyn.m_bActivityType = iActivityType;
    rstPveActivitySyn.m_stActivityInfo = rstActivityInfo;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayerData->m_pOwner, &m_stScPkg);
}

void ActivityMgr::_SendLevelEvent(PlayerData* poPlayerData, uint32_t dwLevelId, uint8_t bType, uint32_t dwValue)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_LEVEL_EVENT_NTF;
    m_stSsPkg.m_stHead.m_ullUin = poPlayerData->GetRoleBaseInfo().m_ullUin;

    SS_PKG_GUILD_LEVEL_EVENT_NTF& rstNtf = m_stSsPkg.m_stBody.m_stGuildLevelEventNtf;
    rstNtf.m_dwLevelId = dwLevelId;
    rstNtf.m_bType = bType;
    rstNtf.m_dwValue = dwValue;

    DT_ROLE_GUILD_INFO& rstGuildInfo = poPlayerData->GetGuildInfo();
    rstNtf.m_ullGuildId = rstGuildInfo.m_ullGuildId;

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
}

