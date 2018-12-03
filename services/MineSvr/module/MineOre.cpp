
#include "define.h"
#include "MineOre.h"
#include "GameTime.h"
#include "LogMacros.h"
#include "PKGMETA_metalib.h"
#include "strutil.h"
#include "MineLogicMgr.h"
#include "MineDataMgr.h"
using namespace PKGMETA;

void MineOre::Reset()
{
    TLIST_INIT(&m_stDirtyListNode);
    TLIST_INIT(&m_stTimeListNode);
	TLIST_INIT(&m_stEmptyListNode);
    bzero(&m_stOreInfo, sizeof(m_stOreInfo));
}


void MineOre::Clear4Reuse()
{
	uint64_t ullUid = m_stOreInfo.m_ullUid;
	uint32_t dwOreId = m_stOreInfo.m_dwOreId;
	uint8_t bOreType = m_stOreInfo.m_bOreType;
    LOGWARN("Ore<%lu> clear for reuse, owner<%lu>", m_stOreInfo.m_ullUid, m_stOreInfo.m_ullOwnerUin);
	bzero(&m_stOreInfo, sizeof(m_stOreInfo));
	m_stOreInfo.m_ullUid = ullUid;
	m_stOreInfo.m_dwOreId = dwOreId;
	m_stOreInfo.m_bOreType = bOreType;
	m_stOreInfo.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();

}

bool MineOre::InitFromDB(PKGMETA::DT_MINE_ORE_DATA& rstData)
{
    this->Reset();
    size_t ulUseSize = 0;
    uint32_t dwVersion = rstData.m_dwVersion;
    int iRet = m_stOreInfo.unpack((char*)rstData.m_stOreBlob.m_szData, sizeof(rstData.m_stOreBlob.m_szData), &ulUseSize, dwVersion);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("Id(%lu) unpack m_stOreBlob failed, Ret(%d)", rstData.m_ullOreUid, iRet);
        return false;
    }
	LOGWARN("OreUid<%lu> Type<%hhu>, Onwer<%lu>, CreateTime<%lu>, OccupyTime<%lu>", m_stOreInfo.m_ullUid, m_stOreInfo.m_bOreType, 
		m_stOreInfo.m_ullOwnerUin, m_stOreInfo.m_ullCreateTime, m_stOreInfo.m_ullOccupyTime);
    return true;
}


bool MineOre::PackToData(OUT PKGMETA::DT_MINE_ORE_DATA& rstData)
{
    size_t ulUseSize = 0;
    rstData.m_ullOreUid = m_stOreInfo.m_ullUid;
    rstData.m_dwVersion = MetaLib::getVersion();
    int iRet = m_stOreInfo.pack((char*)rstData.m_stOreBlob.m_szData, sizeof(rstData.m_stOreBlob.m_szData), &ulUseSize);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("OreUid(%lu) pack m_stOreBlob failed! Ret(%d) ", m_stOreInfo.m_ullUid, iRet);
        return false;
    }
    rstData.m_stOreBlob.m_iLen = (int)ulUseSize;


	DT_MINE_ORE_INFO stOreInfo = { 0 };
	 ulUseSize = 0;
	iRet = stOreInfo.unpack((char*)rstData.m_stOreBlob.m_szData, sizeof(rstData.m_stOreBlob.m_szData), &ulUseSize, rstData.m_dwVersion);
	if (iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR("Id(%lu) unpack m_stOreBlob failed, Ret(%d)", rstData.m_ullOreUid, iRet);
	}

    return true;
}

void MineOre::Init(uint64_t ullUid, uint8_t bOreType, uint32_t dwOreId)
{
    Reset();
    m_stOreInfo.m_ullUid = ullUid;
    m_stOreInfo.m_bOreType = bOreType;
    m_stOreInfo.m_dwOreId = dwOreId;
	m_stOreInfo.m_szOwnerName[0] = '\0';
    m_stOreInfo.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();
}

void MineOre::GetOreInfo(OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo)
{
    memcpy(&rstOreInfo, &m_stOreInfo, sizeof(rstOreInfo));
}

int MineOre::Occupy(char szName[], uint32_t dwAddr, uint64_t ullOwnerUin, uint32_t dwLeaderValue,
	uint8_t bTroopCount, DT_TROOP_INFO* pstTroopInfo, DT_ITEM_MSKILL& rstMSkill)
{
    if (ullOwnerUin == 0 || bTroopCount == 0)
    {
        LOGERR("Uin<%lu> bTroopCount<%hhu> Occupy the OreId<%u> OreUid<%lu> error: the ore's owner<%lu> ",
            ullOwnerUin, bTroopCount, m_stOreInfo.m_dwOreId, m_stOreInfo.m_ullUid, m_stOreInfo.m_ullOwnerUin);
        return ERR_MINE_OCCUPY_FAIL;
    }
	Clear4Reuse();
	uint64_t ullNow = CGameTime::Instance().GetCurrSecond();
    m_stOreInfo.m_ullOwnerUin = ullOwnerUin;
    m_stOreInfo.m_bTroopCount = MIN(bTroopCount, MAX_MINE_TROOP_NUM);
	SetOccupyTime(ullNow);
	//SetGuardTime(ullNow + MineLogicMgr::Instance().m_dwOccupyOreGuardTime);	//设置占领保护时间
    StrCpy(m_stOreInfo.m_szOwnerName, szName, sizeof(m_stOreInfo.m_szOwnerName));
    m_stOreInfo.m_dwOwnerAddr = dwAddr;
	m_stOreInfo.m_dwOwnerLi = 0;
	m_stOreInfo.m_dwLeaderValue = dwLeaderValue;
    memcpy(m_stOreInfo.m_astTroopInfo, pstTroopInfo, sizeof(DT_TROOP_INFO) * m_stOreInfo.m_bTroopCount);
    for (int i = 0; i < (int)m_stOreInfo.m_bTroopCount; i++)
    {
        m_stOreInfo.m_dwOwnerLi += m_stOreInfo.m_astTroopInfo[i].m_stGeneralInfo.m_dwLi;
    }
    m_stOreInfo.m_stMasterSkill = rstMSkill;
	
    LOGWARN("Uin<%lu> occppuy  OreUid=%lu  ", ullOwnerUin, m_stOreInfo.m_ullUid);

    return ERR_NONE;
     
}

int MineOre::Drop()
{

	uint64_t ullUid = m_stOreInfo.m_ullUid;
	uint32_t dwOreId = m_stOreInfo.m_dwOreId;
	uint8_t bOreType = m_stOreInfo.m_bOreType;

    bzero(&m_stOreInfo, sizeof(m_stOreInfo));

	m_stOreInfo.m_ullUid = ullUid;
	m_stOreInfo.m_dwOreId = dwOreId;
	m_stOreInfo.m_bOreType = bOreType;

    return ERR_NONE;
}

int MineOre::Modify(uint64_t ullOwnerUin, PKGMETA::DT_ITEM_MSKILL& rstMSkill)
{
    if (m_stOreInfo.m_ullOwnerUin != ullOwnerUin)
    {
        LOGERR("Uin<%lu> drop the ore<Uid=%lu Id=%u Owner=%lu> error! ", ullOwnerUin, m_stOreInfo.m_ullUid, m_stOreInfo.m_dwOreId, m_stOreInfo.m_ullOwnerUin);
        return ERR_MINE_DROP_FAIL;
    }
    memcpy(&m_stOreInfo.m_stMasterSkill, &rstMSkill, sizeof(DT_ITEM_MSKILL));
    return ERR_NONE;
}


int MineOre::GiveUpGrab()
{
	SetSelectOccupyOutTime(0);
	SetChallenger(0);
	return ERR_NONE;
}

//	获取生产周期数:
//		- 每日结算
//			1.若当前时间比今天的结算时间大,那么需要结算从占领到现在的产出
//			2.若当前时间比今天的结算时间小,那么只需结算今天之前的产出	(这个是容错处理, 服务器关闭后,在下一天 结算起始时间之后开启)
//			3.每日结算后会把占领时间修改到下一次结算的开始时间
//		- 其他(放弃,抢夺)结算
//			计算今天的产出,也只会有今天的产出

int MineOre::GetProduceCycleNum(RESMINE* pResMine, uint8_t bType, OUT uint64_t* pstSettleTime)
{
	uint64_t ullNow = CGameTime::Instance().GetCurrSecond();
	uint64_t ullOccupyZeroSec = CGameTime::Instance().GetBeginOfDayInSomeDay(m_stOreInfo.m_ullOccupyTime);

	int iProduceTime = 0;
	uint64_t ullLogSettleTime = 0;
	if (MINE_AWARD_LOG_TYPE_DAILY == bType)
	{
		if (ullNow >= MineLogicMgr::Instance().m_ullNowDailyEndTime)	//结算全部时间
		{
			int iSettleDayNum = (CGameTime::Instance().GetBeginOfDay() - ullOccupyZeroSec) / SECONDS_OF_DAY + 1;
			//如果出现异常(如跨赛季停机)最多结算一个赛季的奖励
			iSettleDayNum = MIN(iSettleDayNum, MineLogicMgr::Instance().m_bSeasonEndWeekDay - MineLogicMgr::Instance().m_bSeasonStartWeekDay + 1);
			iProduceTime = iSettleDayNum * MineLogicMgr::Instance().m_dwDailyMaxProduceTime - 
				(m_stOreInfo.m_ullOccupyTime - (ullOccupyZeroSec + MineLogicMgr::Instance().m_dwDailyStartSecOfDay));

			m_stOreInfo.m_ullOccupyTime = MineLogicMgr::Instance().m_ullNowDailyStartTime + SECONDS_OF_DAY;
			ullLogSettleTime = MineLogicMgr::Instance().m_ullDailySettleAwardTime;
		}
		else //结算今天以前的
		{
			int iSettleDayNum = (CGameTime::Instance().GetBeginOfDay() - ullOccupyZeroSec) / SECONDS_OF_DAY;
			if (iSettleDayNum <= 0)
			{
				return 0;
			}
			//如果出现异常(如跨赛季停机)最多结算一个赛季的奖励
			iSettleDayNum = MIN(iSettleDayNum, MineLogicMgr::Instance().m_bSeasonEndWeekDay - MineLogicMgr::Instance().m_bSeasonStartWeekDay + 1);

			iProduceTime = iSettleDayNum * MineLogicMgr::Instance().m_dwDailyMaxProduceTime - 
				(m_stOreInfo.m_ullOccupyTime - (ullOccupyZeroSec + MineLogicMgr::Instance().m_dwDailyStartSecOfDay));
			m_stOreInfo.m_ullOccupyTime = MineLogicMgr::Instance().m_ullNowDailyStartTime;

			//计算结算时间
			ullLogSettleTime = ullOccupyZeroSec + (iSettleDayNum - 1) * SECONDS_OF_DAY + MineLogicMgr::Instance().m_dwDailySettleAwardSecOfDay;
		}
	}
	else
	{

		if (!CGameTime::Instance().IsInSameDay(m_stOreInfo.m_ullOccupyTime, ullNow)
			|| m_stOreInfo.m_ullOccupyTime < MineLogicMgr::Instance().m_ullNowDailyStartTime
			|| m_stOreInfo.m_ullOccupyTime > MineLogicMgr::Instance().m_ullNowDailyEndTime)
		{
			LOGERR("Uin<%lu> get produce time error! OccupyTime<%lu>", GetOwner(), m_stOreInfo.m_ullOccupyTime);
			return 0;
		}
		else
		{
			iProduceTime = MIN(ullNow, MineLogicMgr::Instance().m_ullNowDailyEndTime) - m_stOreInfo.m_ullOccupyTime;
		}
		ullLogSettleTime = CGameTime::Instance().GetCurrSecond();
	}

	uint32_t dwProduceCount = iProduceTime / 60 / pResMine->m_dwProduceTime;
	if (pstSettleTime)
	{
		*pstSettleTime = ullLogSettleTime;
	}
	LOGWARN("Uin<%lu> OreProduceCycleNum<%u>  OreUid<%lu> OreId<%u>  OccupyTIme<%lu> ",
		m_stOreInfo.m_ullOwnerUin, dwProduceCount, m_stOreInfo.m_ullUid, m_stOreInfo.m_dwOreId , m_stOreInfo.m_ullOccupyTime);
	return dwProduceCount;
}


void MineOre::ClearBeLootedInfo()
{
	m_stOreInfo.m_bBeLootedCount = 0;
	m_stOreInfo.m_bLostCount = 0;
	bzero(m_stOreInfo.m_astLostList, sizeof(m_stOreInfo.m_astLostList));
}

uint64_t MineOre::GetRealFightGuardTime()
{
	return MAX(m_stOreInfo.m_ullSelectOccupyOutTime, MAX(m_stOreInfo.m_ullFightGuardTime,
		m_stOreInfo.m_ullOccupyTime + MineLogicMgr::Instance().m_dwOccupyOreGuardTime));
}


