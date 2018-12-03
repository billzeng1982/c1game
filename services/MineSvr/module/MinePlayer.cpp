
#include "define.h"
#include "MinePlayer.h"
#include "LogMacros.h"
#include "PKGMETA_metalib.h"
#include "GameTime.h"
#include "MineLogicMgr.h"
#include "../gamedata/GameDataMgr.h"
#include "MineDataMgr.h"
#include "strutil.h"

using namespace PKGMETA;

// static int CompareUid(const void* pA, const void* pB)
// {
//
// 	if ( *(uint64_t*)pA > *(uint64_t*)pB )
// 	{
// 		return 1;
// 	}
// 	else if (*(uint64_t*)pA < *(uint64_t*)pB )
// 	{
// 		return -1;
// 	}
// 	else
// 	{
// 		return 0;
// 	}
// }

static int CompareExploreOreUid(const void* pA, const void* pB)
{
	if (((DT_MINE_EXPLORE_ORE_INFO*)pA)->m_ullOreUid > ((DT_MINE_EXPLORE_ORE_INFO*)pB)->m_ullOreUid)
	{
		return 1;
	}
	else if (((DT_MINE_EXPLORE_ORE_INFO*)pA)->m_ullOreUid < ((DT_MINE_EXPLORE_ORE_INFO*)pB)->m_ullOreUid)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}
static int ItemCmp(const void *pstFirst, const void *pstSecond)
{
    DT_ITEM_SIMPLE* pstItemFirst = (DT_ITEM_SIMPLE*)pstFirst;
    DT_ITEM_SIMPLE* pstItemSecond = (DT_ITEM_SIMPLE*)pstSecond;


    if (pstItemFirst->m_bItemType < pstItemSecond->m_bItemType)
    {
        return -1;
    }
    else if (pstItemFirst->m_bItemType > pstItemSecond->m_bItemType)
    {
        return 1;
    }
    else
    {
        //type相等
        if (pstItemFirst->m_dwItemId < pstItemSecond->m_dwItemId)
        {
            return -1;
        }
        else if (pstItemFirst->m_dwItemId > pstItemSecond->m_dwItemId)
        {
            return 1;
        }
        else
        {
            return 0;
        }

    }

}
void MinePlayer::Reset()
{
    TLIST_INIT(&m_stDirtyListNode);
    TLIST_INIT(&m_stTimeListNode);
    bzero(&m_stPlayerInfo, sizeof(m_stPlayerInfo));
}

void MinePlayer::Init(uint64_t ullUin)
{
    Reset();
    m_stPlayerInfo.m_ullUin = ullUin;
}

bool MinePlayer::InitFromDB(PKGMETA::DT_MINE_PLAYER_DATA& rstData)
{
    this->Reset();
    size_t ulUseSize = 0;
    uint32_t dwVersion = rstData.m_dwVersion;
    int iRet = m_stPlayerInfo.unpack((char*)rstData.m_stPlayerBlob.m_szData, sizeof(rstData.m_stPlayerBlob.m_szData), &ulUseSize, dwVersion);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("Uin(%lu) unpack m_stPlayerBlob failed, Ret(%d)", rstData.m_ullUin, iRet);
        return false;
    }
    return true;
}

bool MinePlayer::PackToData(OUT PKGMETA::DT_MINE_PLAYER_DATA& rstData)
{
    size_t ulUseSize = 0;
    rstData.m_ullUin = m_stPlayerInfo.m_ullUin;
    rstData.m_dwVersion = MetaLib::getVersion();
    int iRet = m_stPlayerInfo.pack((char*)rstData.m_stPlayerBlob.m_szData, sizeof(rstData.m_stPlayerBlob.m_szData), &ulUseSize);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("PlayerUin(%lu) pack m_stPlayerBlob failed! Ret(%d) ", m_stPlayerInfo.m_ullUin, iRet);
        return false;
    }
    rstData.m_stPlayerBlob.m_iLen = (int)ulUseSize;
    return true;
}

void MinePlayer::SetPlayerInfo(char* szName, uint32_t dwAddr, uint16_t wLv, uint16_t wIconId, uint32_t dwLeaderValue)
{
    if (szName[0] != '\0' )
    {
        StrCpy(m_stPlayerInfo.m_szName, szName, sizeof(m_stPlayerInfo.m_szName));
    }
    m_stPlayerInfo.m_dwSvrAddr = dwAddr;
	m_stPlayerInfo.m_wLv = wLv;
	m_stPlayerInfo.m_wIconId = wIconId;
	m_stPlayerInfo.m_dwLeaderValue = dwLeaderValue;
}

int MinePlayer::CanOccupyOre(uint64_t ullOreUid)
{

    if (!CheckOreUidInExploreList(ullOreUid)       //不在探索列表里
        || CheckOreUidInOwnList(ullOreUid))        //在占领列表里
    {
        return ERR_MINE_INVALID_ORE;
    }
    if (m_stPlayerInfo.m_bOwnOreCount >= MAX_MINE_OWN_ORE_NUM)
    {
        return ERR_MINE_OWN_ORE_NUM_FULL;
    }
    return ERR_NONE;
}

bool MinePlayer::CheckOreUidInExploreList(uint64_t ullOreUid)
{
    int iExploreEqual = 0;
	DT_MINE_EXPLORE_ORE_INFO tmp = { 0 };
	tmp.m_ullOreUid = ullOreUid;
    MyBSearch(&tmp, m_stPlayerInfo.m_astExploreOreIdList, m_stPlayerInfo.m_bExploreOreCount, sizeof(DT_MINE_EXPLORE_ORE_INFO), &iExploreEqual, CompareExploreOreUid);
    return (bool)iExploreEqual;
}

bool MinePlayer::CheckOreUidInOwnList(uint64_t ullOreUid)
{
    //在拥有列表中不存在, 拥有列表得按先后顺序,不能用BinSearch
    for (int i = 0; i < (int)m_stPlayerInfo.m_bOwnOreCount; i++)
    {
        if (m_stPlayerInfo.m_OwnOreIdList[i] == ullOreUid)
        {
            return true;

        }
    }
    return false;
}


int MinePlayer::Investigate(uint64_t ullOreUid, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo)
{

	MineOre* pstOre = MineDataMgr::Instance().GetMineOre(ullOreUid);
	if (!pstOre)
	{
		LOGERR("Uin<%lu> can't find the ore<%lu> or player", m_stPlayerInfo.m_ullUin, ullOreUid);
		return ERR_SYS;
	}
	if (pstOre->GetType() != ORE_TYPE_INVESTIGATE)
	{
		LOGERR("Uin<%lu> investigate the ore error, ore<%lu> type not match!", m_stPlayerInfo.m_ullUin, ullOreUid);
		return ERR_MINE_TYPE_ERROR;
	}

	if(IsSetOreUsedState(ullOreUid) != ERR_NONE)
	{
		return ERR_SYS;
	}
	SetOreUesedState(ullOreUid);
	pstOre->GetOreInfo(rstOreInfo);
	return ERR_NONE;
}


int MinePlayer::SetOreUesedState(uint64_t ullOreUid)
{
	int iExploreEqual = 0;
	DT_MINE_EXPLORE_ORE_INFO tmp = { 0 };
	tmp.m_ullOreUid = ullOreUid;
	int iIndex = MyBSearch(&tmp, m_stPlayerInfo.m_astExploreOreIdList, m_stPlayerInfo.m_bExploreOreCount, sizeof(DT_MINE_EXPLORE_ORE_INFO), &iExploreEqual, CompareExploreOreUid);
	if (!iExploreEqual)
	{
		LOGERR("Uin<%lu> the OreUid<%lu> error", m_stPlayerInfo.m_ullUin, ullOreUid);
		return  ERR_MINE_INVALID_ORE;
	}
	m_stPlayerInfo.m_astExploreOreIdList[iIndex].m_bState = 1; //已调查
	return ERR_NONE;
}

int MinePlayer::IsSetOreUsedState(uint64_t ullOreUid)
{
	int iExploreEqual = 0;
	DT_MINE_EXPLORE_ORE_INFO tmp = { 0 };
	tmp.m_ullOreUid = ullOreUid;
	int iIndex = MyBSearch(&tmp, m_stPlayerInfo.m_astExploreOreIdList, m_stPlayerInfo.m_bExploreOreCount, sizeof(DT_MINE_EXPLORE_ORE_INFO), &iExploreEqual, CompareExploreOreUid);
	if (!iExploreEqual)
	{
		LOGERR("Uin<%lu> the OreUid<%lu> error", m_stPlayerInfo.m_ullUin, ullOreUid);
		return  ERR_MINE_INVALID_ORE;
	}
	if(m_stPlayerInfo.m_astExploreOreIdList[iIndex].m_bState == 1)
	{
		return ERR_SYS;
	}

	return ERR_NONE;
}



int MinePlayer::GetAllInfo(SS_PKG_MINE_GET_INFO_RSP& rstRsp)
{
    vector<void*>   TmpUidVector;
    vector<void*>   TmpInfoVector;
    DataKey TmpDataKeyArry[MAX_MINE_EXPLORE_ORE_NUM + MAX_MINE_OWN_ORE_NUM];

    if (m_stPlayerInfo.m_bExploreOreCount == 0 && m_stPlayerInfo.m_bOwnOreCount == 0)
    {
        return ERR_NONE;
    }

    for (int i = 0; i < MAX_MINE_EXPLORE_ORE_NUM + MAX_MINE_OWN_ORE_NUM; i++)
    {
        if (i < m_stPlayerInfo.m_bExploreOreCount)
        {
            TmpDataKeyArry[i].m_bType = DATA_TYPE_ORE;
            TmpDataKeyArry[i].m_ullKey = m_stPlayerInfo.m_astExploreOreIdList[i].m_ullOreUid;
            TmpUidVector.push_back((void*)&TmpDataKeyArry[i]);
        }
        else if ((i - m_stPlayerInfo.m_bExploreOreCount) < m_stPlayerInfo.m_bOwnOreCount)
        {
            TmpDataKeyArry[i].m_bType = DATA_TYPE_ORE;
            TmpDataKeyArry[i].m_ullKey = m_stPlayerInfo.m_OwnOreIdList[i - m_stPlayerInfo.m_bExploreOreCount];
            TmpUidVector.push_back((void*)&TmpDataKeyArry[i]);
        }
        else
        {
            break;
        }
    }
    int iRet = MineDataMgr::Instance().GetData(TmpUidVector, TmpInfoVector);
    if (iRet != 0)
    {
        LOGERR("Uin<%lu> GetMineOre GetData error<%d>", m_stPlayerInfo.m_ullUin, iRet);
        return ERR_SYS;
    }

    MineOre* pstOre = NULL;
    for (int i = 0; i < MAX_MINE_EXPLORE_ORE_NUM + MAX_MINE_OWN_ORE_NUM; i++)
    {
        pstOre = (MineOre*)TmpInfoVector[i];
        if (pstOre == NULL)
        {
            continue;
        }
        if (i < m_stPlayerInfo.m_bExploreOreCount)
        {
            pstOre->GetOreInfo(rstRsp.m_astExploreOreList[rstRsp.m_bExploreOreCount]);
			//赋值玩家对矿的操作状态
			rstRsp.m_astExploreOreList[rstRsp.m_bExploreOreCount].m_bState = m_stPlayerInfo.m_astExploreOreIdList[rstRsp.m_bExploreOreCount].m_bState;
			rstRsp.m_bExploreOreCount++;
        }
        else if ((i - m_stPlayerInfo.m_bExploreOreCount) < m_stPlayerInfo.m_bOwnOreCount)
        {
			pstOre->UpdateName(m_stPlayerInfo.m_szName);
            pstOre->GetOreInfo(rstRsp.m_astOwnOreList[rstRsp.m_bOwnOreCount++]);
        }
        else
        {
            break;
        }
    }

    rstRsp.m_bAwardCount = m_stPlayerInfo.m_bAwardCount;
    if (m_stPlayerInfo.m_bAwardCount != 0)
    {
        memcpy(rstRsp.m_astAwardList, m_stPlayerInfo.m_astAwardList, sizeof(DT_MINE_AWARD)*m_stPlayerInfo.m_bAwardCount);
    }

    rstRsp.m_bComLogCount = m_stPlayerInfo.m_bComLogCount;
    if (m_stPlayerInfo.m_bComLogCount != 0)
    {
        memcpy(rstRsp.m_astComLogList, m_stPlayerInfo.m_astComLogList, sizeof(DT_MINE_COM_LOG)*m_stPlayerInfo.m_bComLogCount);
    }

    rstRsp.m_bRevengeLogCount = m_stPlayerInfo.m_bRevengeLogCount;
    if (m_stPlayerInfo.m_bRevengeLogCount != 0)
    {
        memcpy(rstRsp.m_astRevengeLogList, m_stPlayerInfo.m_astRevengeLogList, sizeof(DT_MINE_REVENGE_LOG)*m_stPlayerInfo.m_bRevengeLogCount);
    }

    LOGWARN("MinePlayer<%lu>::GetAllInfo AwardCnt<%hhu> OwnOreCnt<%hhu> ExploreOreCnt<%hhu>", m_stPlayerInfo.m_ullUin,
        rstRsp.m_bAwardCount, rstRsp.m_bOwnOreCount, rstRsp.m_bExploreOreCount);

    return ERR_NONE;
}

int MinePlayer::GetOwnOreInfo(OUT uint8_t& rbOreCount, OUT DT_MINE_ORE_INFO* pstOreList)
{
	DataKey tmpKey;
	MineOre* pstOre = NULL;
	rbOreCount = 0;
    for (int i = 0; rbOreCount < m_stPlayerInfo.m_bOwnOreCount; i++)
    {
		tmpKey.m_bType = DATA_TYPE_ORE;
		tmpKey.m_ullKey = m_stPlayerInfo.m_OwnOreIdList[i];
		pstOre = (MineOre*) MineDataMgr::Instance().GetData((void*)&tmpKey);
		if (!pstOre)
		{
			LOGERR("Uid<%lu> don't find the ore", m_stPlayerInfo.m_ullUin);
			continue;
		}
		pstOre->GetOreInfo(pstOreList[rbOreCount]);
		rbOreCount++;
    }
    return ERR_NONE;
}


int MinePlayer::DropOre(MineOre& rstOre)
{
	uint64_t ullNow = CGameTime::Instance().GetCurrSecond();

	if (ullNow < rstOre.GetSelectOccupyOutTime()
		|| ullNow < rstOre.GetFightGuardTime())  //战斗保护时间
	{
		//被抢夺中,无法操作
		return ERR_MINE_ORE_IS_GRABBED;
	}
    uint64_t ullOreUid = rstOre.GetUid();
    bool DelTag = false;
    for (int i = 0; i < m_stPlayerInfo.m_bOwnOreCount; i++)
    {
        if (DelTag)
        {
            memcpy(&m_stPlayerInfo.m_OwnOreIdList[i - 1], &m_stPlayerInfo.m_OwnOreIdList[i], sizeof(m_stPlayerInfo.m_OwnOreIdList[0]));
        }
        else
        {
            if (ullOreUid == m_stPlayerInfo.m_OwnOreIdList[i])
            {
                DelTag = true;
            }
        }
    }
    if (!DelTag || rstOre.GetOwner() != m_stPlayerInfo.m_ullUin)
    {
        LOGERR("Uin<%lu> can't drop the ore<%lu>", m_stPlayerInfo.m_ullUin, ullOreUid);
        return ERR_MINE_DROP_FAIL;
    }



	m_stPlayerInfo.m_bOwnOreCount--;
    //  结算奖励
//     if (m_stPlayerInfo.m_bAwardCount >= MAX_MINE_AWARD_LOG_NUM)
//     {
//         LOGERR("Uin<%lu> the AwardList is full, count<%hhu>", m_stPlayerInfo.m_ullUin, m_stPlayerInfo.m_bAwardCount);
//         return ERR_SYS;
//     }

	DT_MINE_AWARD stAward = { 0 };//m_stPlayerInfo.m_astAwardList[m_stPlayerInfo.m_bAwardCount];

    if (ERR_NONE != SettleOneOre(MINE_AWARD_LOG_TYPE_DROP, rstOre, stAward))
    {
        return ERR_NONE;
    }
	this->AddAwardLog(stAward);
	DT_MINE_COM_LOG stComLog = { 0 };
	stComLog.m_bType = MINE_EXPLORE_COMM_LOG_ABANDON_MINE;
	stComLog.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();
	stComLog.m_dwOreId = rstOre.GetId();
	for (int i = 0; i < MIN(stAward.m_bPropNum, MAX_MINE_COM_LOG_PROP_NUM); i++)
	{
		stComLog.m_astPropList[i].m_bItemType = stAward.m_astPropList[i].m_bItemType;
		stComLog.m_astPropList[i].m_dwItemId = stAward.m_astPropList[i].m_dwItemId;
		stComLog.m_astPropList[i].m_dwItemNum = stAward.m_astPropList[i].m_dwItemNum;
		stComLog.m_bPropNum++;
	}
	this->AddComLog(stComLog);

	rstOre.Drop();
    LOGRUN("Uin<%lu> SettleOreAward by drop OK, the count<%hhu>", m_stPlayerInfo.m_ullUin, m_stPlayerInfo.m_bAwardCount);
    return ERR_NONE;
}

int MinePlayer::OccupyOre(MineOre& rstOre, uint8_t bTroopCount, DT_TROOP_INFO* pstTroopInfo, DT_ITEM_MSKILL& rstMSkill)
{
    uint64_t ullOreUid = rstOre.GetUid();
    if (!CheckOreUidInExploreList(ullOreUid)       //不在探索列表里
        || CheckOreUidInOwnList(ullOreUid))        //在占领列表里
    {
        return ERR_MINE_INVALID_ORE;
    }
    if (m_stPlayerInfo.m_bOwnOreCount >= MAX_MINE_OWN_ORE_NUM)
    {
        return ERR_MINE_OWN_ORE_NUM_FULL;
    }

    int iRet = rstOre.Occupy(m_stPlayerInfo.m_szName, m_stPlayerInfo.m_dwSvrAddr, m_stPlayerInfo.m_ullUin,
		m_stPlayerInfo.m_dwLeaderValue, bTroopCount, pstTroopInfo, rstMSkill);
    if (iRet != ERR_NONE)
    {
        return iRet;
    }
    m_stPlayerInfo.m_OwnOreIdList[m_stPlayerInfo.m_bOwnOreCount++] = ullOreUid;
	DT_MINE_COM_LOG stComLog = { 0 };
	stComLog.m_bType = MINE_EXPLORE_COMM_LOG_OCCUPY_EMPTY_MINE;
	stComLog.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();
	stComLog.m_dwOreId = rstOre.GetId();
	AddComLog(stComLog);
    LOGWARN("Uin<%lu> add OccpuyOreUid<%lu>", m_stPlayerInfo.m_ullUin, ullOreUid);

    LOGWARN("Uin<%lu> OwnOreCount<%hhu> Uid=%lu,%lu,%lu ", m_stPlayerInfo.m_ullUin, m_stPlayerInfo.m_bOwnOreCount,
        m_stPlayerInfo.m_OwnOreIdList[0], m_stPlayerInfo.m_OwnOreIdList[1], m_stPlayerInfo.m_OwnOreIdList[2]);

    return ERR_NONE;
}


int MinePlayer::DelOwnOreUid(uint64_t ullOreUid)
{
	if (ullOreUid == 0)
	{
		return ERR_NONE;
	}
	bool bIsDel = false;
	for (int i = 0; i < (int)m_stPlayerInfo.m_bOwnOreCount; i++)
	{
		if (m_stPlayerInfo.m_OwnOreIdList[i] == ullOreUid)
		{
			bIsDel = true;
		}
		if (bIsDel && i < (int)m_stPlayerInfo.m_bOwnOreCount - 1)
		{
			m_stPlayerInfo.m_OwnOreIdList[i] = m_stPlayerInfo.m_OwnOreIdList[i + 1];
		}
	}
	if (bIsDel)
	{
		m_stPlayerInfo.m_bOwnOreCount--;
	}
	return ERR_NONE;
}

int MinePlayer::_AddItemByMerge(uint8_t bItemType, uint32_t dwItemId, uint32_t dwItemNum, OUT DT_MINE_AWARD& rstAward)
{
    DT_ITEM_SIMPLE stTmpItem;
    stTmpItem.m_bItemType = bItemType;
    stTmpItem.m_dwItemId = dwItemId;
    stTmpItem.m_dwItemNum = dwItemNum;
    int iEqual = 0;
    int iIndex = MyBSearch(&stTmpItem, rstAward.m_astPropList, rstAward.m_bPropNum, sizeof(DT_ITEM_SIMPLE), &iEqual, ItemCmp);
    if (iEqual)
    {
        //有,合并
        DT_ITEM_SIMPLE& rstDestItem = rstAward.m_astPropList[iIndex];
        rstDestItem.m_dwItemNum += stTmpItem.m_dwItemNum;
    }
    else
    {
        //插入新的
        //插入新的数据
        if (rstAward.m_bPropNum >= MAX_MINE_AWARD_PROP_NUM)
        {
            LOGERR("mine _AddItemByMerge");
            return ERR_SYS;
        }
        size_t nmemb = (size_t)rstAward.m_bPropNum;
        MyBInsert(&stTmpItem, rstAward.m_astPropList, &nmemb, sizeof(DT_ITEM_SIMPLE), 1, ItemCmp);
        rstAward.m_bPropNum = (int32_t)nmemb;
    }
    return ERR_NONE;
}

void MinePlayer::SendNtf(SSPKG& rstPkgNew, uint8_t bType)
{
    rstPkgNew.m_stHead.m_ullReservId = m_stPlayerInfo.m_dwSvrAddr; //目标ZoneSvr
    rstPkgNew.m_stHead.m_ullUin = m_stPlayerInfo.m_ullUin;
    rstPkgNew.m_stHead.m_wMsgId = SS_MSG_MINE_INFO_NTF;
    rstPkgNew.m_stBody.m_stMineInfoNtf.m_bType = bType;
	LOGWARN("MinePlayer send ntf type<%hhu>", bType);
    MineSvrMsgLayer::Instance().SendToClusterGate(rstPkgNew);
    return;
}

void MinePlayer::SentAddAwardNtf(DT_MINE_AWARD& rstAward)
{
    SSPKG& rstPkgNew = MineSvrMsgLayer::Instance().GetSsPkgData();
    DT_MINE_INFO_NTF& rstInfoNtf = rstPkgNew.m_stBody.m_stMineInfoNtf.m_stInfoNtf;
    memcpy(&rstInfoNtf.m_stAddAward, &rstAward, sizeof(DT_MINE_AWARD));
    SendNtf(rstPkgNew, MINE_NTF_TYPE_ADD_AWARD);
    return;
}



void MinePlayer::SendRevengeLogNtf(DT_MINE_REVENGE_LOG& rstRevengeLog)
{
    SSPKG& rstPkgNew = MineSvrMsgLayer::Instance().GetSsPkgData();
    DT_MINE_INFO_NTF& rstInfoNtf = rstPkgNew.m_stBody.m_stMineInfoNtf.m_stInfoNtf;
    memcpy(&rstInfoNtf.m_stAddRevengeLog, &rstRevengeLog, sizeof(DT_MINE_REVENGE_LOG));
    SendNtf(rstPkgNew, MINE_NTF_TYPE_ADD_REVENGE_LOG);
}

void MinePlayer::SendComLogNtf(DT_MINE_COM_LOG& rstComLog)
{
    SSPKG& rstPkgNew = MineSvrMsgLayer::Instance().GetSsPkgData();
    DT_MINE_INFO_NTF& rstInfoNtf = rstPkgNew.m_stBody.m_stMineInfoNtf.m_stInfoNtf;
    memcpy(&rstInfoNtf.m_stAddComLog, &rstComLog, sizeof(DT_MINE_COM_LOG));
    SendNtf(rstPkgNew, MINE_NTF_TYPE_ADD_COM_LOG);
}


bool MinePlayer::IsInCurSeason()
{
	uint64_t ullNow = CGameTime::Instance().GetCurrSecond();

	//当前时间在赛季时间内
	bool ret = MineLogicMgr::Instance().m_ullSeasonStartTime <= ullNow && ullNow < MineLogicMgr::Instance().m_ullSeasonEndTime;

	//玩家参与时间在赛季时间内
	bool ret2 = m_stPlayerInfo.m_ullJoinTime != 0 && m_stPlayerInfo.m_ullJoinTime >= MineLogicMgr::Instance().m_ullSeasonStartTime
		&& m_stPlayerInfo.m_ullJoinTime < MineLogicMgr::Instance().m_ullSeasonEndTime;

	//当正常赛季结算没有触发时, ret2的判断就有作用了,这样可以修复玩家没有结算的问题
	return ret && ret2;

}

int MinePlayer::ChallengeRequest(MineOre& rstOre)
{
	if (rstOre.GetOwner() == 0)
	{
		//无人矿, 无需抢夺
		return ERR_MINE_NO_ONE_OCCUPY_CANT_CHALLENGE;
	}
    if (!CheckOreUidInExploreList(rstOre.GetUid())
		|| rstOre.GetOwner() == m_stPlayerInfo.m_ullUin
		|| IsSetOreUsedState(rstOre.GetUid()) != ERR_NONE)
    {
        LOGERR("Uin<%lu> challenge request the ore<Uid=%lu>error!", m_stPlayerInfo.m_ullUin, rstOre.GetUid());
        return ERR_MINE_ORE_CANT_CHALLENGE;
    }
    uint64_t ullNow =  CGameTime::Instance().GetCurrSecond();

    if (ullNow < rstOre.GetRealFightGuardTime())
    {
        LOGERR("Uin<%lu> ore<Uid=%lu> in guard time error!", m_stPlayerInfo.m_ullUin, rstOre.GetUid());
        return ERR_MINE_ORE_IN_GUARD_STATE;
    }
    rstOre.SetFightGuardTime(ullNow + MineLogicMgr::Instance().m_dwChallengeGuardTime);
    rstOre.SetChallenger(m_stPlayerInfo.m_ullUin);
    return ERR_NONE;
}


void MinePlayer::UpdateExploreOreList(uint8_t bExploreOreCount, DT_MINE_ORE_INFO* pstExploreOreList)
{
	if (m_stPlayerInfo.m_ullJoinTime == 0 )
	{
		//设置赛季参与时间
		m_stPlayerInfo.m_ullJoinTime = CGameTime::Instance().GetCurrSecond();
	}
    m_stPlayerInfo.m_bExploreOreCount = 0;
	DT_MINE_EXPLORE_ORE_INFO tmp = { 0 };
    size_t nmemb = (size_t)m_stPlayerInfo.m_bExploreOreCount;
	bzero(m_stPlayerInfo.m_astExploreOreIdList, sizeof(m_stPlayerInfo.m_astExploreOreIdList));
    for (int i = 0; i < bExploreOreCount && i < MAX_MINE_EXPLORE_ORE_NUM; i++)
    {
		tmp.m_ullOreUid = pstExploreOreList[i].m_ullUid;
		if (pstExploreOreList[i].m_ullUid == 0)
		{
			//这里 做个保护,暂时还没找到原因 为什么是0
			continue;
		}
        if (!MyBInsert(&tmp, m_stPlayerInfo.m_astExploreOreIdList, &nmemb, sizeof(DT_MINE_EXPLORE_ORE_INFO), 1, CompareExploreOreUid))
        {
            LOGERR("Uin<%lu> UpdateExploreOreList err!, InsertUid<%lu>", m_stPlayerInfo.m_ullUin, tmp.m_ullOreUid);
            break;
        }
        m_stPlayerInfo.m_bExploreOreCount = nmemb;

    }
	LOGWARN("Uin<%lu> Update ExploreOreCount<%hhu> Uid=%lu,%lu,%lu,%lu,%lu Own=%lu,%lu,%lu ", m_stPlayerInfo.m_ullUin, m_stPlayerInfo.m_bExploreOreCount,
		m_stPlayerInfo.m_astExploreOreIdList[0].m_ullOreUid, m_stPlayerInfo.m_astExploreOreIdList[1].m_ullOreUid,
		m_stPlayerInfo.m_astExploreOreIdList[2].m_ullOreUid, m_stPlayerInfo.m_astExploreOreIdList[3].m_ullOreUid,
  		m_stPlayerInfo.m_astExploreOreIdList[4].m_ullOreUid, m_stPlayerInfo.m_OwnOreIdList[0], m_stPlayerInfo.m_OwnOreIdList[1], m_stPlayerInfo.m_OwnOreIdList[2]);
}

int MinePlayer::GetAwardLog(uint8_t bIndex, OUT DT_MINE_AWARD& rstAward)
{
    if (bIndex >= m_stPlayerInfo.m_bAwardCount)
    {
        LOGERR("Uin<%lu> GetAwardLog Index<%hhu> error, Count<%hhu>", m_stPlayerInfo.m_ullUin, bIndex, m_stPlayerInfo.m_bAwardCount);
        return ERR_WRONG_PARA;
    }
    memcpy(&rstAward, &m_stPlayerInfo.m_astAwardList[bIndex], sizeof(DT_MINE_AWARD));
    m_stPlayerInfo.m_astAwardList[bIndex].m_bState = COMMON_AWARD_STATE_DRAWED;
    return ERR_NONE;

}

void MinePlayer::AddComLog(DT_MINE_COM_LOG& rstComLog)
{
    if (m_stPlayerInfo.m_bComLogCount >= MAX_MINE_COM_LOG_NUM)
    {
        memcpy(m_stPlayerInfo.m_astComLogList, m_stPlayerInfo.m_astComLogList + 1, sizeof(DT_MINE_COM_LOG) * m_stPlayerInfo.m_bComLogCount - 1);
        memcpy(&m_stPlayerInfo.m_astComLogList[m_stPlayerInfo.m_bComLogCount-1], &rstComLog, sizeof(DT_MINE_COM_LOG));
    }
    else
    {
        memcpy(&m_stPlayerInfo.m_astComLogList[m_stPlayerInfo.m_bComLogCount++], &rstComLog, sizeof(DT_MINE_COM_LOG));
    }
    SendComLogNtf(rstComLog);
}

void MinePlayer::AddRevengeLog(DT_MINE_REVENGE_LOG& rstRevengeLog)
{
    if (m_stPlayerInfo.m_bRevengeLogCount >= MAX_MINE_FIGHT_LOG_NUM)
    {
        memcpy(m_stPlayerInfo.m_astRevengeLogList, m_stPlayerInfo.m_astRevengeLogList + 1, sizeof(DT_MINE_REVENGE_LOG) * m_stPlayerInfo.m_bRevengeLogCount - 1);
        memcpy(&m_stPlayerInfo.m_astRevengeLogList[m_stPlayerInfo.m_bRevengeLogCount - 1], &rstRevengeLog, sizeof(DT_MINE_REVENGE_LOG));
    }
    else
    {
        memcpy(&m_stPlayerInfo.m_astRevengeLogList[m_stPlayerInfo.m_bRevengeLogCount++], &rstRevengeLog, sizeof(DT_MINE_REVENGE_LOG));
    }
    SendRevengeLogNtf(rstRevengeLog);

}


void MinePlayer::AddAwardLog(DT_MINE_AWARD& rstAward)
{
	bool bIsAllPropZero = true;
	for (int i = 0; i < rstAward.m_bPropNum; i++)
	{
		if (rstAward.m_astPropList[i].m_dwItemNum != 0)
		{
			bIsAllPropZero = false;
			break;
		}
	}
	//所有物品都是0
	if (bIsAllPropZero)
	{
		return;
	}
	if (m_stPlayerInfo.m_bAwardCount >= MAX_MINE_AWARD_LOG_NUM)
	{
		memcpy(m_stPlayerInfo.m_astAwardList, m_stPlayerInfo.m_astAwardList + 1, sizeof(DT_MINE_AWARD) * m_stPlayerInfo.m_bAwardCount - 1);
		memcpy(&m_stPlayerInfo.m_astAwardList[m_stPlayerInfo.m_bAwardCount - 1], &rstAward, sizeof(DT_MINE_AWARD));
	}
	else
	{
		memcpy(&m_stPlayerInfo.m_astAwardList[m_stPlayerInfo.m_bAwardCount++], &rstAward, sizeof(DT_MINE_AWARD));
	}
	SentAddAwardNtf(rstAward);
}


void MinePlayer::UpdateAddr(uint32_t dwAddr)
{
	m_stPlayerInfo.m_dwSvrAddr = dwAddr;

}

int MinePlayer::SettleOneOre(uint8_t bType, MineOre& rstOre, OUT DT_MINE_AWARD& rstAward)
{
	RESMINE* pResMine = CGameDataMgr::Instance().GetResMineMgr().Find(rstOre.GetId());
	if (!pResMine)
	{
		LOGERR("Uin<%lu> the pResMine<%d> is NULL", m_stPlayerInfo.m_ullUin, rstOre.GetId());
		return ERR_SYS;
	}
	uint32_t  dwProduceCount = rstOre.GetProduceCycleNum(pResMine, bType, &rstAward.m_ullCreateTime);
	DT_MINE_ORE_INFO& rstOreInfo = rstOre.GetOreInfo();
	uint32_t dwRealGetPropNum = 0;
    for (int i = 0; i < pResMine->m_bPropsCount && i < MAX_MINE_AWARD_PROP_NUM; i++)
    {
		dwRealGetPropNum = pResMine->m_propsNum[i] * dwProduceCount > rstOreInfo.m_astLostList[i].m_dwItemNum ?
			pResMine->m_propsNum[i] * dwProduceCount - rstOreInfo.m_astLostList[i].m_dwItemNum : 0;
        if (bType == MINE_AWARD_LOG_TYPE_DAILY)
        {
            _AddItemByMerge(pResMine->m_szPropsType[i], pResMine->m_propsId[i],
				dwRealGetPropNum, rstAward);
        }
        else
        {
            rstAward.m_astPropList[i].m_bItemType = pResMine->m_szPropsType[i];
            rstAward.m_astPropList[i].m_dwItemId = pResMine->m_propsId[i];
            rstAward.m_astPropList[i].m_dwItemNum = dwRealGetPropNum * MineLogicMgr::Instance().m_fDropRate;
            rstAward.m_bPropNum++;
        }
    }
    rstAward.m_bState = COMMON_AWARD_STATE_AVAILABLE;
    rstAward.m_bDealType = bType;   //=1#正常结算|2#放弃结算|3#抢夺
    //rstAward.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();
	rstAward.m_dwOreId = rstOreInfo.m_dwOreId;
    return ERR_NONE;
}


int MinePlayer::LootOre(RESMINE* pResMine, uint8_t bAwardLogType, MineOre& rstOre, DT_MINE_AWARD& rstAward, DT_MINE_COM_LOG& rstComLog, DT_MINE_COM_LOG& rstSelfComLog)
{
	//产出周期个数
	uint32_t dwProduceCount = rstOre.GetProduceCycleNum(pResMine, bAwardLogType);
	DT_MINE_ORE_INFO& rstOreInfo = rstOre.GetOreInfo();
	for (int i = 0; i < pResMine->m_bPropsCount && i < MAX_MINE_AWARD_PROP_NUM && i < MAX_MINE_COM_LOG_PROP_NUM; i++)
	{
		//累积产出
		uint32_t dwAccProduceNum = pResMine->m_propsNum[i] * dwProduceCount > rstOreInfo.m_astLostList[i].m_dwItemNum ?
			pResMine->m_propsNum[i] * dwProduceCount - rstOreInfo.m_astLostList[i].m_dwItemNum : 0;
		uint32_t dwPropLootNum = 0;
		if (rstOreInfo.m_bBeLootedCount < MineLogicMgr::Instance().m_dwBeLootCount)
		{
			dwPropLootNum = MIN(dwAccProduceNum * MineLogicMgr::Instance().m_fLootRate,
				pResMine->m_propsNum[i] * MineLogicMgr::Instance().m_fLootMaxCycleNum); //本次可抢夺资源

			//被抢矿记录 丢失的资源 记录到矿上
			rstOreInfo.m_astLostList[i].m_dwItemNum += dwPropLootNum;				//累加
			rstOreInfo.m_astLostList[i].m_dwItemId = pResMine->m_propsId[i];
			rstOreInfo.m_astLostList[i].m_bItemType = pResMine->m_szPropsType[i];
			rstOreInfo.m_bLostCount = i + 1;										//这里必须是赋值

			//记录到被抢夺者日志记录里
			rstComLog.m_astPropList[i].m_bItemType = pResMine->m_szPropsType[i];
			rstComLog.m_astPropList[i].m_dwItemId = pResMine->m_propsId[i];
			rstComLog.m_astPropList[i].m_dwItemNum = dwPropLootNum; //抢夺者得到
			rstComLog.m_bPropNum++;
		}
		else
		{
			//超过保护次数的抢夺
			dwPropLootNum = pResMine->m_propsNum[i] * MineLogicMgr::Instance().m_fLootLowCycleNum;
		}

		//抢夺者记录得到的资源 到奖励日志里
		rstAward.m_astPropList[i].m_bItemType = pResMine->m_szPropsType[i];
		rstAward.m_astPropList[i].m_dwItemId = pResMine->m_propsId[i];
		rstAward.m_astPropList[i].m_dwItemNum = dwPropLootNum; //抢夺者得到
		rstAward.m_bPropNum++;

		//记录到自己的日志记录里
		rstSelfComLog.m_astPropList[i].m_bItemType = pResMine->m_szPropsType[i];
		rstSelfComLog.m_astPropList[i].m_dwItemId = pResMine->m_propsId[i];
		rstSelfComLog.m_astPropList[i].m_dwItemNum = dwPropLootNum; //抢夺者得到
		rstSelfComLog.m_bPropNum++;
	}
	return ERR_NONE;
}

int MinePlayer::RevengeWin(MineOre& rstOre, uint64_t ullObjUin, uint8_t bRevengeLogIndex)
{
	if (rstOre.GetOwner() != ullObjUin)
	{
		//矿拥者与被复仇的对象不一致
		LOGERR("Uin<%lu> revenge the ore<%lu> of owner<%lu> and the owner lost the ore", m_stPlayerInfo.m_ullUin, rstOre.GetUid(), ullObjUin);
		return ERR_MINE_REVENGE_OBJECT_LOST_ORE;
	}
	uint64_t ullNow = CGameTime::Instance().GetCurrSecond();
	if (rstOre.GetType() == ORE_TYPE_SUPPER)
	{
		//高级矿需要继续保护
		rstOre.SetSelectOccupyOutTime(ullNow + MineLogicMgr::Instance().m_dwSelectOccupyOutTime);
	}
	else
	{
		rstOre.SetChallenger(0);
	}
	//低级矿放开保护
	rstOre.SetFightGuardTime(0);

	MinePlayer* pstObjPlayer = MineDataMgr::Instance().GetMinePlayer(rstOre.GetOwner());
	if (!pstObjPlayer)
	{
		LOGERR("Uin<%lu> find the objPlayer<%lu>  error!", m_stPlayerInfo.m_ullUin, rstOre.GetOwner());
		return ERR_SYS;
	}

	if (bRevengeLogIndex < m_stPlayerInfo.m_bRevengeLogCount)
	{
		m_stPlayerInfo.m_astRevengeLogList[bRevengeLogIndex].m_bState = 1;
	}

	RESMINE* pResMine = CGameDataMgr::Instance().GetResMineMgr().Find(rstOre.GetId());
	if (!pResMine)
	{
		LOGERR("Uin<%lu> the pResMine<%d> is NULL", m_stPlayerInfo.m_ullUin, rstOre.GetId());
		return ERR_SYS;
	}


	DT_MINE_ORE_INFO& rstOreInfo = rstOre.GetOreInfo();
	DT_MINE_AWARD stAward = { 0 };
	stAward.m_bState = COMMON_AWARD_STATE_AVAILABLE;
	stAward.m_bDealType = MINE_AWARD_LOG_TYPE_REVENGE;   //=1#正常结算|2#放弃结算|3#挑战|4#复仇
	stAward.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();
	stAward.m_dwOreId = rstOre.GetId();
	//记录到被复仇对象日志记录里
	DT_MINE_COM_LOG rstComLog = { 0 };
	StrCpy(rstComLog.m_szObjName, m_stPlayerInfo.m_szName, sizeof(rstComLog.m_szObjName));
	rstComLog.m_dwObjAddr = m_stPlayerInfo.m_dwSvrAddr;
	rstComLog.m_bType = MINE_EXPLORE_COMM_LOG_BE_REVENGED_SUCCESS;	//被复仇成功
	rstComLog.m_dwOreId = rstOre.GetId();
	rstComLog.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();



	//记录到自己的日志中
	DT_MINE_COM_LOG stSelfComLog = { 0 };
	StrCpy(stSelfComLog.m_szObjName, pstObjPlayer->GetPlayerName(), sizeof(stSelfComLog.m_szObjName));
	stSelfComLog.m_dwObjAddr = pstObjPlayer->GetPlayerAddr();
	stSelfComLog.m_bType = MINE_EXPLORE_COMM_LOG_REVENGE_SUCCESS;	//自己复仇成功
	stSelfComLog.m_dwOreId = rstOre.GetId();
	stSelfComLog.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();

	LootOre(pResMine, MINE_AWARD_LOG_TYPE_REVENGE, rstOre, stAward, rstComLog, stSelfComLog);

	if (rstOreInfo.m_bBeLootedCount < MineLogicMgr::Instance().m_dwBeLootCount)
	{
		pstObjPlayer->AddComLog(rstComLog);
		rstOre.AddBeLootedCount();
	}


	this->AddAwardLog(stAward);
	this->AddComLog(stSelfComLog);
	return ERR_NONE;

}


int MinePlayer::RevengeLost(MineOre& rstOre, uint64_t ullObjUin)
{
	if (rstOre.GetOwner() != ullObjUin)
	{
		//矿拥者与被复仇的对象不一致
		LOGERR("Uin<%lu> revenge the ore<%lu> of owner<%lu> and the owner lost the ore", m_stPlayerInfo.m_ullUin, rstOre.GetUid(), ullObjUin);
		return ERR_MINE_REVENGE_OBJECT_LOST_ORE;
	}
	rstOre.SetFightGuardTime(0);
	rstOre.SetChallenger(0);
	MinePlayer* pstObjPlayer = MineDataMgr::Instance().GetMinePlayer(rstOre.GetOwner());
	if (!pstObjPlayer)
	{
		LOGERR("Uin<%lu> find the objPlayer<%lu>  error!", m_stPlayerInfo.m_ullUin, rstOre.GetOwner());
		return ERR_SYS;
	}
	//记录到被抢夺者日志记录里
	DT_MINE_COM_LOG rstComLog = { 0 };
	StrCpy(rstComLog.m_szObjName, m_stPlayerInfo.m_szName, sizeof(rstComLog.m_szObjName));
	rstComLog.m_dwObjAddr = m_stPlayerInfo.m_dwSvrAddr;
	rstComLog.m_bType = MINE_EXPLORE_COMM_LOG_BE_REVENGED_FAILED;	//被复仇失败
	rstComLog.m_dwOreId = rstOre.GetId();
	rstComLog.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();

	//记录到自己的日志中
	DT_MINE_COM_LOG stSelfComLog = { 0 };
	StrCpy(stSelfComLog.m_szObjName, pstObjPlayer->GetPlayerName(), sizeof(rstComLog.m_szObjName));
	stSelfComLog.m_dwObjAddr = pstObjPlayer->GetPlayerAddr();
	stSelfComLog.m_bType = MINE_EXPLORE_COMM_LOG_REVENGE_FAILED;	//自己复仇失败
	stSelfComLog.m_dwOreId = rstOre.GetId();
	stSelfComLog.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();

	pstObjPlayer->AddComLog(rstComLog);
	this->AddComLog(stSelfComLog);
	return ERR_NONE;
}

//  每天结算
//
int MinePlayer::SettleDaily()
{
    uint64_t ullCurSettleTime = MineLogicMgr::Instance().m_ullDailySettleAwardTime;
    //uint64_t ullNow = CGameTime::Instance().GetCurrSecond();
    if (m_stPlayerInfo.m_ullLastSettleTime >= ullCurSettleTime)
    {
        return ERR_NONE;
    }
    m_stPlayerInfo.m_ullLastSettleTime = ullCurSettleTime;

	DT_MINE_AWARD rstAward = { 0 };

    bool bIsOneSuccess = false;
    for (int i = 0; i < m_stPlayerInfo.m_bOwnOreCount; i++)
    {
        uint64_t ullOreUid = m_stPlayerInfo.m_OwnOreIdList[i];
        MineOre* pstOre = MineDataMgr::Instance().GetMineOre(ullOreUid);
        if (!pstOre)
        {
            LOGERR("Uin<%lu> when settle, can't find the ore<%lu>", m_stPlayerInfo.m_ullUin, ullOreUid);
            continue;
        }

        if (ERR_NONE == SettleOneOre(MINE_AWARD_LOG_TYPE_DAILY, *pstOre, rstAward))
        {
            bIsOneSuccess = true;
        }
		if (!IsInCurSeason())	//赛季结算
		{
			pstOre->Clear4Reuse();
			MineDataMgr::Instance().AddEmptyNum(pstOre);
			MineDataMgr::Instance().AddToEmptyList(pstOre);
			MineDataMgr::Instance().DelFromTimeList(pstOre);
			MineDataMgr::Instance().AddToDirtyList(pstOre);
		}
		else
		{
			pstOre->ClearBeLootedInfo();
		}
    }
    if (bIsOneSuccess)
    {
		this->AddAwardLog(rstAward);
    }
	SeasonSettle();
    LOGRUN("Uin<%lu> SettleDaily OK, the AwardCount<%hhu>, JoinTime<%lu>",
		m_stPlayerInfo.m_ullUin, m_stPlayerInfo.m_bAwardCount, m_stPlayerInfo.m_ullJoinTime);
	MineDataMgr::Instance().DelFromTimeList(this);
	MineDataMgr::Instance().AddToDirtyList(this);
    return ERR_NONE;
}


int MinePlayer::SeasonSettle()
{
	if (IsInCurSeason())
	{
		return 0;
	}
	SSPKG& rstPkgNew = MineSvrMsgLayer::Instance().GetSsPkgData();
	rstPkgNew.m_stHead.m_wMsgId = SS_MSG_MINE_SEASON_SETTLE_NTF;
	rstPkgNew.m_stHead.m_ullReservId = m_stPlayerInfo.m_dwSvrAddr; //目标ZoneSvr
	rstPkgNew.m_stHead.m_ullUin = m_stPlayerInfo.m_ullUin;
	SS_PKG_MINE_SEASON_SETTLE_NTF& rstSsNtf = rstPkgNew.m_stBody.m_stMineSeasonSettleNtf;
	rstSsNtf.m_ullObjUin = m_stPlayerInfo.m_ullUin;
	rstSsNtf.m_bPropNum = 0;
	//	整理奖励 发邮件
	for (int i = 0; i < m_stPlayerInfo.m_bAwardCount; i++)
	{
		if (m_stPlayerInfo.m_astAwardList[i].m_bState != COMMON_AWARD_STATE_AVAILABLE)
		{
			continue;
		}
		DT_MINE_AWARD& rstAward = m_stPlayerInfo.m_astAwardList[i];
		for (int j = 0; j < rstAward.m_bPropNum; j++)
		{
			int iEqual = 0;
			int iIndex = MyBSearch(&rstAward.m_astPropList[j], rstSsNtf.m_astPropList, rstSsNtf.m_bPropNum, sizeof(DT_ITEM_SIMPLE), &iEqual, ItemCmp);
			if (iEqual)
			{
				//有,合并
				rstSsNtf.m_astPropList[iIndex].m_dwItemNum += rstAward.m_astPropList[j].m_dwItemNum;
			}
			else
			{
				//插入新的
				//插入新的数据
				if (rstSsNtf.m_bPropNum >= sizeof(rstSsNtf.m_astPropList)/sizeof(rstSsNtf.m_astPropList[0]))
				{
					LOGERR("mine _AddItemByMerge");
					continue;
				}
				size_t nmemb = (size_t)rstSsNtf.m_bPropNum;
				MyBInsert(&rstAward.m_astPropList[j], rstSsNtf.m_astPropList, &nmemb, sizeof(DT_ITEM_SIMPLE), 1, ItemCmp);
				rstSsNtf.m_bPropNum = (uint8_t)nmemb;
			}
		}
	}
	if (rstSsNtf.m_bPropNum != 0)
	{
		MineSvrMsgLayer::Instance().SendToClusterGate(rstPkgNew);
	}

	SeasonReset();
	return 0;
}


void MinePlayer::SeasonReset()
{
	//删除矿  (数据库中也删除)
	//MineDataMgr::Instance().DelOreFromPlayer(m_stPlayerInfo.m_bOwnOreCount, m_stPlayerInfo.m_OwnOreIdList);
	m_stPlayerInfo.m_ullJoinTime = 0;
	m_stPlayerInfo.m_bExploreOreCount = 0;
	bzero(&m_stPlayerInfo.m_astExploreOreIdList, sizeof(m_stPlayerInfo.m_astExploreOreIdList));
	m_stPlayerInfo.m_bOwnOreCount = 0;
	bzero(&m_stPlayerInfo.m_OwnOreIdList, sizeof(m_stPlayerInfo.m_OwnOreIdList));
	m_stPlayerInfo.m_bAwardCount = 0;
	bzero(&m_stPlayerInfo.m_astAwardList, sizeof(m_stPlayerInfo.m_astAwardList));
	m_stPlayerInfo.m_bRevengeLogCount = 0;
	bzero(&m_stPlayerInfo.m_astRevengeLogList, sizeof(m_stPlayerInfo.m_astRevengeLogList));
	m_stPlayerInfo.m_bComLogCount = 0;
	bzero(&m_stPlayerInfo.m_astComLogList, sizeof(m_stPlayerInfo.m_astComLogList));
}

int MinePlayer::ChallengeWin(MineOre& rstOre)
{

	if (rstOre.GetChallenger() != m_stPlayerInfo.m_ullUin)
	{
		LOGERR("Uin<%lu> the AwardList is full, count<%hhu>", m_stPlayerInfo.m_ullUin, m_stPlayerInfo.m_bAwardCount);
		return ERR_MINE_INVALID_ORE_DEAL_TYPE;
	}

	uint64_t ullNow = CGameTime::Instance().GetCurrSecond();
	if (rstOre.GetType() == ORE_TYPE_SUPPER)
	{
		//高级矿需要 选择占领保护
		rstOre.SetSelectOccupyOutTime(ullNow + MineLogicMgr::Instance().m_dwSelectOccupyOutTime);
	}
	else
	{
		rstOre.SetChallenger(0);
	}
	//低级矿放开保护
	rstOre.SetFightGuardTime(0);

	RESMINE* pResMine = CGameDataMgr::Instance().GetResMineMgr().Find(rstOre.GetId());
	if (!pResMine)
	{
		LOGERR("Uin<%lu> the pResMine<%d> is NULL", m_stPlayerInfo.m_ullUin, rstOre.GetId());
		return ERR_SYS;
	}

	MinePlayer* pstObjPlayer = MineDataMgr::Instance().GetMinePlayer(rstOre.GetOwner());
	if (!pstObjPlayer)
	{
		LOGERR("Uin<%lu> find the objPlayer<%lu>  error!", m_stPlayerInfo.m_ullUin, rstOre.GetOwner());
		return ERR_SYS;
	}
	SetOreUesedState(rstOre.GetUid());


	//添加复仇记录
	DT_MINE_REVENGE_LOG rstRevengeLog = { 0 };
	StrCpy(rstRevengeLog.m_szObjName, m_stPlayerInfo.m_szName, sizeof(rstRevengeLog.m_szObjName));
	rstRevengeLog.m_dwObjAddr = m_stPlayerInfo.m_dwSvrAddr;
	rstRevengeLog.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();
	rstRevengeLog.m_wObjLv = m_stPlayerInfo.m_wLv;
	rstRevengeLog.m_dwOreId = rstOre.GetId();
	rstRevengeLog.m_ullObjUin = m_stPlayerInfo.m_ullUin;
	rstRevengeLog.m_wObjIconId = m_stPlayerInfo.m_wIconId;


	//记录到被抢夺者日志记录里
	DT_MINE_COM_LOG rstComLog = { 0 };
	StrCpy(rstComLog.m_szObjName, m_stPlayerInfo.m_szName, sizeof(rstComLog.m_szObjName));
	rstComLog.m_dwObjAddr = m_stPlayerInfo.m_dwSvrAddr;
	rstComLog.m_bType = MINE_EXPLORE_COMM_LOG_BE_SNATCHED_SUCCESS;	//被别人抢夺
	rstComLog.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();
	rstComLog.m_dwOreId = rstOre.GetId();

	//记录到抢夺者日志记录中
	DT_MINE_COM_LOG rstSelfComLog = { 0 };
	StrCpy(rstSelfComLog.m_szObjName, pstObjPlayer->GetPlayerName(), sizeof(rstComLog.m_szObjName));
	rstSelfComLog.m_dwObjAddr = pstObjPlayer->GetPlayerAddr();
	rstSelfComLog.m_bType = MINE_EXPLORE_COMM_LOG_SNATCH_SUCCESS;	//自己抢夺
	rstSelfComLog.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();
	rstSelfComLog.m_dwOreId = rstOre.GetId();

	DT_MINE_AWARD stAward = { 0 };
	LootOre(pResMine, MINE_AWARD_LOG_TYPE_CHALLENGE, rstOre, stAward, rstComLog, rstSelfComLog);

	if (rstOre.GetType() == ORE_TYPE_SUPPER || rstOre.GetBeLootedCount() < MineLogicMgr::Instance().m_dwBeLootCount)
	{
		pstObjPlayer->AddComLog(rstComLog);
		pstObjPlayer->AddRevengeLog(rstRevengeLog);
		rstOre.AddBeLootedCount();
	}

	this->AddComLog(rstSelfComLog);
    stAward.m_bState = COMMON_AWARD_STATE_AVAILABLE;
    stAward.m_bDealType = MINE_AWARD_LOG_TYPE_CHALLENGE;
	stAward.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();
	stAward.m_dwOreId = rstOre.GetId();
	this->AddAwardLog(stAward);

    return ERR_NONE;
}


int MinePlayer::ChallengeLost(MineOre& rstOre)
{
	if (rstOre.GetChallenger() != m_stPlayerInfo.m_ullUin)
	{
		LOGERR("Uin<%lu> the AwardList is full, count<%hhu>", m_stPlayerInfo.m_ullUin, m_stPlayerInfo.m_bAwardCount);
		return ERR_MINE_INVALID_ORE_DEAL_TYPE;
	}
	//失败放开保护
	rstOre.SetFightGuardTime(0);
	rstOre.SetChallenger(0);
	MinePlayer* pstObjPlayer = MineDataMgr::Instance().GetMinePlayer(rstOre.GetOwner());
	if (!pstObjPlayer)
	{
		LOGERR("Uin<%lu> find the objPlayer<%lu>  error!", m_stPlayerInfo.m_ullUin, rstOre.GetOwner());
		return ERR_SYS;
	}

	//记录到被抢夺者日志记录里
	DT_MINE_COM_LOG rstComLog = { 0 };
	StrCpy(rstComLog.m_szObjName, m_stPlayerInfo.m_szName, sizeof(rstComLog.m_szObjName));
	rstComLog.m_dwObjAddr = m_stPlayerInfo.m_dwSvrAddr;
	rstComLog.m_bType = MINE_EXPLORE_COMM_LOG_BE_SNATCHED_FAILED;	//被别人抢夺
	rstComLog.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();
	rstComLog.m_dwOreId = rstOre.GetId();

	//记录到抢夺者日志记录中
	DT_MINE_COM_LOG rstSelfComLog = { 0 };
	StrCpy(rstSelfComLog.m_szObjName, pstObjPlayer->GetPlayerName(), sizeof(rstComLog.m_szObjName));
	rstSelfComLog.m_dwObjAddr = pstObjPlayer->GetPlayerAddr();
	rstSelfComLog.m_bType = MINE_EXPLORE_COMM_LOG_SNATCH_FAILED;	//自己抢夺
	rstSelfComLog.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();
	rstSelfComLog.m_dwOreId = rstOre.GetId();

	pstObjPlayer->AddComLog(rstComLog);
	this->AddComLog(rstSelfComLog);


	return ERR_NONE;
}

int MinePlayer::GrabOre(MineOre& rstOre, uint8_t bTroopCount, DT_TROOP_INFO* pstTroopInfo, DT_ITEM_MSKILL& rstMSkill)
{
    uint64_t ullOreUid = rstOre.GetUid();
    if (rstOre.GetType() != ORE_TYPE_SUPPER
		|| rstOre.GetChallenger() != m_stPlayerInfo.m_ullUin)
    {
        LOGERR("Uin<%lu>  OreChallenger=<%lu> ", m_stPlayerInfo.m_ullUin, rstOre.GetChallenger());
        return ERR_MINE_INVALID_ORE_DEAL_TYPE;
    }
	uint64_t ullOutTime = rstOre.GetSelectOccupyOutTime();
	if (ullOutTime < (uint64_t)CGameTime::Instance().GetCurrSecond() || ullOutTime == 0)
	{
		LOGERR("Uin<%lu> grab ore timeout, SelectOccupyOuttime<%lu> ", m_stPlayerInfo.m_ullUin, ullOutTime);
		return ERR_MINE_INVALID_ORE_DEAL_TYPE;
	}
	if (m_stPlayerInfo.m_bOwnOreCount >= MAX_MINE_OWN_ORE_NUM
		|| CheckOreUidInOwnList(ullOreUid))        //在占领列表里
	{
		LOGERR("Uin<%lu> can't grab the ore<%lu>", m_stPlayerInfo.m_ullUin, ullOreUid);
		return ERR_MINE_INVALID_ORE;
	}
	MinePlayer* pstObjPlayer = MineDataMgr::Instance().GetMinePlayer(rstOre.GetOwner());

	if (pstObjPlayer)
	{
		DT_MINE_AWARD rstAward = { 0 };
		pstObjPlayer->SettleOneOre(MINE_AWARD_LOG_TYPE_BE_GRABED, rstOre, rstAward);
		pstObjPlayer->AddAwardLog(rstAward);
		pstObjPlayer->DelOwnOreUid(ullOreUid);


		//记录到被抢占日志记录里
		DT_MINE_COM_LOG rstComLog = { 0 };
		StrCpy(rstComLog.m_szObjName, m_stPlayerInfo.m_szName, sizeof(rstComLog.m_szObjName));
		rstComLog.m_dwObjAddr = m_stPlayerInfo.m_dwSvrAddr;
		rstComLog.m_bType = MINE_EXPLORE_COMM_LOG_BE_GRABED_ORE;	//被别人抢占
		rstComLog.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();
		rstComLog.m_dwOreId = rstOre.GetId();
		pstObjPlayer->AddComLog(rstComLog);


		//记录到抢占日志记录中
		DT_MINE_COM_LOG rstSelfComLog = { 0 };
		StrCpy(rstSelfComLog.m_szObjName, pstObjPlayer->GetPlayerName(), sizeof(rstComLog.m_szObjName));
		rstSelfComLog.m_dwObjAddr = pstObjPlayer->GetPlayerAddr();
		rstSelfComLog.m_bType = MINE_EXPLORE_COMM_LOG_GRAB_ORE;	//自己抢夺
		rstSelfComLog.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();
		rstSelfComLog.m_dwOreId = rstOre.GetId();
		this->AddComLog(rstSelfComLog);
	}

    int iRet = rstOre.Occupy(m_stPlayerInfo.m_szName, m_stPlayerInfo.m_dwSvrAddr, m_stPlayerInfo.m_ullUin,
		m_stPlayerInfo.m_dwLeaderValue, bTroopCount, pstTroopInfo, rstMSkill);
    if (iRet != ERR_NONE)
    {
        return iRet;
    }





	rstOre.SetSelectOccupyOutTime(0);
    m_stPlayerInfo.m_OwnOreIdList[m_stPlayerInfo.m_bOwnOreCount++] = ullOreUid;

    LOGWARN("Uin<%lu> GrabOre add OccpuyOreUid<%lu>", m_stPlayerInfo.m_ullUin, ullOreUid);
    LOGWARN("Uin<%lu> GrabOre OwnOreCount<%hhu> Uid=%lu,%lu,%lu ", m_stPlayerInfo.m_ullUin, m_stPlayerInfo.m_bOwnOreCount,
        m_stPlayerInfo.m_OwnOreIdList[0], m_stPlayerInfo.m_OwnOreIdList[1], m_stPlayerInfo.m_OwnOreIdList[2]);
    return ERR_NONE;
}

