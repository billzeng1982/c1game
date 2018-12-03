#include "TaskAct.h"
#include "TimeCycle.h"
#include <assert.h>
#include "Task.h"
#include "SvrTime.h"
#include "Lottery.h"
#include "Pay.h"

int ActInfoCmp(const void *pstFirst, const void *pstSecond)
{

	if (((DT_ACT_INFO*)pstFirst)->m_dwActId > ((DT_ACT_INFO*)pstSecond)->m_dwActId)
	{
		return 1;
	}
	else if (((DT_ACT_INFO*)pstFirst)->m_dwActId < ((DT_ACT_INFO*)pstSecond)->m_dwActId)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

bool TaskAct::Init()
{
    m_ullLastCheckTime = 0; //保证下个循环 执行UpdateActState() 
    int iNum = CGameDataMgr::Instance().GetResPayActMgr().GetResNum();
    DT_ACT_TIME stActPayTime;
    RESPAYACT* poResPayAct = NULL;
    for (int i = 0; i < iNum; i++)
    {
        poResPayAct = CGameDataMgr::Instance().GetResPayActMgr().GetResByPos(i);
        assert(poResPayAct);
        vector<uint32_t>& rIdVector = m_ActType2ActIdsMap[poResPayAct->m_wActType];
        rIdVector.push_back(poResPayAct->m_dwId);
        if (poResPayAct->m_bOpenType == ACT_OPEN_TYPE_PRIVATE)
        {
            m_PravieActSet.insert(poResPayAct->m_dwId);
            continue;
        }
        stActPayTime.m_dwActId = poResPayAct->m_dwId;
        stActPayTime.m_wActType = poResPayAct->m_wActType;
        stActPayTime.m_ullStartTime = 0;
        stActPayTime.m_ullEndTime = 0;
        stActPayTime.m_bOpen = false;
		 m_ActId2ActTimeMap.insert(make_pair(poResPayAct->m_dwId, stActPayTime));
		
    }
    m_ullActLastUpdateTime = CGameTime::Instance().GetCurrSecond();
    return true;
}

void TaskAct::Update()
{
    uint64_t ullNow = CGameTime::Instance().GetCurrSecond();
    //1分钟检查一次
    if (m_ullLastCheckTime + 30 < ullNow)
    {
        UpdateActState();
        m_ullLastCheckTime = ullNow;
    }
}

void TaskAct::UpdateActState()
{
    RESPAYACT * poResPayAct = NULL;
    map<uint32_t, DT_ACT_TIME>::iterator iter = m_ActId2ActTimeMap.begin();
    for (; iter != m_ActId2ActTimeMap.end(); iter++)
    {
        poResPayAct = CGameDataMgr::Instance().GetResPayActMgr().Find(iter->second.m_dwActId);
        assert(poResPayAct);
        bool bNowState = false;
        for (int i = 0; i < poResPayAct->m_bNum; i++)
        {
            if (ERR_NONE == TimeCycle::Instance().CheckTime(poResPayAct->m_timeIds[i], 0, &iter->second.m_ullStartTime, &iter->second.m_ullEndTime))
            {
                bNowState = true;
                break;
            }
        }
        //活动状态有改变
        if (bNowState && !iter->second.m_bOpen)
        {
            //从关到开
            m_CurOpenActIdSet.insert(iter->first);
            m_ullActLastUpdateTime = CGameTime::Instance().GetCurrSecond();
            iter->second.m_bOpen = bNowState;
        }
        else if (!bNowState && iter->second.m_bOpen)
        {
            //从开到关
            m_CurOpenActIdSet.erase(iter->first);
            iter->second.m_bOpen = bNowState;
            //结算
            if (iter->second.m_wActType == ACT_TYPE_FORCE_RANK_BUY_ACTIVITY)
            {
                Pay::Instance().SettleLiRank();
            }
        }
       

    }
}

void TaskAct::UpdatePlayer(PlayerData* pstPData)
{
    UpdatePublicAct(pstPData);
    UpdatePrivateAct(pstPData);
}


void TaskAct::UpdatePublicAct(PlayerData* pstPData)
{
    DT_ROLE_TASK_INFO& rstTaskInfo = pstPData->GetTaskInfo();
    if (rstTaskInfo.m_ullActLastUpdateTime >= m_ullActLastUpdateTime)
    {
        return;
    }
    DT_ACT_INFO* pstActInfo = NULL;
    for (set<uint32_t>::iterator it = m_CurOpenActIdSet.begin(); it != m_CurOpenActIdSet.end(); it++)
    {
        pstActInfo = FindAct(pstPData, *it);
        if (pstActInfo == NULL)
        {
            pstActInfo = AddAct(pstPData, *it);
            if (pstActInfo == NULL)
            {
                LOGERR("Uin<%lu> add a ActInfo<%u> failed.", pstPData->m_ullUin, *it);
                continue;
            }
        }
        m_ActIdActTimeMapIter = m_ActId2ActTimeMap.find(*it);
        if (m_ActIdActTimeMapIter == m_ActId2ActTimeMap.end())
        {
            LOGERR("Uin<%lu> find the ActId<%u> from m_ActId2ActTimeMap error", pstPData->m_ullUin, *it);
            continue;
        }
        ActOpenDo(pstPData, pstActInfo, &m_ActIdActTimeMapIter->second);
    }
    rstTaskInfo.m_ullActLastUpdateTime = m_ullActLastUpdateTime;
}


void TaskAct::UpdatePrivateAct(PlayerData* pstPData)
{
    uint64_t ullNow = CGameTime::Instance().GetCurrSecond();
    if (pstPData->m_ullPrivateActLastUpdateTime + 40 > ullNow)
    {
        return;
    }
    pstPData->m_ullPrivateActLastUpdateTime = ullNow;
    uint64_t ullRegTime = pstPData->GetRoleBaseInfo().m_llFirstLoginTime;
    DT_ACT_INFO* pstActInfo = NULL;
    RESPAYACT* poResPayAct = NULL;
    map<uint32_t, DT_ACT_TIME>& rPrivateActTimeMap = pstPData->m_PrivateActMap;
    for (set<uint32_t>::iterator it = m_PravieActSet.begin(); it != m_PravieActSet.end(); it++)
    {
        pstActInfo = FindAct(pstPData, *it);
        if (pstActInfo == NULL)
        {
            pstActInfo = AddAct(pstPData, *it);
            if (pstActInfo == NULL)
            {
                LOGERR("Uin<%lu> add a ActInfo<%u> failed.", pstPData->m_ullUin, *it);
                continue;
            }
        }
        poResPayAct = CGameDataMgr::Instance().GetResPayActMgr().Find(*it);
        if (poResPayAct == NULL)
        {
            LOGERR("Uin<%lu> the poResPayAct<%u> is NULL", pstPData->m_ullUin, *it);
            continue;
        }


        uint64_t ullStartTime = 0, ullEndTime = 0;
        bool bNowState = CheckActOpen(poResPayAct, ullRegTime, ullStartTime, ullEndTime);
        m_ActIdActTimeMapIter = rPrivateActTimeMap.find(*it);
        if (m_ActIdActTimeMapIter == rPrivateActTimeMap.end())
        {
            DT_ACT_TIME tmpActTime;
            tmpActTime.m_bOpen = false;
            tmpActTime.m_dwActId = *it;
            tmpActTime.m_ullEndTime = 0;
            tmpActTime.m_ullStartTime = 0;
            tmpActTime.m_wActType = poResPayAct->m_wActType;
            pair<map<uint32_t, DT_ACT_TIME>::iterator, bool> ret = rPrivateActTimeMap.insert(make_pair(*it, tmpActTime));
            if (!ret.second)
            {
                LOGERR("Uin<%lu> insert ActId<%u> to map error", pstPData->m_ullUin, *it);
                continue;
            }
            m_ActIdActTimeMapIter = ret.first;
        }
        //未开启
        m_ActIdActTimeMapIter->second.m_bOpen = bNowState;
        m_ActIdActTimeMapIter->second.m_ullEndTime = ullEndTime;
        m_ActIdActTimeMapIter->second.m_ullStartTime = ullStartTime;
        if (!bNowState)
        {
            continue;
        }
        
        ActOpenDo(pstPData, pstActInfo, &m_ActIdActTimeMapIter->second);
    }
}

bool TaskAct::IsActTypeOpen(PlayerData* pstPData, uint32_t dwType)
{
    uint64_t ullTemp;
    return IsActTypeOpen(pstPData, dwType, ullTemp, ullTemp);
}


bool TaskAct::IsActTypeOpen(PlayerData* pstPData, uint32_t dwType, OUT uint64_t& rStartTime, OUT uint64_t& rEndTime)
{
    vector<uint32_t>& IdList = m_ActType2ActIdsMap[dwType];
    for (uint32_t i = 0; i < IdList.size(); i++)
    {
        if (GetActState(pstPData, IdList[i], rStartTime, rEndTime))
        {
            return true;
        }

    }
    return false;
}

PKGMETA::DT_ACT_INFO* TaskAct::AddAct(PlayerData* pstPData, uint32_t dwId)
{
    DT_ACT_INFO tmpActInfo;
    tmpActInfo.m_dwActId = dwId;
    tmpActInfo.m_ullJoinTime = 0;

    DT_ROLE_TASK_INFO& rstRoleTaskInfo = pstPData->GetTaskInfo();

    size_t nmemb = (size_t)rstRoleTaskInfo.m_wActCount;
    if (nmemb >= MAX_NUM_ACT)
    {
        LOGERR("Uin<%lu> rstRoleTaskInfo.m_wActCount<%d> reaches the max.", pstPData->m_ullUin, (int)rstRoleTaskInfo.m_wActCount);
        return NULL;
    }
    if (MyBInsert(&tmpActInfo, rstRoleTaskInfo.m_astActInfos, &nmemb, sizeof(DT_ACT_INFO), 1, ActInfoCmp))
    {
        rstRoleTaskInfo.m_wActCount = (uint16_t)nmemb;
    }
    return FindAct(pstPData, dwId);
}

PKGMETA::DT_ACT_INFO* TaskAct::FindAct(PlayerData* pstPData, uint32_t dwActId)
{
    DT_ACT_INFO tmpActInfo;
    tmpActInfo.m_dwActId = dwActId;
    DT_ROLE_TASK_INFO& rstRoleTaskInfo = pstPData->GetTaskInfo();
    int iEqual = 0;
    int iIndex = MyBSearch(&tmpActInfo, rstRoleTaskInfo.m_astActInfos, rstRoleTaskInfo.m_wActCount, sizeof(DT_ACT_INFO), &iEqual, ActInfoCmp);
    if (!iEqual)
    {
        return NULL;
    }

    return &rstRoleTaskInfo.m_astActInfos[iIndex];
}


uint32_t TaskAct::GetActPara(uint32_t dwId)
{
    RESPAYACT* poResPayAct = CGameDataMgr::Instance().GetResPayActMgr().Find(dwId);
    if (!poResPayAct)
    {
        return 0;
    }
    return poResPayAct->m_dwParam;
}

bool TaskAct::CheckActOpen(RESPAYACT* poResPayAct, uint64_t ullPara, OUT uint64_t& rStartTime, OUT uint64_t& rEndTime)
{
    for (int i = 0; i < poResPayAct->m_bNum; i++)
    {
        if (ERR_NONE == TimeCycle::Instance().CheckTime(poResPayAct->m_timeIds[i], ullPara, &rStartTime, &rEndTime))
        {
            return true;
        }
    }
    return false;
}


bool TaskAct::GetActState(PlayerData* pstPData, uint32_t dwActId, OUT uint64_t& rStartTime, OUT uint64_t& rEndTime)
{
    set<uint32_t>::iterator it = m_PravieActSet.find(dwActId);
    DT_ACT_INFO* pstActInfo = FindAct(pstPData, dwActId);
    if (!pstActInfo)
    {
        //这里找不到 一般是要等下一次PlayerUpdate
        if (dwActId == 0)
        {
            LOGERR("Uin<%lu GetActState and don't find the pstActInfo<%u>", pstPData->m_ullUin, dwActId);
        }
        return false;
    }
    if (it != m_PravieActSet.end())
    {
        //私有活动
        m_ActIdActTimeMapIter = pstPData->m_PrivateActMap.find(dwActId);
        if (m_ActIdActTimeMapIter == pstPData->m_PrivateActMap.end())
        {
            return false;
        }
        else
        {
            rStartTime = m_ActIdActTimeMapIter->second.m_ullStartTime;
            rEndTime = m_ActIdActTimeMapIter->second.m_ullEndTime;
            return m_ActIdActTimeMapIter->second.m_bOpen && pstActInfo->m_ullJoinTime >= m_ActIdActTimeMapIter->second.m_ullStartTime;
        }
    }
    else
    {
        //共有活动
        m_ActIdActTimeMapIter = m_ActId2ActTimeMap.find(dwActId);
        if (m_ActIdActTimeMapIter == m_ActId2ActTimeMap.end())
        {
            return false;
        }
        else
        {
            rStartTime = m_ActIdActTimeMapIter->second.m_ullStartTime;
            rEndTime = m_ActIdActTimeMapIter->second.m_ullEndTime;
            return m_ActIdActTimeMapIter->second.m_bOpen && pstActInfo->m_ullJoinTime >= m_ActIdActTimeMapIter->second.m_ullStartTime;
        }
    }
}

void TaskAct::ActOpenDo(PlayerData* pstPData, DT_ACT_INFO* pstActInfo, DT_ACT_TIME* pstActTime)
{
    //参与活动的时间比这次开放的时间还小,说明是参加的上一次活动,这里就需要重置 (0也兼容)
    if (pstActInfo->m_ullJoinTime < pstActTime->m_ullStartTime)
    {
        pstActInfo->m_ullJoinTime = CGameTime::Instance().GetCurrSecond();
        if (pstActTime->m_wActType == ACT_TYPE_TASK_NORMAL ||
			pstActTime->m_wActType == ACT_TYPE_TASK_DAILY || pstActTime->m_wActType == ACT_TYPE_WEB_TASK_NORMAL || pstActTime->m_wActType == ACT_TYPE_WEB_TASK_ACCUMULATE)
        {
            Task::Instance().TaskActReopen(pstPData, pstActInfo->m_dwActId);
        }
        //重置 活动抽奖  魂匣
        else if (pstActTime->m_wActType == ACT_TYPE_LOTTERY_ACT_DRAW)
        {
            Lottery::Instance().DrawActReset(pstPData);
        }
        //七日目标 重置
        else if (pstActTime->m_wActType == ACT_TYPE_NEW7DAY)
        {
            Task::Instance().ReopenNew7DActTask(pstPData, pstActInfo->m_dwActId);
        }
    }
}

