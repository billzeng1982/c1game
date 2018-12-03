#include <list>
#include "Lottery.h"
#include "../gamedata/GameDataMgr.h"
#include "../../common/utils/FakeRandom.h"
#include "../../common/sys/GameTime.h"
#include "../../../common/log/LogMacros.h"
#include "Equip.h"
#include "GeneralCard.h"
#include "Props.h"
#include "Item.h"
#include "Consume.h"
#include "player/Player.h"
#include "dwlog_svr.h"
#include "ZoneLog.h"
#include "Task.h"
#include "TimeCycle.h"
#include "TaskAct.h"
#include "Message.h"


using namespace std;
using namespace PKGMETA;
using namespace DWLOG;



#define DISCOUNT_ITEM_INDEX (8001)
//新增的觉醒商店单抽独立奖池
//由于规则改变，觉醒商店单抽需要在10连抽的奖池中抽取一件物品，并且在单抽的独立奖池中抽取一件物品
#define AWAKE_EQUIP_DRAW_ONE_POUND (60)
#define AWAKE_EQUIP_DRAW_TEN_POUND (61)


int RandomNodeFindCmp(const void *pstFirst, const void *pstSecond)
{
    RandomNode* pstNodeFirst = (RandomNode*)pstFirst;
    RandomNode* pstNodeSecond = (RandomNode*)pstSecond;
    //high比high大,值大, Low比low小,值小,  交叉就相等了,应为查找模块low和high都会设置相同
    if (pstNodeFirst->m_dwHighRange > pstNodeSecond->m_dwHighRange)
    {
        return 1;
    }
    else if (pstNodeFirst->m_dwLowRange < pstNodeSecond->m_dwLowRange)
    {
        return -1;
    }

    return 0;
}

Lottery::Lottery()
{

}

Lottery::~Lottery()
{
    this->_Clear();
}

bool Lottery::Init()
{
    LOGRUN("init lottery info");
    LOGRUN("---------------------------------------------------");

    this->_Clear();
    if (   this->_InitDrawType()
        && this->_InitNewLottery()
        && this->_InitRule()
        && this->_InitFreeCond())
    {
        return true;
    }
    //_RandDiaDrawPond();
    LOGRUN("---------------------------------------------------");

    return false;
}

bool Lottery::_InitDrawType()
{
    ResLotteryDrawTypeMgr_t& rstResLotteryDrawTypeMgr = CGameDataMgr::Instance().GetResLotteryDrawTypeMgr();
    int iSumCount = rstResLotteryDrawTypeMgr.GetResNum();
    for (int i=0; i<iSumCount; i++)
    {
        RESLOTTERYDRAWTYPE* pstResRecord = rstResLotteryDrawTypeMgr.GetResByPos(i);
        if (pstResRecord)
        {
            m_stDrawType2PondIdMap.insert(std::pair<uint32_t, uint32_t>(pstResRecord->m_dwId, pstResRecord->m_dwDefaultPondId));
        }

        m_stPrice.m_bCount = iSumCount;
        m_stPrice.m_astLotteryPrice[i].m_bItemType = pstResRecord->m_bConsumeItemType;
        m_stPrice.m_astLotteryPrice[i].m_dwItemId = pstResRecord->m_dwConsumeItemId;
        m_stPrice.m_astLotteryPrice[i].m_dwItemNum = pstResRecord->m_dwConsumeItemNum;
    }

    return true;
}





bool Lottery::_InitRule()
{
    ResLotteryRuleMgr_t& rstResLotteryRuleMgr = CGameDataMgr::Instance().GetResLotteryRuleMgr();

    int iCount = rstResLotteryRuleMgr.GetResNum();
    RESLOTTERYRULE* pstResRecord = NULL;
    m_dwActDrawMaxNumOfGodCard = 0;
    for (int i=0; i<iCount; i++)
    {
        // 这里根据数据档硬编码
        pstResRecord = rstResLotteryRuleMgr.GetResByPos(i);
        assert(pstResRecord);
        if (pstResRecord->m_dwDrawTypeId == LOTTERY_DRAW_TYPE_DIAMOND)
        {
            if (pstResRecord->m_bIsDrawTens)
            {
                m_stFirstDrawTensToPondIdMap[pstResRecord->m_wAccumuCnt] = pstResRecord->m_dwPondId;
            }
            else
            {
                m_dwFirstDrawNumToPondIdMap[pstResRecord->m_wAccumuCnt] = pstResRecord->m_dwPondId;
            }
        }
        else if (pstResRecord->m_dwDrawTypeId == LOTTERY_DRAW_TYPE_ACT_CNT)
        {
            //活动抽奖 浮动奖池映射
            m_stActCntDrawPondFloatMap[pstResRecord->m_wAccumuCnt] = pstResRecord->m_dwPondId;
        }
        else if (pstResRecord->m_dwDrawTypeId == LOTTERY_DRAW_TYPE_ACT) 
        {
            //单抽浮动
            m_stActOneDrawPondFloatMap[pstResRecord->m_wAccumuCnt] = pstResRecord->m_dwPondId;
        }
        else if (pstResRecord->m_dwDrawTypeId == LOTTERY_DRAW_TYPE_ACT_CNT_COMMON)
        {
            //活动抽奖 浮动奖池映射 通用版
            m_stActCntCommonDrawPondFloatMap[pstResRecord->m_wAccumuCnt] = pstResRecord->m_dwPondId;
        }
        else if (pstResRecord->m_dwDrawTypeId == LOTTERY_DRAW_TYPE_ACT_COMMON)
        {
            //单抽浮动  通用版
            m_stActOneCommonDrawPondFloatMap[pstResRecord->m_wAccumuCnt] = pstResRecord->m_dwPondId;
        }
		else if (pstResRecord->m_dwDrawTypeId == LOTTERY_DRAW_TYPE_ACT_FMONTH)
		{
			m_stActOneFMonthDrawPondFloatMap[pstResRecord->m_wAccumuCnt] = pstResRecord->m_dwPondId;
		}
		else if (pstResRecord->m_dwDrawTypeId == LOTTERY_DRAW_TYPE_ACT_CNT_FMONTH)
		{
			m_stActCntFMonthDrawPondFloatMap[pstResRecord->m_wAccumuCnt] = pstResRecord->m_dwPondId;
		}
        else if (pstResRecord->m_dwDrawTypeId == LOTTERY_DRAW_TYPE_EQUIP)
        {
            continue;
        }
        else
        {
            continue;
        }
    }

    ResDiaLotteryRuleMgr_t& rstResDiaLotteryMgr = CGameDataMgr::Instance().GetResDiaLotteryRuleMgr();
    uint32_t dwResSize = rstResDiaLotteryMgr.GetResNum();
    RESDIALOTTERYRULE* pstRecord = NULL;
    m_dwFactMax = 1;
    for (uint32_t i = 0; i < dwResSize; i++)
    {
        pstRecord = rstResDiaLotteryMgr.GetResByPos(i);
        if (!pstRecord)
        {
            LOGERR("FATA ERROR: pstRecord ID<%d> is null", i+1);
            return false;
        }
        if ( i < MAX_LOTTERY_CNT_NUM)
        {
            m_dwFactMax *= i+1;
            m_dwDiaDrawPondIdTen[i] = pstRecord->m_dwPondId;
        }
		else if( i < 2 * MAX_LOTTERY_CNT_NUM ) //后十行为金币相关
		{
            m_dwGoldDrawPondIdTen[i - MAX_LOTTERY_CNT_NUM] = pstRecord->m_dwPondId;
		}
        else if (i < 3 * MAX_LOTTERY_CNT_NUM) //活动相关
        {
            m_dwActDrawPondIdTen[i - 2 * MAX_LOTTERY_CNT_NUM] = pstRecord->m_dwPondId;
        }
        else if (i < 4 * MAX_LOTTERY_CNT_NUM) //活动相关
        {
            m_dwActCommonDrawPondIdTen[i - 3 * MAX_LOTTERY_CNT_NUM] = pstRecord->m_dwPondId;
        }
		else if (i < 5 * MAX_LOTTERY_CNT_NUM)
		{
			m_dwActFMonthDrawPondIdTen[i - 4 * MAX_LOTTERY_CNT_NUM] = pstRecord->m_dwPondId;
		}
        else if (i < 6 * MAX_LOTTERY_CNT_NUM) //觉醒商店
        {
            m_dwAwakeEquipDrawPondIdTen[i - 5 * MAX_LOTTERY_CNT_NUM] = pstRecord->m_dwPondId;
        }
        else if (i == AWAKE_EQUIP_DRAW_ONE_POUND)
        {
            m_dwAwakeEquipDrawPondIdOne = pstRecord->m_dwPondId;
        }
        else if (i == AWAKE_EQUIP_DRAW_TEN_POUND)
        {
            m_dwAwakeEquipDrawPondId = pstRecord->m_dwPondId;
        }
    }


	RESBASIC* pstResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(DISCOUNT_ITEM_INDEX);
    assert(pstResBasic);
	m_dwDiscount = pstResBasic->m_para[0];

    return true;

}

bool Lottery::_InitFreeCond()
{
    ResLotteryFreeCondMgr_t& rstResLotteryFreeCondMgr = CGameDataMgr::Instance().GetResLotteryFreeCondMgr();

    int iCount = rstResLotteryFreeCondMgr.GetResNum();
    RESLOTTERYFREECOND* pstResRecord = NULL;
    for (int i=0; i<iCount; i++)
    {
        pstResRecord = rstResLotteryFreeCondMgr.GetResByPos(i);
        if (!pstResRecord)
        {
            LOGERR("_InitFreeCond:pstResRecord is null");
            return false;
        }

        if (pstResRecord->m_dwDrawTypeId == LOTTERY_DRAW_TYPE_GOLD)
        {
            m_dwGoldFreeCountByDay = (uint32_t)pstResRecord->m_bGoldDrawMaxNum;
            m_dwGoldFreeIntervalSec = pstResRecord->m_wIntervalMinutes1 * 60;
            LOGRUN("_InitFreeCond m_dwGoldFreeIntervalSec = %u", m_dwGoldFreeIntervalSec);
        }
        else if (pstResRecord->m_dwDrawTypeId == LOTTERY_DRAW_TYPE_DIAMOND)
        {
            m_dwDiamondFreeInterval1Sec = pstResRecord->m_wIntervalMinutes1 * 60;
            m_dwDiamondFreeInterval2Sec = pstResRecord->m_wIntervalMinutes2 * 60;
            LOGRUN("_InitFreeCond m_dwDiamondFreeInterval1Sec = %u", m_dwDiamondFreeInterval1Sec);
        }
        else if (pstResRecord->m_dwDrawTypeId == LOTTERY_DRAW_TYPE_EQUIP)
        {
            m_dwAwakeEquipFreeInterval1Sec = pstResRecord->m_wIntervalMinutes1 * 60;
            m_dwAwakeEquipFreeInterval2Sec = pstResRecord->m_wIntervalMinutes2 * 60;
            LOGRUN("_InitFreeCond m_dwAwakeEquipFreeInterval1Sec = %u", m_dwDiamondFreeInterval1Sec);
        }
    }

	ResBasicMgr_t& rResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
	RESBASIC* poBasic = rResBasicMgr.Find(COMMON_UPDATE_TIME);
	if (!poBasic)
	{
		LOGERR("_InitFreeCond:poBasic is null");
		return false;
	}
	m_iDailyResetTime = poBasic->m_para[0];

    return true;
}


bool Lottery::_InitNewLottery()
{
    RESBASIC* pstResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find((int)BASIC_LOTTERY_DEFAULT_ITEM);
    assert(pstResBasic);
    m_stDefaultItem.m_bItemType = (uint8_t)pstResBasic->m_para[0]; // 物品类型
    m_stDefaultItem.m_dwItemId = (uint32_t)pstResBasic->m_para[1]; // 物品ID
    m_stDefaultItem.m_iValueChg = (int)pstResBasic->m_para[2]; // 物品变化量

    pstResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find((int)BASIC_ACT_DRAW);
    assert(pstResBasic);
    m_dwActDarwTimeId = (uint32_t)pstResBasic->m_para[0];
    m_dwActDarwLvLimit = (uint32_t)pstResBasic->m_para[1];
    m_dwActDarwVipLimit = (uint32_t)pstResBasic->m_para[2];
	m_dwActFMonthDarwActId = (uint32_t)pstResBasic->m_para[3];

    ResPondLotteryMapMgr_t& rstResPondLotteryMapMgr = CGameDataMgr::Instance().GetResPondLotteryMapMgr();
    ResLotteryBonusMapMgr_t& rstResLotteryBonusMapMgr = CGameDataMgr::Instance().GetResLotteryBonusMapMgr();
    int iResPondCount = rstResPondLotteryMapMgr.GetResNum();
    int iResBonusCount = rstResLotteryBonusMapMgr.GetResNum();
    if (!iResPondCount || !iResBonusCount)
    {
        LOGERR("iResPondCount or  iResBonusCount  is 0");
        return false;
    }

    /*
    处理生成实际随机时使用的Bonus概率区间
    遍历rstResLotteryBonusMapMgr，得出Bonus的概率
    再遍历rstResPondLotteryMapMgr，得出每个奖池下的所有Bonus的概率，然后转化为程序随机时使用的概率区间
    某个奖池下Bonus实际概率=Lottery概率 * Bonus概率
    单条目概率=(上个条目的范围-减去当前条目的范围) / MAX_LOTTERY_PROBABILITY

    */
    vector<Bonus> oBonusVector(iResBonusCount);
    map<uint32_t, vector<Bonus*> > oLotteryId2BonusVectorMap;
    map<uint32_t, vector<Bonus*> >::iterator oLotteryId2BonusVectorIter;
    vector<Bonus*> pBonusTmpVector;
    uint32_t dwLastRange = 0;
    for (int iIndex = 0; iIndex < iResBonusCount; iIndex++)
    {
        RESLOTTERYBONUSMAP* pResRecond = rstResLotteryBonusMapMgr.GetResByPos(iIndex);
        if (!pResRecond)
        {
            LOGERR("RESLOTTERYBONUSMAP<index %u> is NULL", iIndex);
            return false;
        }

        oBonusVector[iIndex].m_dwLotteryId = pResRecond->m_dwLotteryId;
        oBonusVector[iIndex].m_dwResId = pResRecond->m_dwId;
        oBonusVector[iIndex].m_ullRate = pResRecond->m_dwProbability - dwLastRange;
        dwLastRange = pResRecond->m_dwProbability;
        pBonusTmpVector.push_back(&oBonusVector[iIndex]);
        if (dwLastRange == MAX_LOTTERY_PROBABILITY)
        {
            dwLastRange = 0;
            oLotteryId2BonusVectorMap.insert(make_pair(pResRecond->m_dwLotteryId, pBonusTmpVector));
            pBonusTmpVector.clear();
        }

    }


    RESPONDLOTTERYMAP* pResPondLottery = NULL;
    m_oPondId2RandomNodeVectMap.clear();
    map<uint32_t, vector<RandomNode> >::iterator PondIdRandomMapIter;
    RandomNode oRandomNodeTmp;
    uint32_t dwLastPondLotteryRange = 0;    //上个奖池Id的最大范围
    uint32_t dwLastHighRange = 0;
    for (int iIndex = 0 ; iIndex < iResPondCount; iIndex++)
    {
        pResPondLottery = rstResPondLotteryMapMgr.GetResByPos(iIndex);
        if (!pResPondLottery)
        {
            LOGERR("pResPondLottery<iIndex %d> is NULL", iIndex);
            return false;
        }
        oLotteryId2BonusVectorIter = oLotteryId2BonusVectorMap.find(pResPondLottery->m_dwLotteryId);
        if (oLotteryId2BonusVectorIter == oLotteryId2BonusVectorMap.end())
        {
            LOGERR("can't find the LotteryId <%d> from m_oBonusId2BonusVectMap", pResPondLottery->m_dwLotteryId);
            return false;
        }
        if (dwLastPondLotteryRange == 0)
        {
            //初始第一个条目
            vector<RandomNode> tmpVector;
            pair< map<uint32_t, vector<RandomNode> >::iterator, bool > ret =
                m_oPondId2RandomNodeVectMap.insert(make_pair(pResPondLottery->m_dwPondId, tmpVector));
            if (!ret.second)
            {
                LOGERR("insert pResPondLottery->m_dwPondId<%u> error", pResPondLottery->m_dwLotteryId);
                return false;
            }
            PondIdRandomMapIter = ret.first;
        }
        vector<RandomNode> & RandomNodeVector = PondIdRandomMapIter->second;
        //LOGWARN("Init LotteryNew:PondId<%u> Pro<%u> LotteryId<%u>", pResPondLottery->m_dwPondId, pResPondLottery->m_dwProbability, pResPondLottery->m_dwLotteryId);
        for (vector<Bonus*>::iterator iter = oLotteryId2BonusVectorIter->second.begin();
            iter != oLotteryId2BonusVectorIter->second.end(); iter++)
        {
            oRandomNodeTmp.m_dwValue = (*iter)->m_dwResId;    //实际抽中的BonusId
            //实际概率调整为概率区间
            oRandomNodeTmp.m_dwLowRange = dwLastHighRange + 1;
            oRandomNodeTmp.m_dwHighRange = (pResPondLottery->m_dwProbability - dwLastPondLotteryRange) * (*iter)->m_ullRate * CONST_PROBABILITY_REDUCE_RATE
                * CONST_PROBABILITY_REDUCE_RATE + dwLastHighRange;

            RandomNodeVector.push_back(oRandomNodeTmp);
            dwLastHighRange = oRandomNodeTmp.m_dwHighRange;
            //LOGWARN("InitLottery:PondId<%u>, LotteryId<%u>, BonusId<%u>, Low<%u> High<%u>", pResPondLottery->m_dwPondId, pResPondLottery->m_dwLotteryId,
            //    oRandomNodeTmp.m_dwValue, oRandomNodeTmp.m_dwLowRange, oRandomNodeTmp.m_dwHighRange);
        }
        dwLastPondLotteryRange = pResPondLottery->m_dwProbability;
        if (pResPondLottery->m_dwProbability == MAX_LOTTERY_PROBABILITY)
        {
            //iIndex为同奖池ID的最后一个条目
            //因为精度问题可能会导致概率不完全覆盖,强制覆盖
            RandomNodeVector.back().m_dwHighRange = CONST_REAL_MAX_PROBABILITY;
            dwLastPondLotteryRange = 0;
            dwLastHighRange = 0;
            //LOGWARN("====Same PondId<%u> end====", pResPondLottery->m_dwPondId);
        }

    }
    return true;
}

int Lottery::_Clear()
{

    for (map<uint32_t, vector<RandomNode> >::iterator iterRandom = m_oPondId2RandomNodeVectMap.begin();
        iterRandom != m_oPondId2RandomNodeVectMap.end(); iterRandom++)
    {
        iterRandom->second.clear();
    }
    m_oPondId2RandomNodeVectMap.clear();

    m_stDrawType2PondIdMap.clear();


    m_dwFirstDrawNumToPondIdMap.clear();
	m_stFirstDrawTensToPondIdMap.clear();
    m_stActCntDrawPondFloatMap.clear();
    m_stActOneDrawPondFloatMap.clear();

    bzero(&m_stPrice, sizeof(DT_LOTTERY_PRICE));

    m_bCheckFree = false;
    return 0;
}

int Lottery::_CheckEnoughMoney(PlayerData* pstData, uint32_t dwNeedGold, uint32_t dwNeedDiamond)
{
    if (!Consume::Instance().IsEnoughGold(pstData, dwNeedGold))
    {
        return ERR_NOT_ENOUGH_GOLD;
    }

    if (!Consume::Instance().IsEnoughDiamond(pstData, dwNeedDiamond))
    {
        return ERR_NOT_ENOUGH_DIAMOND;
    }

    return ERR_NONE;
}

int Lottery::_CheckFreeCond(PlayerData* pstData, uint8_t bDrawType)
{
    m_bCheckFree = false;
    DT_ROLE_MISC_INFO& rstInfo = pstData->GetMiscInfo();
    DT_ROLE_BASE_INFO& rstBaseInfo = pstData->GetRoleBaseInfo();

    int64_t llCurTime = (int64_t)CGameTime::Instance().GetCurrSecond();
    uint32_t dwIntervalSec = 0;

    switch(bDrawType)
    {
    case LOTTERY_DRAW_TYPE_GOLD:
		{
			if ((uint32_t)rstInfo.m_chFreeByGCount >= m_dwGoldFreeCountByDay)
				return -1;

			if (llCurTime >= (rstInfo.m_llFreeByGLastTime + (int64_t)m_dwGoldFreeIntervalSec))
			{
				rstInfo.m_llFreeByGLastTime = llCurTime;
				rstInfo.m_chFreeByGCount++;
				m_bCheckFree = true;
				return 0;
			}
		}
        break;

    case LOTTERY_DRAW_TYPE_DIAMOND:
		{
			if (rstInfo.m_chFreeByDCount >= 1)
				return -1;

			dwIntervalSec = m_dwDiamondFreeInterval1Sec;
			if ((llCurTime - rstBaseInfo.m_llFirstLoginTime) > (SECONDS_OF_DAY *3) )
			{
				dwIntervalSec = m_dwDiamondFreeInterval2Sec;
			}

			if (llCurTime >= (rstInfo.m_llFreeByDLastTime + (int64_t)dwIntervalSec))
			{
				rstInfo.m_llFreeByDLastTime = llCurTime;
				rstInfo.m_chFreeByDCount++;
				m_bCheckFree = true;
				return 0;
			}
		}
        break;
    case LOTTERY_DRAW_TYPE_EQUIP:
        {
            if (rstInfo.m_dwFreeDrawByAwakeEquip >= 1)
                return -1;

            dwIntervalSec = ((llCurTime - rstBaseInfo.m_llFirstLoginTime) > (SECONDS_OF_DAY * 3)) ? 
                m_dwDiamondFreeInterval2Sec : m_dwAwakeEquipFreeInterval1Sec;
            if (llCurTime >= (rstInfo.m_llFreeByAwakeEquipLastTime + (int64_t)dwIntervalSec))
            {
                rstInfo.m_llFreeByAwakeEquipLastTime = llCurTime;
                rstInfo.m_dwFreeDrawByAwakeEquip++;
                m_bCheckFree = true;
                return 0;
            }
        }
    default:
        return -1;
    }
    return 0;
}

bool Lottery::_CheckRule(PlayerData* pstData, uint8_t bDrawType, uint32_t& rdwPondId)
{
    switch(bDrawType)
    {
    case LOTTERY_DRAW_TYPE_GOLD:
        return _CheckRuleGold(pstData, rdwPondId);
    case LOTTERY_DRAW_TYPE_DIAMOND:
        return _CheckRuleDiamond(pstData, rdwPondId);
    case LOTTERY_DRAW_TYPE_DIAMOND_CNT:
        return _CheckRuleDiamondCnt(pstData, rdwPondId);
    default:
        return false;
    }

    return false;
}

bool Lottery::_CheckRuleGold(PlayerData* pstData, uint32_t& rdwPondId)
{
    return false;
}

//钻石单抽首次按特殊规则处理
bool Lottery::_CheckRuleDiamond(PlayerData* pstData, uint32_t& rdwPondId)
{
	m_dwFirstDrawNumToPondIdMapIter = m_dwFirstDrawNumToPondIdMap.find(pstData->GetMiscInfo().m_dwDrawByDCount1 + 1);
	if(m_dwFirstDrawNumToPondIdMap.end()==m_dwFirstDrawNumToPondIdMapIter)
	{
		return false;
	}
	else
	{
		rdwPondId = m_dwFirstDrawNumToPondIdMapIter->second;
		return true;
	}
}

bool Lottery::_CheckRuleDiamondCnt(PlayerData* pstData, uint32_t& rdwPondId)
{
    // 十连抽保底，通过替换最后一次的抽奖池实现，这里不处理
	m_stFirstDrawTensToPondIdMapIter = m_stFirstDrawTensToPondIdMap.find(pstData->GetMiscInfo().m_dwDrawByDCntCount+1);
	if(m_stFirstDrawTensToPondIdMap.end()==m_stFirstDrawTensToPondIdMapIter)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void Lottery::HandleLotteryInfo(PlayerData* pstData, SCPKG& rstScPkg)
{
    rstScPkg.m_stHead.m_wMsgId = SC_MSG_LOTTERY_INFO_RSP;
    SC_PKG_LOTTERY_INFO_RSP& rstScPkgBody = rstScPkg.m_stBody.m_stLotteryInfoRsp;
    DT_ROLE_MISC_INFO& rstInfo = pstData->GetMiscInfo();
    DT_ROLE_BASE_INFO& rstBaseInfo = pstData->GetRoleBaseInfo();


	rstScPkgBody.m_dwFreeDrawGoldNextTimeSec = rstInfo.m_llFreeByGLastTime + m_dwGoldFreeIntervalSec;  //免费金币抽下次抽的时间
    rstScPkgBody.m_dwFreeDrawGoldMaxCnt = m_dwGoldFreeCountByDay;
	rstScPkgBody.m_dwFreeDrawGoldLeftCnt =
    ((long)m_dwGoldFreeCountByDay >= rstInfo.m_chFreeByGCount) ? (m_dwGoldFreeCountByDay -rstInfo.m_chFreeByGCount) : 0;  //免费金币抽剩余次数

	int64_t llCurTime = (int64_t)CGameTime::Instance().GetCurrSecond();
	uint32_t dwIntervalSec = m_dwDiamondFreeInterval1Sec;
	if ((llCurTime - rstBaseInfo.m_llFirstLoginTime) > (SECONDS_OF_DAY*3) )
	{
		dwIntervalSec = m_dwDiamondFreeInterval2Sec;
	}
	rstScPkgBody.m_dwFreeDrawDiamondNextTimeSec = rstInfo.m_llFreeByDLastTime + dwIntervalSec;			// 免费钻石抽下次抽的时间

    dwIntervalSec = m_dwAwakeEquipFreeInterval1Sec;
    if ((llCurTime - rstBaseInfo.m_llFirstLoginTime) > (SECONDS_OF_DAY * 3))
    {
        dwIntervalSec = m_dwAwakeEquipFreeInterval2Sec;
    }
    rstScPkgBody.m_dwFreeDrawAwakeNextTimeSec = rstInfo.m_llFreeByAwakeEquipLastTime + dwIntervalSec;


	if(!CGameTime::Instance().IsInSameDay((time_t)llCurTime, rstInfo.m_llCostDiaLastTime))    //折扣是否已使用
	{
		rstScPkgBody.m_bIsDiscountUsed = 0;
	}
	else
	{
		rstScPkgBody.m_bIsDiscountUsed = 1;
	}

	//还剩几次必出紫将
	for(m_dwFirstDrawNumToPondIdMapIter = m_dwFirstDrawNumToPondIdMap.begin();m_dwFirstDrawNumToPondIdMapIter != m_dwFirstDrawNumToPondIdMap.end(); m_dwFirstDrawNumToPondIdMapIter++)
	{
		if(rstInfo.m_dwDrawByDCount1<m_dwFirstDrawNumToPondIdMapIter->first)
		{
			rstScPkgBody.m_bPurpleDiaDrawTimes = m_dwFirstDrawNumToPondIdMapIter->first - rstInfo.m_dwDrawByDCount1 - 1;
			break;
		}
	}
	if(m_dwFirstDrawNumToPondIdMapIter == m_dwFirstDrawNumToPondIdMap.end())
	{
		rstScPkgBody.m_bPurpleDiaDrawTimes = MAX_LOTTERY_CNT_NUM - rstInfo.m_dwDrawByDCount1%MAX_LOTTERY_CNT_NUM - 1;
	}

    rstScPkgBody.m_stLotteryDrawPrice = m_stPrice;
    rstScPkgBody.m_dwDrawActCount = rstInfo.m_dwDrawActCount;
	rstScPkgBody.m_bIsFirstDrawDiaCntUsed = 0== rstInfo.m_dwDrawByDCntCount ? 0 : 1;

    LOGRUN("Uin<%lu> freegoldtime=%u, freediamondtime=%u", pstData->m_ullUin, rstScPkgBody.m_dwFreeDrawGoldNextTimeSec, rstScPkgBody.m_dwFreeDrawDiamondNextTimeSec);
    /*LOGRUN("Uin<%lu> freeCntLeft=%u, gold=%u, goldcnt=%u, diamond=%u, diamondcnt=%u", pstData->m_ullUin, rstScPkgBody.m_dwFreeDrawGoldLeftCnt,
        rstScPkgBody.m_dwDrawGoldPrice, rstScPkgBody.m_dwDrawGoldCntPrice, rstScPkgBody.m_dwDrawDiamondPrice, rstScPkgBody.m_dwDrawDiamondCntPrice);*/

    return;
}

int Lottery::HandleLotteryDrawByNormal(PlayerData* pstData, uint8_t bDrawType, uint8_t bIsFree, SC_PKG_LOTTERY_DRAW_RSP& rstScPkgBodyRsp)
{
    //参数检查
    int iCount = 0;
	
	uint8_t bConsumeType = 0;
	int32_t iConsumeNum = 0;
	uint32_t dwTicketId = 0;
	int32_t iTicketNum = 0;
    if (GetParaFromDrawType(bDrawType, &iCount, &bConsumeType, &iConsumeNum, &dwTicketId) != 0)
    {
        LOGERR("Uin<%lu> bDrawType<%hhu> error", pstData->m_ullUin, bDrawType);
        return ERR_LOTTERY_DRAW_TYPE;
    }

    //消费数据检查

	time_t lCurTime = CGameTime::Instance().GetCurrSecond();
	bool bIsDiscountUsed = CGameTime::Instance().IsInSameDay(lCurTime, pstData->GetMiscInfo().m_llCostDiaLastTime);

	if (LOTTERY_DRAW_TYPE_DIAMOND == bDrawType && !bIsDiscountUsed)
	{
		//只有钻石单抽 会打折
		iConsumeNum = m_dwDiscount * iConsumeNum / 100 ;
	}

	

    if (bIsFree == 1)
    {
        if (_CheckFreeCond(pstData, bDrawType) < 0)
        {
            LOGERR("Uin<%lu> darw free:NOT_SATISFY_COND", pstData->m_ullUin);
            return ERR_NOT_SATISFY_COND;
        }
    }
    else if(!_CheckConsume(pstData, iCount, bConsumeType, iConsumeNum, dwTicketId, iTicketNum))
	{
		return ERR_NOT_ENOUGH_DIAMOND;
	}


    // 特殊规则是否生效
    uint32_t dwPondId = 0;
    bool bRuleEffect = _CheckRule(pstData, bDrawType, dwPondId);

    if (!bRuleEffect)
    {
        dwPondId = _DrawTypeConvPondId((uint32_t)bDrawType);     //采用默认奖池
        if (dwPondId < 1)
        {
            LOGERR("Uin<%lu> dwDrawType-<%hhu> is not found", pstData->m_ullUin, bDrawType);
            return ERR_SYS;
        }
    }

    uint16_t wMethod = bDrawType + METHOD_LOTTERY_DRAW_TYPE_OFFSET;


    if (m_bCheckFree)
    {
        m_bCheckFree = false;
    }
	else
	{
		Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, dwTicketId, -iTicketNum, rstScPkgBodyRsp.m_stSyncItemInfo, wMethod);
		Item::Instance().ConsumeItem(pstData, bConsumeType, 0, -iConsumeNum, rstScPkgBodyRsp.m_stSyncItemInfo, wMethod);
		if(LOTTERY_DRAW_TYPE_DIAMOND == bDrawType)
		{
			pstData->GetMiscInfo().m_llCostDiaLastTime = lCurTime;
		}
	}

	if (LOTTERY_DRAW_TYPE_DIAMOND_CNT == bDrawType)
	{
		_RandDrawPond(pstData, m_dwDiaDrawPondIdTen, bRuleEffect);
		pstData->GetMiscInfo().m_dwDrawByDCntCount++;
	}

	if (LOTTERY_DRAW_TYPE_GOLD_CNT == bDrawType)
	{
		_RandDrawPond(pstData, m_dwGoldDrawPondIdTen, bRuleEffect);
	}

    // 抽取奖品
    for (int i=0; i<iCount; i++)
    {
        //  为了应付版署增加的
        //      相当于购买强化石
		if (LOTTERY_DRAW_TYPE_DIAMOND_CNT == bDrawType || LOTTERY_DRAW_TYPE_DIAMOND == bDrawType)
		{
			_DrawLotteryByPond(pstData, (uint32_t)1003,  rstScPkgBodyRsp.m_stSyncItemInfo, wMethod);
		}
		else
		{
			_DrawLotteryByPond(pstData, (uint32_t)1002,  rstScPkgBodyRsp.m_stSyncItemInfo, wMethod);
		}

		//正式招募逻辑
		if(LOTTERY_DRAW_TYPE_DIAMOND_CNT==bDrawType)
		{
			// 钻石十连抽，每次奖池都不一样,如过是前n次十连，第十次抽取特殊处理
			dwPondId = m_dwDrawPondIdTen[i];
			_DrawLotteryByPond(pstData, dwPondId, rstScPkgBodyRsp.m_stSyncItemInfo, wMethod);
		}
		else if(LOTTERY_DRAW_TYPE_DIAMOND==bDrawType)
		{
			if (!bRuleEffect)
			{
				dwPondId = m_dwDiaDrawPondIdTen[pstData->GetMiscInfo().m_dwDrawByDCount1%MAX_LOTTERY_CNT_NUM];
			}
			pstData->GetMiscInfo().m_dwDrawByDCount1++;
			_DrawLotteryByPond(pstData, dwPondId, rstScPkgBodyRsp.m_stSyncItemInfo, wMethod);
		}
		else if(LOTTERY_DRAW_TYPE_GOLD_CNT==bDrawType)
		{
			dwPondId = m_dwDrawPondIdTen[i];
			_DrawLotteryByPond(pstData, dwPondId,  rstScPkgBodyRsp.m_stSyncItemInfo, wMethod);
		}
		else
		{
			if (!bRuleEffect)
			{
				dwPondId = m_dwGoldDrawPondIdTen[pstData->GetMiscInfo().m_dwDrawByGCount%MAX_LOTTERY_CNT_NUM];
			}
			pstData->GetMiscInfo().m_dwDrawByGCount++;
			_DrawLotteryByPond(pstData, dwPondId,  rstScPkgBodyRsp.m_stSyncItemInfo, wMethod);
		}

        LOGRUN("Uin<%lu> lottery draw OK! DrawType<%hhu>, PondId<%u>, DiamondCnt<%u>", pstData->m_ullUin, bDrawType, dwPondId, pstData->GetMiscInfo().m_dwDrawByDCount1);

    }

    Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_LOTTERY, iCount, 3);

    if (bDrawType == LOTTERY_DRAW_TYPE_DIAMOND_CNT ||bDrawType == LOTTERY_DRAW_TYPE_DIAMOND)
    {
        Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_LOTTERY, iCount, 2);
    }

    ZoneLog::Instance().WriteLotteryLog(pstData, bDrawType, rstScPkgBodyRsp.m_stSyncItemInfo, rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount - 1);
    return ERR_NONE;
}

int Lottery::HandleAwakeEquipLottery(PlayerData * pstData, uint8_t bDrawType, uint8_t bIsFree, SC_PKG_LOTTERY_DRAW_RSP & rstScPkgBodyRsp)
{
	//参数检查
	int iCount = 0;
	uint8_t bConsumeType = 0;
	int32_t iConsumeNum = 0;
	uint32_t dwTicketId = 0;
	int32_t iTicketNum = 0;
	if (GetParaFromDrawType(bDrawType, &iCount, &bConsumeType, &iConsumeNum, &dwTicketId) != 0)
	{
		LOGERR("Uin<%lu> bDrawType<%hhu> error", pstData->m_ullUin, bDrawType);
		return ERR_LOTTERY_DRAW_TYPE;
	}

    time_t lCurTime = CGameTime::Instance().GetCurrSecond();
    //觉醒商店暂时没有折扣
    //bool bIsDiscountUsed = CGameTime::Instance().IsInSameDay(lCurTime, pstData->GetMiscInfo().m_llFreeByAwakeEquipLastTime);

    //int32_t dwConsumeDiaDiscount = (!bIsDiscountUsed && LOTTERY_DRAW_TYPE_DIAMOND == bDrawType) ? m_dwDiscount*pstResRecord->m_dwConsumeDiamond / 100 : pstResRecord->m_dwConsumeDiamond;

    if (bIsFree == 1)
    {
        if (_CheckFreeCond(pstData, bDrawType) < 0)
        {
            LOGERR("Uin<%lu> darw free:NOT_SATISFY_COND", pstData->m_ullUin);
            return ERR_NOT_SATISFY_COND;
        }
    }
	else if (!_CheckConsume(pstData, iCount, bConsumeType, iConsumeNum, dwTicketId, iTicketNum))
	{
		return ERR_NOT_SATISFY_COND;
	}
    uint16_t wMethod = bDrawType + METHOD_LOTTERY_DRAW_TYPE_OFFSET;

    if (m_bCheckFree)
    {
        m_bCheckFree = false;
    }
    else 
    {
        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, dwTicketId, -iTicketNum, rstScPkgBodyRsp.m_stSyncItemInfo, wMethod);
        Item::Instance().ConsumeItem(pstData, bConsumeType, 0, -iConsumeNum, rstScPkgBodyRsp.m_stSyncItemInfo, wMethod);
        if (LOTTERY_DRAW_TYPE_EQUIP == bDrawType)
        {
            pstData->GetMiscInfo().m_llFreeByAwakeEquipLastTime = lCurTime;
        }
    }

    if (LOTTERY_DRAW_TYPE_EQUIP_CNT == bDrawType)
    {
        _RandDrawPond(pstData, m_dwAwakeEquipDrawPondIdTen, false);
        pstData->GetMiscInfo().m_dwCntDrawByAwakeEquip++;
    }

    uint32_t dwPondId = 0;

    for (int i = 0; i < iCount; ++i)
    {
        if (bDrawType == LOTTERY_DRAW_TYPE_EQUIP_CNT)
        {
            dwPondId = m_dwDrawPondIdTen[i];
            _DrawLotteryByPond(pstData, dwPondId, rstScPkgBodyRsp.m_stSyncItemInfo, wMethod);
        }
        else if (bDrawType == LOTTERY_DRAW_TYPE_EQUIP)
        {
            dwPondId = m_dwAwakeEquipDrawPondIdTen[pstData->GetMiscInfo().m_dwDrawByAwakeEquip++ % MAX_LOTTERY_CNT_NUM];
            _DrawLotteryByPond(pstData, dwPondId, rstScPkgBodyRsp.m_stSyncItemInfo, wMethod);
        }

        //LOGRUN("Uin<%lu> lottery draw OK! DrawType<%hhu>, PondId<%u>, DiamondCnt<%u>", pstData->m_ullUin, bDrawType, dwPondId, pstData->GetMiscInfo().m_dwDrawByDCount1);
    }
    switch (bDrawType)
    {
    case LOTTERY_DRAW_TYPE_EQUIP_CNT:
        _DrawLotteryByPond(pstData, m_dwAwakeEquipDrawPondId, rstScPkgBodyRsp.m_stSyncItemInfo, wMethod);
        break;
    case LOTTERY_DRAW_TYPE_EQUIP:
        _DrawLotteryByPond(pstData, m_dwAwakeEquipDrawPondIdOne, rstScPkgBodyRsp.m_stSyncItemInfo, wMethod);
        break;
    default:
        break;
    }
	Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_LOTTERY, iCount, 5);
    return ERR_NONE;
}


int Lottery::HandleLotteryDrawByAct(PlayerData* pstData, uint8_t bDrawType, SC_PKG_LOTTERY_DRAW_RSP& rstScPkgBodyRsp)
{
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    if ((pstData->GetVIPLv() < m_dwActDarwVipLimit && pstData->GetLv() < m_dwActDarwLvLimit))
    {
        LOGERR("Uin<%lu> can't join the act draw.", pstData->m_ullUin);
        return ERR_NOT_SATISFY_COND;
    }
    //检查活动是否开启
    int iRet = ERR_NONE;
	uint64_t ullTemp = 0;
	//开服魂匣
    if (TimeCycle::Instance().CheckTime(m_dwActDarwTimeId) == ERR_NONE)
    {
        if (bDrawType == LOTTERY_DRAW_TYPE_ACT) //单抽
        {
            iRet = HandleLotteryDrawByActBase(pstData, bDrawType, m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_ACT - 1].m_dwItemNum, 1,
                m_stActOneDrawPondFloatMap, m_dwActDrawPondIdTen, rstScPkgBodyRsp.m_stSyncItemInfo);
        }
        else  //十连
        {
            iRet = HandleLotteryDrawByActBase(pstData, bDrawType, m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_ACT_CNT - 1].m_dwItemNum, MAX_LOTTERY_CNT_NUM,
                m_stActCntDrawPondFloatMap, m_dwActDrawPondIdTen, rstScPkgBodyRsp.m_stSyncItemInfo);
        }
    }
	//首月魂匣
	else if (TaskAct::Instance().GetActState(pstData, m_dwActFMonthDarwActId, ullTemp, ullTemp))
	{
		if (bDrawType == LOTTERY_DRAW_TYPE_ACT) //单抽
		{
			iRet = HandleLotteryDrawByActBase(pstData, bDrawType, m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_ACT - 1].m_dwItemNum, 1,
				m_stActOneFMonthDrawPondFloatMap, m_dwActDrawPondIdTen, rstScPkgBodyRsp.m_stSyncItemInfo);
		}
		else  //十连
		{
			iRet = HandleLotteryDrawByActBase(pstData, bDrawType, m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_ACT_CNT - 1].m_dwItemNum, MAX_LOTTERY_CNT_NUM,
				m_stActCntFMonthDrawPondFloatMap, m_dwActDrawPondIdTen, rstScPkgBodyRsp.m_stSyncItemInfo);
		}
	}
	//通用魂匣
    else if ( TaskAct::Instance().IsActTypeOpen(pstData, ACT_TYPE_LOTTERY_ACT_DRAW))
    {
        if (bDrawType == LOTTERY_DRAW_TYPE_ACT)
        {
            iRet = HandleLotteryDrawByActBase(pstData, bDrawType, m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_ACT - 1].m_dwItemNum, 1,
                m_stActOneCommonDrawPondFloatMap, m_dwActCommonDrawPondIdTen, rstScPkgBodyRsp.m_stSyncItemInfo);
        }
        else
        {
            iRet = HandleLotteryDrawByActBase(pstData, bDrawType, m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_ACT_CNT - 1].m_dwItemNum, MAX_LOTTERY_CNT_NUM,
                m_stActCntCommonDrawPondFloatMap, m_dwActCommonDrawPondIdTen, rstScPkgBodyRsp.m_stSyncItemInfo);
        }
    }
    else
    {
        iRet = ERR_ACT_NOT_OPEN;
        LOGERR("Uin<%lu> Lotter act draw error:ERR_ACT_NOT_OPEN", pstData->m_ullUin);
    }
    return iRet;
}


int Lottery::HandleLotteryDrawByActBase(PlayerData* pstData, uint8_t bDrawType, uint32_t dwDiamond, uint32_t dwDrawCnt, 
    std::map<uint32_t, uint32_t>& rFloatMap, uint32_t(&dwActDrawPondIdTen)[MAX_LOTTERY_CNT_NUM], OUT DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    bool bIsSpecialCard = false;
    uint16_t wMethod = bDrawType + METHOD_LOTTERY_DRAW_TYPE_OFFSET;
    uint32_t& rdwPlayerCurDrawCnt = pstData->GetMiscInfo().m_dwDrawActCount;
    if (!Item::Instance().IsEnough(pstData, ITEM_TYPE_DIAMOND, 0, dwDiamond))
    {
        LOGERR("Uin<%lu> have no enought diamond<%u>", pstData->m_ullUin, dwDiamond);
        return ERR_NOT_ENOUGH_DIAMOND;
    }
    //扣除消耗
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -dwDiamond, rstSyncItemInfo, wMethod);
    m_stPondIdMapIter = rFloatMap.lower_bound(rdwPlayerCurDrawCnt + dwDrawCnt);
    if (m_stPondIdMapIter == rFloatMap.end())
    {
        LOGERR("Uin<%lu> find the count<%u> of PondId error", pstData->m_ullUin, rdwPlayerCurDrawCnt + MAX_LOTTERY_CNT_NUM);
        return ERR_SYS;
    }
    if (dwDrawCnt == 1)
    {
        m_dwDrawPondIdTen[0] = m_stPondIdMapIter->second;
    }
    else
    {
        //第10个是需要浮动 替换的，
        dwActDrawPondIdTen[MAX_LOTTERY_CNT_NUM - 1] = m_stPondIdMapIter->second;
        _RandDrawPond(pstData, dwActDrawPondIdTen, false);
    }

    // 抽取奖品
    for (int i = 0; i < (int)dwDrawCnt; i++)
    {

        rdwPlayerCurDrawCnt++;
        _DrawLotteryByPond(pstData, (uint32_t)1003, rstSyncItemInfo, wMethod);
        _DrawLotteryByPond(pstData, m_dwDrawPondIdTen[i], rstSyncItemInfo, wMethod);
        DT_ITEM& rstGetItme = rstSyncItemInfo.m_astSyncItemList[rstSyncItemInfo.m_bSyncItemCount - 1];
        if (rstGetItme.m_bItemType == ITEM_TYPE_GCARD)
        {
			RESGENERAL* pstGCard = CGameDataMgr::Instance().GetResGeneralMgr().Find(rstGetItme.m_stItemData.m_stGCard.m_dwId);
			if (pstGCard && pstGCard->m_dwTalent >= 14)
			{
				bIsSpecialCard = true;
			}
        }
        LOGRUN("Uin<%lu> lottery draw OK! DrawType<%hhu>, PondId<%u>, Cnt<%u>", pstData->m_ullUin, bDrawType, m_dwDrawPondIdTen[i], rdwPlayerCurDrawCnt);
    }
    
    if (bIsSpecialCard)
    {
        rdwPlayerCurDrawCnt = 0;
    }

    Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_LOTTERY, dwDrawCnt, 4);

    ZoneLog::Instance().WriteLotteryLog(pstData, bDrawType, rstSyncItemInfo, rstSyncItemInfo.m_bSyncItemCount - 1);
    return ERR_NONE;
}

void Lottery::_ChgBonusSequence(DT_SYNC_ITEM_INFO& rstRewardItemInfo)
{
	// 保底奖品切换位置
    uint32_t dwChgIndex = CFakeRandom::Instance().Random(0, MAX_LOTTERY_CNT_NUM-2);

    DT_ITEM stBonus = rstRewardItemInfo.m_astSyncItemList[dwChgIndex];
    rstRewardItemInfo.m_astSyncItemList[dwChgIndex] = rstRewardItemInfo.m_astSyncItemList[MAX_LOTTERY_CNT_NUM - 1];
    rstRewardItemInfo.m_astSyncItemList[MAX_LOTTERY_CNT_NUM - 1] = stBonus;
    return;
}

void Lottery::UpdatePlayerData(PlayerData* pstData)
{
    DT_ROLE_MISC_INFO& rstInfo = pstData->GetMiscInfo();
    //DT_ROLE_BASE_INFO& rstBaseInfo = pstData->GetRoleBaseInfo();

    //int64_t llCurTime = (int64_t)CGameTime::Instance().GetCurrSecond();
    //uint32_t dwIntervalSec = m_dwDiamondFreeInterval1Sec;

    if ((rstInfo.m_llFreeByGLastTime!=0) && CGameTime::Instance().IsNeedUpdateByHour(rstInfo.m_llFreeByGLastTime*1000, m_iDailyResetTime))
    {
        rstInfo.m_chFreeByGCount = 0;
        rstInfo.m_llFreeByGLastTime = 0;
        LOGRUN("Uin<%lu> update gold free draw", pstData->m_ullUin);
    }

//     if ((llCurTime - rstBaseInfo.m_llFirstLoginTime) > (SECONDS_OF_DAY *3))
//     {
//         dwIntervalSec = m_dwDiamondFreeInterval2Sec;
//     }

    if ((rstInfo.m_llFreeByDLastTime != 0) && CGameTime::Instance().IsNeedUpdateByHour(rstInfo.m_llFreeByDLastTime*1000, m_iDailyResetTime))
    {
        rstInfo.m_chFreeByDCount = 0;
        rstInfo.m_llFreeByDLastTime = 0;
        LOGRUN("Uin<%lu> update diamond free draw", pstData->m_ullUin);
    }

    if ((rstInfo.m_llFreeByAwakeEquipLastTime != 0) && CGameTime::Instance().IsNeedUpdateByHour(rstInfo.m_llFreeByAwakeEquipLastTime * 1000, m_iDailyResetTime))
    {
        rstInfo.m_dwFreeDrawByAwakeEquip = 0;
        rstInfo.m_llFreeByAwakeEquipLastTime = 0;
        LOGRUN("Uin<%lu> update awake equip free draw", pstData->m_ullUin);
    }

    return;
}

int Lottery::DrawLotteryByPond(PlayerData* pstData, uint32_t dwPondId, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach, bool bIsMerge)
{
    return this->_DrawLotteryByPond(pstData, dwPondId, rstSyncItemInfo, dwApproach, bIsMerge);
}

int Lottery::TestUnit(uint32_t dwPondId, int iTestTimes)
{

    DT_SYNC_ITEM_INFO stSyncItemInfo;
    for (int i=0; i<iTestTimes; i++)
    {
        _DrawLotteryByPond(NULL, dwPondId, stSyncItemInfo, true);
    }

    return 0;
}

void Lottery::DrawActReset(PlayerData* pstData)
{
    pstData->GetMiscInfo().m_dwDrawActCount = 0;
}


int Lottery::GetParaFromDrawType(uint8_t bDrawType, OUT int* piDrawCount, OUT uint8_t* pbConsumeType, OUT int32_t* pdiConsumeNum, OUT uint32_t* pdwTicketId, 
    OUT uint32_t* pdwConsumeId/*= 0*/)
{
	switch (bDrawType)
	{
	case LOTTERY_DRAW_TYPE_GOLD:
	{
		*piDrawCount = 1;
		*pbConsumeType = m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_GOLD - 1].m_bItemType;
		*pdiConsumeNum = m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_GOLD - 1].m_dwItemNum;
		if (pdwTicketId)
		{
			*pdwTicketId = GOLD_TICKET_ID;
		}
		return 0;
	}
	case LOTTERY_DRAW_TYPE_GOLD_CNT:
	{
		*piDrawCount = MAX_LOTTERY_CNT_NUM;
		*pbConsumeType = m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_GOLD_CNT - 1].m_bItemType;
		*pdiConsumeNum = m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_GOLD_CNT - 1].m_dwItemNum;
		if (pdwTicketId)
		{
			*pdwTicketId = GOLD_TICKET_ID;
		}
		return 0;
	}
	case LOTTERY_DRAW_TYPE_DIAMOND:
	{
		*piDrawCount = 1;
		*pbConsumeType = m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_DIAMOND - 1].m_bItemType;
		*pdiConsumeNum = m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_DIAMOND - 1].m_dwItemNum;
		if (pdwTicketId)
		{
			*pdwTicketId = DIAMOND_TICKET_ID;
		}
		return 0;
	}
	case LOTTERY_DRAW_TYPE_DIAMOND_CNT:
	{
		*piDrawCount = MAX_LOTTERY_CNT_NUM;
		*pbConsumeType = m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_DIAMOND_CNT - 1].m_bItemType;
		*pdiConsumeNum = m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_DIAMOND_CNT - 1].m_dwItemNum;
		if (pdwTicketId)
		{
			*pdwTicketId = DIAMOND_TICKET_ID;
		}
		return 0;
	}
	case LOTTERY_DRAW_TYPE_EQUIP:
	{
		*piDrawCount = 1;
		*pbConsumeType = m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_EQUIP - 1].m_bItemType;
		*pdiConsumeNum = m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_EQUIP - 1].m_dwItemNum;
		if (pdwTicketId)
		{
			*pdwTicketId = AWAKE_EQUIP_TICKET_ID;
		}
		return 0;
	}
	case LOTTERY_DRAW_TYPE_EQUIP_CNT:
	{
		*piDrawCount = MAX_LOTTERY_CNT_NUM;
		*pbConsumeType = m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_EQUIP_CNT - 1].m_bItemType;
		*pdiConsumeNum = m_stPrice.m_astLotteryPrice[LOTTERY_DRAW_TYPE_EQUIP_CNT - 1].m_dwItemNum;
		if (pdwTicketId)
		{
			*pdwTicketId = AWAKE_EQUIP_TICKET_ID;
		}
		return 0;
	}
	default:
		return -1;
	}
}

//获取默认奖池
int Lottery::_DrawTypeConvPondId(uint32_t dwDrawType)
{
    std::map<uint32_t, uint32_t>::iterator iter = m_stDrawType2PondIdMap.find(dwDrawType);
    if (iter != m_stDrawType2PondIdMap.end())
    {
        return (int)iter->second;
    }
    else
    {
        return 0;
    }
}

int Lottery::_DrawLotteryByPond(PlayerData* pstData, uint32_t dwPondId, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach, bool bIsMerge)
{
    {
        // random pick lottery
        int iRet = ERR_NONE;
        do
        {
            map<uint32_t, vector<RandomNode> >::iterator iterLotteryList = m_oPondId2RandomNodeVectMap.find(dwPondId);
            if (iterLotteryList == m_oPondId2RandomNodeVectMap.end())
            {
                LOGERR("Uin<%lu> dwPondId-%u is not found", pstData->m_ullUin, dwPondId);
                // default value
                iterLotteryList = m_oPondId2RandomNodeVectMap.find(1);
            }
            vector<RandomNode>& pRandomNodeVector = iterLotteryList->second;
            if (pRandomNodeVector.empty())
            {
                LOGERR("Uin<%lu> dwPondId-%u bonus is empty", pstData->m_ullUin, dwPondId);
                iRet = -1001;
                break;
            }

            uint32_t dwRandom = CFakeRandom::Instance().Random(1, CONST_REAL_MAX_PROBABILITY);
            m_stRandomNode.SetRange(dwRandom);
            int iEqual = 0;
            int iIndex = MyBSearch(&m_stRandomNode, &(pRandomNodeVector[0]), pRandomNodeVector.size(), sizeof(RandomNode), &iEqual, RandomNodeFindCmp);
            if (!iEqual)
            {
                LOGERR("Uin<%lu> dwPondId-%u, dwRandom-%u is not found", pstData->m_ullUin, dwPondId, dwRandom);
                iRet = -1002;
                break;
            }
            uint32_t dwBonusResId = pRandomNodeVector[iIndex].m_dwValue;
            RESLOTTERYBONUSMAP* pstBonus = CGameDataMgr::Instance().GetResLotteryBonusMapMgr().Find(dwBonusResId);
            if (!pstBonus)
            {
                LOGERR("Uin<%lu> draw !!PondId<%u>, dwBonusResId<%u>", pstData->m_ullUin, dwPondId, dwBonusResId);
                iRet = -1003;
                break;
            }
            // 添加奖励
            iRet = Item::Instance().RewardItem(pstData, pstBonus->m_bBonusType, pstBonus->m_dwBonusId,
                pstBonus->m_dwBonusNum, rstSyncItemInfo, dwApproach, Item::CONST_SHOW_PROPERTY_NORMAL, bIsMerge);

            //当获得14资质武将时发送系统/世界消息
            int iItemIndex = rstSyncItemInfo.m_bSyncItemCount - 1;
            if (rstSyncItemInfo.m_astSyncItemList[iItemIndex].m_bItemType == ITEM_TYPE_GCARD)
            {
                ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
                RESGENERAL* pResGeneral = rstResGeneralMgr.Find(rstSyncItemInfo.m_astSyncItemList[iItemIndex].m_dwItemId);
                if (!pResGeneral)
                {
                    LOGERR_r("pResGeneral not found id=%u", rstSyncItemInfo.m_astSyncItemList[iItemIndex].m_dwItemId);
                    return ERR_SYS;
                }
                if (pResGeneral->m_dwTalent == 14)
                {
                    _SendLotteryMessage(pstData, dwApproach, pResGeneral->m_dwId);
                }
            }
            LOGRUN("Uin<%lu> draw OK PondId<%u>, dwBonusResId<%u> BonusType<%hhu>, BonusId<%u>, BonusNum<%u> ",
                pstData->m_ullUin, dwPondId, dwBonusResId, pstBonus->m_bBonusType, pstBonus->m_dwBonusId, pstBonus->m_dwBonusNum);
        } while (0);

        //出错,默认奖励
        if (iRet != ERR_NONE)
        {
            LOGERR("Uin<%lu> draw error PondId<%u> Ret<%d>", pstData->m_ullUin, dwPondId, iRet);
            Item::Instance().RewardItem(pstData, m_stDefaultItem.m_bItemType, m_stDefaultItem.m_dwItemId,
                m_stDefaultItem.m_iValueChg, rstSyncItemInfo, dwApproach, Item::CONST_SHOW_PROPERTY_NORMAL, bIsMerge);
            return ERR_SYS;
        }
        return ERR_NONE;
    }
}

int Lottery::_DrawLotteryTest(uint32_t dwLotteryId)
{
    // random pick bonus

    return 0;
}



void Lottery::_RandDiaDrawPond()
{
    //随机数组
   uint32_t dwRand = CFakeRandom::Instance().Random(m_dwFactMax) + 1;
   uint32_t dwSelect = 0;
   uint32_t tmp = 0;
   for (int i = 0; i < MAX_LOTTERY_CNT_NUM; i++)
   {
       dwSelect = dwRand % (MAX_LOTTERY_CNT_NUM - i) ;
       tmp = m_dwDiaDrawPondIdTen[MAX_LOTTERY_CNT_NUM - 1 -i];
       m_dwDiaDrawPondIdTen[MAX_LOTTERY_CNT_NUM - 1 -i] = m_dwDiaDrawPondIdTen[dwSelect];
       m_dwDiaDrawPondIdTen[dwSelect] = tmp;
   }
}

int Lottery::InitPlayerData(PlayerData* pstData)
{
    UpdatePlayerData(pstData);
    return 0;
}

bool Lottery::_CheckConsume(PlayerData* pstData, int iCount, uint8_t bConsumeType, int32_t& riConsumeNum, uint32_t dwTicketId, int32_t& riTicketNum, uint32_t dwConsumeId)
{
	uint32_t dwPropNums = Props::Instance().GetNum(pstData, dwTicketId);
	riTicketNum = dwPropNums >= (uint32_t)iCount ? iCount : dwPropNums;
	riConsumeNum =  (iCount - riTicketNum) * riConsumeNum / iCount;
    if (dwConsumeId != 0)
    {
        uint32_t dwOwnNum = Props::Instance().GetNum(pstData, dwConsumeId);
        return dwOwnNum >= static_cast<uint32_t>(riConsumeNum);
    }
	else if (riConsumeNum == 0 || Consume::Instance().IsEnough(pstData, bConsumeType, riConsumeNum))
	{
		return true;
	}
	LOGERR("Uin<%lu> lottery draw ten failed: iCount<%d>, ConsumeType<%hhu>, ConsumeNum<%d> TicketId<%u> TicketNum<%d>", pstData->m_ullUin, iCount, bConsumeType, riConsumeNum, dwTicketId, riTicketNum);
	return false;
	
}

void Lottery::_RandDrawPond(PlayerData* pstData, uint32_t (&rstDrawPondIdTen)[MAX_LOTTERY_CNT_NUM], bool bRuleEffect)
{

    memcpy(m_dwDrawPondIdTen, rstDrawPondIdTen, sizeof(m_dwDrawPondIdTen));
	if(bRuleEffect)
	{
		m_dwDrawPondIdTen[MAX_LOTTERY_CNT_NUM-1] = m_stFirstDrawTensToPondIdMap[pstData->GetMiscInfo().m_dwDrawByDCntCount+1];
	}

	//随机数组
	uint32_t dwRand = CFakeRandom::Instance().Random(m_dwFactMax) + 1;
	uint32_t dwSelect = 0;
	uint32_t tmp = 0;
	for (int i = 0; i < MAX_LOTTERY_CNT_NUM; i++)
	{
		dwSelect = dwRand % (MAX_LOTTERY_CNT_NUM - i) ;
		tmp = m_dwDrawPondIdTen[MAX_LOTTERY_CNT_NUM - 1 -i];
		m_dwDrawPondIdTen[MAX_LOTTERY_CNT_NUM - 1 -i] = m_dwDrawPondIdTen[dwSelect];
		m_dwDrawPondIdTen[dwSelect] = tmp;
	}
}

void Lottery::_SendLotteryMessage(PlayerData* pstData, uint32_t dwApproach, uint32_t dwGenenralId)
{
    if (dwApproach == METHOD_LOTTERY_DRAW_TYPE_DIAMOND
        || dwApproach == METHOD_LOTTERY_DRAW_TYPE_DIAMOND_CNT)
    {
        Message::Instance().AutoSendSysMessage(2102, "Name=%s|GCardId=%u", pstData->GetRoleName(), dwGenenralId);
        Message::Instance().AutoSendWorldMessage(pstData, 2100, "GCardId=%u", dwGenenralId);
    }
    else if (dwApproach == METHOD_LOTTERY_DRAW_TYPE_ACT
        || dwApproach == METHOD_LOTTERY_DRAW_TYPE_ACT_CNT)
    {
        Message::Instance().AutoSendSysMessage(2103, "Name=%s|GCardId=%u", pstData->GetRoleName(), dwGenenralId);
        Message::Instance().AutoSendWorldMessage(pstData, 2101, "GCardId=%u", dwGenenralId);
    }
}

