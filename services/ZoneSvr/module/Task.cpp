#include "Task.h"
#include <algorithm>	// std::shuffle
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand
#include "cs_proto.h"
#include "LogMacros.h"
#include "GameTime.h"
#include "../gamedata/GameDataMgr.h"
#include "../utils/FakeRandom.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "Majesty.h"
#include "Consume.h"
#include "AP.h"
#include "player/Player.h"
#include "Item.h"
#include "ov_res_public.h"
#include "dwlog_svr.h"
#include "ZoneLog.h"
#include "TaskAct.h"
#include "player/PlayerMgr.h"
using namespace std;
using namespace PKGMETA;
using namespace DWLOG;

int TaskInfoCmp(const void *pstFirst, const void *pstSecond)
{
    DT_TASK_INFO* pstTaskFirst = (DT_TASK_INFO*)pstFirst;
    DT_TASK_INFO* pstTaskSecond = (DT_TASK_INFO*)pstSecond;

    int iResult = (int)pstTaskFirst->m_dwTaskId - (int)pstTaskSecond->m_dwTaskId;

    return iResult;
}

bool TaskBonusCmp(RESTASKBONUS* pstBonusPrev, RESTASKBONUS* pstBonusNext)
{
	return (pstBonusPrev->m_dwCondValue < pstBonusNext->m_dwCondValue);
}

Task::Task()
{

}

Task::~Task()
{
	MapTask_t::iterator iter = m_dictTask.begin();
	for (; iter != m_dictTask.end(); iter++)
	{
		delete iter->second;
		iter->second = NULL;
	}
	m_dictTask.clear();
	m_setTaskMajestyLvCond.clear();
	m_setTaskGuildCond.clear();
	m_setTaskPvpCond.clear();
	m_setTaskVipLvCond.clear();
}

bool Task::_InitClassifyTask(RESTASK* pResTask)
{
    // 每日更新任务
    //      任务类型更改了,以前的每日任务类型中的公会相关任务独立出来为TASK_TYPE_GUILD 刷新处理逻辑和以前的每日任务一样
    //      分出任务类型是为了前台处理便利
	if (pResTask->m_bType == TASK_TYPE_DAILY || pResTask->m_bType == TASK_TYPE_GUILD || pResTask->m_bType == TASK_TYPE_ACTIVATION)
	{
		// 所有每日任务
		m_listTaskDaily.push_back(pResTask);
		if (pResTask->m_bRandomFlag != 0)
		{
			// 随机每日任务
			m_vectorTaskDailyRandom.push_back(pResTask);
		}
	}
    else if (pResTask->m_bType == TASK_TYPE_CARNIVAL)
    {
        m_vectorTaskCarnival.push_back(pResTask->m_dwId);
    }
    else if (pResTask->m_bType == TASK_TYPE_NEW7D)
    {
        m_vectorTaskNew7D.push_back(pResTask->m_dwId);
        //pResTask->m_dwParam2 表示所属的活动Id
        m_MapAct2Tasks[pResTask->m_dwParam2].push_back(pResTask->m_dwId);
    }
    else if (pResTask->m_bType == TASK_TYPE_PAY)
    {
        m_vectorTaskPay.push_back(pResTask->m_dwId);
    }
    else if (pResTask->m_bType == TASK_TYPE_TOTAL_PAY ||pResTask->m_bType == TASK_TYPE_LOTTERY)
    {
        m_vectorTaskTotalPay.push_back(pResTask->m_dwId);
    }
    else if (pResTask->m_bType == TASK_TYPE_ACT)
    {
        //pResTask->m_dwParam2 表示所属的活动Id
        m_vectorTaskAct.push_back(pResTask->m_dwId);
        m_MapAct2Tasks[pResTask->m_dwParam2].push_back(pResTask->m_dwId);

    }
    else if (pResTask->m_bType == TASK_TYPE_ACT_DAILY)
    {
        m_vectorTaskActDaily.push_back(pResTask->m_dwId);
        m_MapAct2Tasks[pResTask->m_dwParam2].push_back(pResTask->m_dwId);
    }
	else if (pResTask->m_bType == TASK_TYPE_WEB_ACCUMULATE)
	{
		m_vectorTaskWeb.push_back(pResTask->m_dwId);
		m_MapAct2Tasks[pResTask->m_dwParam2].push_back(pResTask->m_dwId);
	}
	else if (pResTask->m_bType == TASK_TYPE_WEB_NORMAL)
	{
		//每日刷新
		m_vectorTaskActDaily.push_back(pResTask->m_dwId);
		//属于web任务
		m_vectorTaskWeb.push_back(pResTask->m_dwId);
		//输入活动任务
		m_MapAct2Tasks[pResTask->m_dwParam2].push_back(pResTask->m_dwId);
	}
    else if (pResTask->m_bType == TASK_TYPE_PASS_GIFT)
    {
        //bugfix
        m_listTaskPassGift.push_back(pResTask);
    }

    return true;
}

#if 0
bool Task::_InitDailyTaskTimeLine()
{
	RESTASK* pResTask = NULL;
	TaskTimeLine stTaskTimeLine;

	m_pqTimeLine.Clear();

	// 每日固定任务
	ListTask_t::iterator iterFixed = m_listTaskDaily.begin();
	for (; iterFixed != m_listTaskDaily.end(); iterFixed++)
	{
		pResTask = *iterFixed;
		if (pResTask->m_bValueType != TASK_VALUE_TYPE_TIME)
		{
			continue;
		}

		stTaskTimeLine.m_dwTaskId = pResTask->m_dwId;

		stTaskTimeLine.m_ullTimeLineSec = CGameTime::Instance().GetSecOfHourInCurrDay(pResTask->m_fStartTime);
		m_pqTimeLine.Push(stTaskTimeLine);

		stTaskTimeLine.m_ullTimeLineSec = CGameTime::Instance().GetSecOfHourInCurrDay(pResTask->m_fFinishTime);
		m_pqTimeLine.Push(stTaskTimeLine);
	}
    return true;
}
#endif

bool Task::_InitTaskIndex(RESTASK* pResTask)
{
	if (pResTask->m_bType == TASK_TYPE_WEB_NORMAL || pResTask->m_bType == TASK_TYPE_WEB_ACCUMULATE)
	{
		m_listTaskWeb.push_back(pResTask);
	}
    ListTask_t* pTaskList = m_dictTask[pResTask->m_bValueType];
	if (pTaskList == NULL)
	{
		pTaskList = new ListTask_t();
		m_dictTask[pResTask->m_bValueType] = pTaskList;
	}

	if (pTaskList != NULL)
	{
		pTaskList->push_back(pResTask);
	}
	else
	{
		LOGERR("pTaskList is null.");
        return false;
	}

    return true;
}

bool Task::_InitTaskCond(RESTASK* pResTask)
{
    if (pResTask->m_bCondIsJoinGuild == 1)
    {
        m_setTaskGuildCond.insert(pResTask->m_dwId);
    }

    if (pResTask->m_wCondMajestyLv != 0)
    {
        m_setTaskMajestyLvCond.insert(pResTask->m_dwId);
    }

    if (pResTask->m_wCondELOId != 0 && (pResTask->m_wCondPVPRankLow != 0 || pResTask->m_wCondPVPRankHigh != 0))
    {
        m_setTaskPvpCond.insert(pResTask->m_dwId);
    }

	if ((pResTask->m_wCondVipLv % 100) != 0)
	{
		m_setTaskVipLvCond.insert(pResTask->m_dwId);
	}


    return true;
}

bool Task::_InitTaskBonus()
{
    ResTaskBonusMgr_t& rstResTaskBonusMgr = CGameDataMgr::Instance().GetResTaskBonusMgr();
    int iCount = rstResTaskBonusMgr.GetResNum();
    for (int i=0; i<iCount; i++)
    {
        RESTASKBONUS* pResTaskBonus = rstResTaskBonusMgr.GetResByPos(i);
        if (!pResTaskBonus)
        {
            LOGERR("InitTaskBonus error: pResTaskBonus <iPos %d> is null", i);
            return false;

        }

        MapTaskBonus_t::iterator iter = m_mapTaskBonus.find(pResTaskBonus->m_dwBonusID);
        if (iter == m_mapTaskBonus.end())
        {
            VectorTaskBonus_t vectorBonus;
            vectorBonus.push_back(pResTaskBonus);
            m_mapTaskBonus.insert(pair<uint32_t /* 礼包Id */, VectorTaskBonus_t /* 礼包奖励数组 */>(pResTaskBonus->m_dwBonusID, vectorBonus));
        }
        else
        {
            VectorTaskBonus_t& rvectorBonus = iter->second;
            rvectorBonus.push_back(pResTaskBonus);
        }
    }

    for (MapTaskBonus_t::iterator iter = m_mapTaskBonus.begin(); iter != m_mapTaskBonus.end(); iter++)
    {
        // 按升序排序
        sort(iter->second.begin(), iter->second.end(), TaskBonusCmp);
    }

    return true;
}

bool Task::Init()
{
	// 时间
	m_ullLastUpdateTimeSec = 0;
    int iHour = CGameTime::Instance().GetCurrHour();
	m_ullLastSyncTimeSec = CGameTime::Instance().GetSecOfHourInCurrDay(iHour);

	RESBASIC* poBasic = CGameDataMgr::Instance().GetResBasicMgr().Find((int)TASK_DAILY_RESET_TIME);
	if (NULL == poBasic)
	{
		return false;
	}

	m_iDailyResetTime = (int)poBasic->m_para[0];

    // 载入所有任务，初始化查找表
    ResTaskMgr_t& rstResTaskMgr = CGameDataMgr::Instance().GetResTaskMgr();
    int iCount = rstResTaskMgr.GetResNum();
    m_iTaskCount = iCount;
    for (int i=0; i<iCount; i++)
    {
        RESTASK* pResTask = rstResTaskMgr.GetResByPos(i);
        if (!pResTask)
        {
            LOGERR("Task init error: pResTask<iPos %d> is NULL", i);
            return false;
        }

        // 每日任务
		if (!_InitClassifyTask(pResTask))
		{
            return false;
		}

        // 任务解锁条件
        if (!_InitTaskCond(pResTask))
        {
            return false;
        }


        // 任务类型索引
		if (!_InitTaskIndex(pResTask))
		{
            return false;
		}
	}

    // 载入任务礼包
    if (!_InitTaskBonus())
    {
        return false;
    }

    return true;
}

bool Task::InitPlayerData(PlayerData* poPlayerData)
{
	// 新玩家首次登录时，初始化
	DT_ROLE_TASK_INFO& rstRoleTaskInfo = poPlayerData->GetTaskInfo();
    rstRoleTaskInfo.m_wNew7DFinishedCnt = 0;
    rstRoleTaskInfo.m_bNew7DFinalAward = 0;
    rstRoleTaskInfo.m_bCarnivalFinalAward = 0;
    rstRoleTaskInfo.m_wCarnivalFinishedCnt = 0;

	ResTaskMgr_t& rstResTaskMgr = CGameDataMgr::Instance().GetResTaskMgr();
	int iCount = rstResTaskMgr.GetResNum();
	int i = 0;
	for (; i<iCount && i<MAX_NUM_TASK; i++)
	{
		RESTASK* pResTask = rstResTaskMgr.GetResByPos(i);

		DT_TASK_INFO& rstTaskInfo = rstRoleTaskInfo.m_astData[i];
		rstTaskInfo.m_dwTaskId = pResTask->m_dwId;
		rstTaskInfo.m_bIsDrawed = COMMON_AWARD_STATE_NONE;
		rstTaskInfo.m_dwStartTime = 0;
		rstTaskInfo.m_dwFinishTime = 0;
		rstTaskInfo.m_dwProgress = 0;
		rstTaskInfo.m_bIsNeedSync = 1;
        rstTaskInfo.m_bIsUnLock = 0;

        if (pResTask->m_bRandomFlag == 0 && !(pResTask->m_bType == TASK_TYPE_ACT
			|| pResTask->m_bType == TASK_TYPE_ACT_DAILY || pResTask->m_bType == TASK_TYPE_NEW7D || pResTask->m_bType == TASK_TYPE_WEB_ACCUMULATE || pResTask->m_bType == TASK_TYPE_WEB_NORMAL))
        {//不是随机任务才判断是否解锁 和某些特殊任务类型的
            TaskUnLock(poPlayerData, &rstTaskInfo);
        }
	}
    //  解锁系统随机后的随机任务
    _UnlockDailyRandomSelectedTask(poPlayerData);

	rstRoleTaskInfo.m_nCount = i;
	rstRoleTaskInfo.m_bIsNeedSync = 1;
	rstRoleTaskInfo.m_ullLastSyncTime = 0;

    // 某些信息在初始化时，就已经有参数
    // 主公等级，初始等级为1级
    ModifyData(poPlayerData, TASK_VALUE_TYPE_LEVEL, 1);
    ModifyData(poPlayerData, TASK_VALUE_TYPE_RANK, poPlayerData->GetELOInfo().m_stPvp6v6Info.m_stBaseInfo.m_bELOLvId/*value*/, 1/*段位*/);
	return true;
}

int Task::UpdateServer()
{
	uint64_t ullUpdateTimeMs = 0;
	if (CGameTime::Instance().IsNeedUpdateByHour(m_ullLastUpdateTimeSec * 1000, m_iDailyResetTime, ullUpdateTimeMs))
	{
		// 重置每日任务

		// 更新时间刷新
		m_ullLastUpdateTimeSec = ullUpdateTimeMs / 1000;

		// 定时重置
		//_InitDailyTaskTimeLine();

        //  随机每日任务  玩家更新时再单独处理
        _DailyTaskRandom();
	}

    if (CGameTime::Instance().GetCurrSecond() - m_ullLastSyncTimeSec > SECONDS_OF_HOUR)
    {
        m_ullLastSyncTimeSec = CGameTime::Instance().GetSecOfHourInCurrDay(CGameTime::Instance().GetCurrHour());
    }

    return 0;
}

int Task::UpdatePlayerData(PlayerData* poPlayerData)
{
    DT_ROLE_TASK_INFO& rstRoleTaskInfo = poPlayerData->GetTaskInfo();

	if (rstRoleTaskInfo.m_ullLastSyncTime < m_ullLastUpdateTimeSec)
	{
		// 每日任务更新
		rstRoleTaskInfo.m_ullLastSyncTime = m_ullLastUpdateTimeSec;
		rstRoleTaskInfo.m_bIsNeedSync = 1;

        //刷新活跃度
        rstRoleTaskInfo.m_wActivation = 0;

		// 跨天,需要更新玩家任务信息表中的固定的每日任务
		ListTask_t::iterator iterDaily = m_listTaskDaily.begin();
		for (; iterDaily != m_listTaskDaily.end(); iterDaily++)
		{
			RESTASK* pResTask = *iterDaily;

			DT_TASK_INFO* pstTaskInfo = this->_Find(poPlayerData, pResTask->m_dwId);
            //当玩家身上没有此任务时,添加此任务
            if (!pstTaskInfo)
            {
                pstTaskInfo = this->_Add(poPlayerData, pResTask->m_dwId);
            }

            //TODEL,为保证老号不导致服务器宕机,临时保护
            if (!pstTaskInfo)
            {
                continue;
            }

            // 随机任务,不在此处解锁,在随机任务中解锁
			if (pResTask->m_bRandomFlag != 0)
			{
				pstTaskInfo->m_bIsNeedSync = 1;
				pstTaskInfo->m_bIsUnLock = 0;
				continue;
			}

			pstTaskInfo->m_bIsNeedSync = 1;
			pstTaskInfo->m_bIsDrawed = COMMON_AWARD_STATE_NONE;
			pstTaskInfo->m_dwProgress = 0;
            pstTaskInfo->m_bIsUnLock = 0;
			pstTaskInfo->m_dwStartTime =0;
			pstTaskInfo->m_dwFinishTime = 0;

			// 解锁
			TaskUnLock(poPlayerData, pstTaskInfo);

		}
		// 每日选择开启的随机任务
		_UnlockDailyRandomSelectedTask(poPlayerData);
        //解锁嘉年华任务
        _UnlockCarnivalTask(poPlayerData);
        //解锁7日任务   是需要每天解锁一个新的
        _UnlockNew7DTask(poPlayerData);
        //解锁充值活动任务
        _UnlockPayTask(poPlayerData);
        //解锁类似充值任务
        _UnlockTotalPayTask(poPlayerData);
        //重置 活动每日任务
        _UnlockActTask(poPlayerData);
    }

    if (rstRoleTaskInfo.m_ullLastSyncTime < m_ullLastSyncTimeSec)
    {
        rstRoleTaskInfo.m_ullLastSyncTime = m_ullLastSyncTimeSec;

        ListTask_t::iterator iterDaily = m_listTaskDaily.begin();
		for (; iterDaily != m_listTaskDaily.end(); iterDaily++)
		{
            RESTASK* pResTask = *iterDaily;
            if (pResTask->m_bRandomFlag != 0 ||
               CGameTime::Instance().GetCurrHour() < (int)pResTask->m_fStartTime ||
               CGameTime::Instance().GetCurrHour() >= (int)pResTask->m_fFinishTime)
            {
                continue;
            }

            DT_TASK_INFO* pstTaskInfo = this->_Find(poPlayerData, pResTask->m_dwId);
            //当玩家身上没有此任务时,添加此任务
            if (!pstTaskInfo)
            {
                pstTaskInfo = this->_Add(poPlayerData, pResTask->m_dwId);
            }

            //TODEL,为保证老号不导致服务器宕机,临时保护
            if (!pstTaskInfo)
            {
                continue;
            }

            // 解锁
			if (TaskUnLock(poPlayerData, pstTaskInfo))
			{
                rstRoleTaskInfo.m_bIsNeedSync = 1;
			}
		}
    }

    //当玩家身上的任务数小于数据档中的任务数，说明数据档有变化，需要更新玩家身上数据
    if (rstRoleTaskInfo.m_nCount < m_iTaskCount)
    {
        UpdatePlayerTask(poPlayerData);
        rstRoleTaskInfo.m_bIsNeedSync = 1;
    }

	if (rstRoleTaskInfo.m_bIsNeedSync != 0)
	{
		// 有需要同步的任务（包含时间相关的任务），这时会附带时间信息，不用再单独发送一次
		this->NotifyClient(poPlayerData);
		rstRoleTaskInfo.m_bIsNeedSync = 0;
	}

    //**检查并自动发送新手7日和嘉年华完成后未领取的奖励
    //** 每个用户都在活动结束后检查一次,并打上标记
    uint64_t ullFirstLoginTime = poPlayerData->GetRoleBaseInfo().m_llFirstLoginTime;
    uint64_t ullNowTime = 0;
    if (rstRoleTaskInfo.m_bNew7DAutoSend == COMMON_DO_SOME_STATE_NONE
        && !TaskAct::Instance().GetActState(poPlayerData, rstRoleTaskInfo.m_dwNew7DCurActId, ullNowTime, ullNowTime))    //延迟60秒检测
    {
        _AutoSendAwardNew7D(poPlayerData);
        rstRoleTaskInfo.m_bNew7DAutoSend = COMMON_DO_SOME_STATE_FINISHED;
        LOGRUN("Uin<%lu> AutoSendAwardNew7D ok!", poPlayerData->m_ullUin);
    }
    ullNowTime = CGameTime::Instance().GetCurrSecond();
    if (rstRoleTaskInfo.m_bCarnivalAutoSend == COMMON_DO_SOME_STATE_NONE
        && (ullFirstLoginTime + CONST_CARNIVAL_DURATION_SEC + 60) < ullNowTime) //延迟60秒检测
    {
        _AutoSendAwardCarnival(poPlayerData);
        rstRoleTaskInfo.m_bCarnivalAutoSend = COMMON_DO_SOME_STATE_FINISHED;
        LOGRUN("Uin<%lu> AutoSendAwardCarnival ok!", poPlayerData->m_ullUin);
    }
    return 0;
}

int Task::UpdatePlayerTask(PlayerData* poPlayerData)
{
    ResTaskMgr_t& rstResTaskMgr = CGameDataMgr::Instance().GetResTaskMgr();
	int iCount = rstResTaskMgr.GetResNum();
    for (int i=0; i<iCount && i<MAX_NUM_TASK; i++)
	{
		RESTASK* pResTask = rstResTaskMgr.GetResByPos(i);

		DT_TASK_INFO* pstTaskInfo = _Find(poPlayerData, pResTask->m_dwId);
        if (!pstTaskInfo)
        {
            pstTaskInfo = _Add(poPlayerData, pResTask->m_dwId);
            if ((pResTask->m_bRandomFlag == 0 && pResTask->m_bType!=TASK_TYPE_PAY )
                || pResTask->m_bType != TASK_TYPE_ACT || pResTask->m_bType != TASK_TYPE_ACT_DAILY
                )
            {//不是随机任务或充值任务才判断是否解锁
                TaskUnLock(poPlayerData, pstTaskInfo);
            }
        }
	}

    return 0;
}


int Task::ModifyData(PlayerData* poPlayerData, int iTaskValueType, uint32_t dwValueChg, uint32_t dwPara1 /*= 0*/, uint32_t dwPara2 /*= 0*/, uint32_t dwPara3 /*= 0*/ )
{

	if (dwValueChg == 0)
	{
		return 0;
	}

	MapTask_t::iterator itDict = m_dictTask.find(iTaskValueType);
	if (itDict != m_dictTask.end())
	{
		ListTask_t* pTaskList = itDict->second;

		ListTask_t::iterator itList = pTaskList->begin();

		for (; itList != pTaskList->end(); itList++)
		{
			RESTASK* pResTask = *itList;
			if (iTaskValueType == TASK_VALUE_TYPE_WEB_ACCUMULATE) //要嵌套完成其他任务
			{
				if (pResTask->m_valuePara[0] == 0 || pResTask->m_valuePara[1] == 0)
				{

					continue;
				}
				if (pResTask->m_valuePara[0] <= dwPara1 && pResTask->m_valuePara[1] >= dwPara1 )
				{
					if (pResTask->m_valuePara[2] == 0)
					{
						//只需嵌套一次
						_ModifyData(poPlayerData, pResTask, dwValueChg);
					}
					else
					{
						int iDoneCnt = 0; //需要完成 pResTask->m_valuePara[2]个子任务,才算一个进度
										  //pResTask->m_valuePara[0],pResTask->m_valuePara[1]表示合法子任务的区间
						for (uint32_t dwCheckCntId = pResTask->m_valuePara[0];  dwCheckCntId <= pResTask->m_valuePara[1]; dwCheckCntId++)
						{
							DT_TASK_INFO *pstTaskInfoTmp = this->_Find(poPlayerData, dwCheckCntId);
							if (!pstTaskInfoTmp)
							{
								continue;
							}

							if (pstTaskInfoTmp->m_bIsUnLock != 0 && pstTaskInfoTmp->m_bIsDrawed != COMMON_AWARD_STATE_NONE)
							{
								if (++iDoneCnt >= pResTask->m_valuePara[2])
								{

									_ModifyData(poPlayerData, pResTask, dwValueChg);
									break;
								}
							}
						}

					}
					
				}
			}
			else
			{

				if (pResTask->m_valuePara[0] != 0 && pResTask->m_valuePara[0] != dwPara1)
				{
					continue;
				}
				if (pResTask->m_valuePara[1] != 0 && pResTask->m_valuePara[1] != dwPara2)
				{
					continue;
				}
				if (pResTask->m_valuePara[2] != 0 && pResTask->m_valuePara[2] != dwPara3)
				{
					continue;
				}



				// 是该任务ID需要修改值
				if (_ModifyData(poPlayerData, pResTask, dwValueChg) > 0  && pResTask->m_bNestTaskTag == 1)	//完成嵌套任务
				{
					ModifyData(poPlayerData, TASK_VALUE_TYPE_WEB_ACCUMULATE, 1, pResTask->m_dwId);
				}

			}
		}
	}
	else
	{
		//LOGERR("Player(%s) Uin(%lu) pTaskList not found, iTaskType = %d.",
                //poPlayerData->GetRoleBaseInfo().m_szRoleName, poPlayerData->m_ullUin, iTaskValueType);
		return ERR_NOT_FOUND;
	}

	return 0;

}
void Task::UnlockTask(PlayerData * poPlayerData, int iTaskType)
{
    for (ListTask_t::iterator it = m_listTaskPassGift.begin(); it != m_listTaskPassGift.end(); ++it)
    {
        TaskUnLock(poPlayerData, (*it)->m_dwId);
    }
}
void Task::GetWebTask(uint64_t ullUin, Player* player, list<DT_TASK_INFO*> &list)
{

	PlayerData *oplayerdata = &player->GetPlayerData();
	for (ListTask_t::iterator it = m_listTaskWeb.begin(); it !=m_listTaskWeb.end(); it++)
	{
		DT_TASK_INFO * dt_task_info_temp = _Find(oplayerdata, (*it)->m_dwId);

		if (!dt_task_info_temp || dt_task_info_temp->m_bIsUnLock == 0)
		{
			continue;
		}
		list.push_back(dt_task_info_temp);
	}
}


PKGMETA::DT_TASK_INFO * Task::GetPlayerTaskById(PlayerData* poPlayerdata, uint32_t dwTaskId)
{
	return this->_Find(poPlayerdata, dwTaskId);
}

int Task::NotifyClient(PlayerData* poPlayerData, bool bJustTime /*= false*/)
{
	DT_ROLE_TASK_INFO& rstRoleTaskInfo = poPlayerData->GetTaskInfo();

	if (rstRoleTaskInfo.m_nCount > MAX_NUM_TASK)
	{
		LOGERR("Uin<%lu> rstInfo.m_nCount > MAX_NUM_TASK", poPlayerData->m_ullUin);
		rstRoleTaskInfo.m_nCount = MAX_NUM_TASK;
	}

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_TASK_INFO_SYN;
	SC_PKG_TASK_INFO_SYN& rstScPkgBodyNtf = m_stScPkg.m_stBody.m_stTaskInfoSyn;
	rstScPkgBodyNtf.m_ullSyncTime = CGameTime::Instance().GetCurrSecond();
	rstScPkgBodyNtf.m_nCount = 0;
    rstScPkgBodyNtf.m_wDailyActivation = rstRoleTaskInfo.m_wActivation;
    rstScPkgBodyNtf.m_wWeekActivation = 0;

	if (!bJustTime)
	{
		ResTaskMgr_t& rstResTaskMgr = CGameDataMgr::Instance().GetResTaskMgr();
		for (int i=0; i<rstRoleTaskInfo.m_nCount; i++)
		{
			// XXX: 这儿对数据档有限制，每日任务必须在主线任务前面，主线任务限制等级越来越高
			DT_TASK_INFO& rstTaskInfo = rstRoleTaskInfo.m_astData[i];
			if (rstTaskInfo.m_bIsUnLock == 0)
			{
				RESTASK* pResTask = rstResTaskMgr.Find(rstTaskInfo.m_dwTaskId);
				if (pResTask == NULL)
				{
					continue;
				}

				if (pResTask->m_bType == TASK_TYPE_DAILY)
				{
					// 每日任务开启限制不满足
					// 直接更新，由客户端去做显示判断
				}
				else
				{
					continue;
				}
			}

			if (rstTaskInfo.m_bIsNeedSync != 0)
			{
				rstTaskInfo.m_bIsNeedSync = 0;
				rstScPkgBodyNtf.m_astTaskList[rstScPkgBodyNtf.m_nCount] = rstTaskInfo;
				rstScPkgBodyNtf.m_nCount++;
				//LOGRUN("Task::NotifyClient Uin<%lu> taskid<%u> progress<%u> taskDraw<%u>",
                        //poPlayerData->m_ullUin, rstTaskInfo.m_dwTaskId, rstTaskInfo.m_dwProgress, rstTaskInfo.m_bIsDrawed);
			}
		}
	}

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayerData->m_pOwner, &m_stScPkg);

	return 0;
}

int Task::HandleDrawBonus(PlayerData* poPlayerData, uint32_t dwId, SC_PKG_TASK_DRAW_RSP& rstPkgBodyRsp)
{
	DT_ROLE_TASK_INFO& rstRoleTaskInfo = poPlayerData->GetTaskInfo();
	rstPkgBodyRsp.m_dwTaskId = dwId;
    rstPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    DT_TASK_INFO* pstTaskInfo = this->_Find(poPlayerData, dwId);
    if (!pstTaskInfo)
    {
        LOGERR("Player(%s) Uin(%lu) HandleDrawBonus taskid(%u) error, cant find it!",
                poPlayerData->GetRoleBaseInfo().m_szRoleName, poPlayerData->m_ullUin, dwId);
		return ERR_NOT_FOUND;
    }

    RESTASK* pResTask = CGameDataMgr::Instance().GetResTaskMgr().Find(dwId);
    if (!pResTask)
    {
        LOGERR("Player(%s) Uin(%lu) HandleDrawBonus taskid(%u) error, cant find ResTask!",
                poPlayerData->GetRoleBaseInfo().m_szRoleName, poPlayerData->m_ullUin, dwId);
        return ERR_NOT_FOUND;
    }

    if (pstTaskInfo->m_bIsDrawed != COMMON_AWARD_STATE_AVAILABLE)
    {
        rstPkgBodyRsp.m_nErrNo = ERR_NOT_SATISFY_COND;
        LOGERR("Player(%s) Uin(%lu) HandleDrawBonus task(%u) state(%hhu) error!",
                poPlayerData->GetRoleBaseInfo().m_szRoleName, poPlayerData->m_ullUin, dwId, pstTaskInfo->m_bIsDrawed);
        return ERR_NOT_SATISFY_COND;
    }

    if (pstTaskInfo->m_dwFinishTime != 0 && CGameTime::Instance().GetCurrSecond() > pstTaskInfo->m_dwFinishTime && pResTask->m_bType!=TASK_TYPE_NEW7D)
    {
       LOGERR("Player(%s) Uin(%lu) HandleDrawBonus task(%u) error, task finish time(%u)!",
                poPlayerData->GetRoleBaseInfo().m_szRoleName, poPlayerData->m_ullUin, dwId, pstTaskInfo->m_dwFinishTime);
        return ERR_NOT_SATISFY_COND;
    }

    // 特殊判定一下任务礼包，默认就是第一个参数
    RESPROPS* pstResProp = CGameDataMgr::Instance().GetResPropsMgr().Find(pResTask->m_rewardId[0]);
    if (pstResProp && pstResProp->m_dwType == PROPS_TYPE_TASK_BONUS)
    {
        // 礼包奖励
        MapTaskBonus_t::iterator iterBonus = m_mapTaskBonus.find(pstResProp->m_dwPropsParam/*礼包Id*/);
        if (iterBonus != m_mapTaskBonus.end())
        {
            VectorTaskBonus_t& rstTaskBonus = iterBonus->second;
            RESTASKBONUS stBonusSearch;
            if (rstTaskBonus.empty())
            {
                LOGERR("Uin<%lu> rstTaskBonus is empty TaskBonusId<%u> ",poPlayerData->m_ullUin, pstResProp->m_dwPropsParam);
                return ERR_SYS;
            }

            stBonusSearch.m_bCondition = (*rstTaskBonus[0]).m_bCondition;
            if (stBonusSearch.m_bCondition == 1 /*等级*/)
            {
                stBonusSearch.m_dwCondValue = poPlayerData->GetMajestyInfo().m_wLevel;
            }
            else if (stBonusSearch.m_bCondition == 2 /*段位Id*/)
            {
                stBonusSearch.m_dwCondValue = poPlayerData->GetELOInfo().m_stPvp6v6Info.m_stBaseInfo.m_bELOLvId;
            }

            VectorTaskBonus_t::iterator iterLow = lower_bound(rstTaskBonus.begin(), rstTaskBonus.end(), &stBonusSearch, TaskBonusCmp); /*前闭后开*/
            if (iterLow != rstTaskBonus.end())
            {
                RESTASKBONUS* pstResTaskBonus = *iterLow;

				for (int i=0; i<pstResTaskBonus->m_bRewardCnt && i<RES_MAX_TASK_REWARD_CNT; i++)
				{
					//注明来源
					Item::Instance().RewardItem(poPlayerData, pstResTaskBonus->m_szRewardType[i],
						pstResTaskBonus->m_rewardId[i], pstResTaskBonus->m_rewardNum[i],
						rstPkgBodyRsp.m_stSyncItemInfo, METHOD_TASK_AWARD);
				}
            }
        }
    }
    else
    {
        // 普通奖励
        for (int i = 0; i < pResTask->m_bRewardCnt && i < RES_MAX_TASK_REWARD_CNT; i++)
        {
			//注明来源
			Item::Instance().RewardItem(poPlayerData, pResTask->m_szRewardType[i], pResTask->m_rewardId[i],
                pResTask->m_rewardNum[i], rstPkgBodyRsp.m_stSyncItemInfo, METHOD_TASK_AWARD);
        }
    }

	// 完成领奖，再同步一次
	pstTaskInfo->m_bIsDrawed = COMMON_AWARD_STATE_DRAWED;
	pstTaskInfo->m_bIsNeedSync = 1;
	rstRoleTaskInfo.m_bIsNeedSync = 1;

    // 任务完成领奖时，判断下是否还有后续任务
    if (pResTask && pResTask->m_dwNextTaskId != 0)
    {
        TaskUnLock(poPlayerData, pResTask->m_dwNextTaskId);
    }

	//记录任务日志
    if (!pResTask)
    {
        LOGERR("Uin<%lu> cant find the ResTask<%u>!", poPlayerData->m_ullUin, dwId);
        return ERR_SYS;
    }
    if (pResTask->m_bType == TASK_TYPE_DAILY)
    {
        LOGWARN("This is daily task!");
        ZoneLog::Instance().WriteTaskLog(poPlayerData, pstTaskInfo->m_dwTaskId, "DailyTask");
    }
    else if (pResTask->m_bType == 2)
    {
        LOGWARN("This is main task");
        ZoneLog::Instance().WriteTaskLog(poPlayerData, pstTaskInfo->m_dwTaskId, "MainTask");
    }
    else
    {
        ZoneLog::Instance().WriteTaskLog(poPlayerData, pstTaskInfo->m_dwTaskId, "Else");
    }

    return 0;
}

/*触发任务解锁的情况
* 1 当某一个任务完成，触发其后置任务时
* 2 军团相关的任务，在加入军团时
* 3 主公等级改变
*/
void Task::TaskCondTrigger(PlayerData* poPlayerData, int iTaskCondType)
{
    switch(iTaskCondType)
    {
        case TASK_COND_TYPE_GUILD:
        {
            SetTaskCond_t::iterator iter = m_setTaskGuildCond.begin();
            for (; iter != m_setTaskGuildCond.end(); iter++)
            {
                TaskUnLock(poPlayerData, *iter);
            }
            break;
        }

        case TASK_COND_TYPE_PVP:
        {
            SetTaskCond_t::iterator iter = m_setTaskPvpCond.begin();
            for (; iter != m_setTaskPvpCond.end(); iter++)
            {
                TaskUnLock(poPlayerData, *iter);
            }
            break;
        }

        case TASK_COND_TYPE_MAJESTY_LV:
        {
            SetTaskCond_t::iterator iter = m_setTaskMajestyLvCond.begin();
            for (; iter != m_setTaskMajestyLvCond.end(); iter++)
            {
                TaskUnLock(poPlayerData, *iter);
            }
            break;
        }

		case TASK_COND_TYPE_VIP_LV:
			{
				SetTaskCond_t::iterator iter = m_setTaskVipLvCond.begin();
				for (; iter != m_setTaskVipLvCond.end(); iter++)
				{
					TaskUnLock(poPlayerData, *iter);
				}
				break;
			}

        default:
            return;
    }

    return;
}


void Task::TaskActReopen(PlayerData* poPlayerData, uint32_t dwActId)
{
    MapAct2Tasks_t::iterator iter = m_MapAct2Tasks.find(dwActId);
    if (iter == m_MapAct2Tasks.end())
    {
        LOGERR("Uin<%lu> the ActId<%u> has no tasks", poPlayerData->m_ullUin, dwActId );
        return;
    }
    RESTASK* pResTask = NULL;
    vector<uint32_t>& tmpVector = iter->second;
    for (int i = 0; i < (int)tmpVector.size(); i++)
    {
        DT_TASK_INFO* pstTaskInfo = this->_Find(poPlayerData, tmpVector[i]);
        if (!pstTaskInfo)
        {
            LOGERR("Player(%s) Uin(%lu) TaskActReopen taskid(%u) error, cant find it!",
                poPlayerData->GetRoleBaseInfo().m_szRoleName, poPlayerData->m_ullUin, tmpVector[i]);
            continue;
        }

        pResTask = CGameDataMgr::Instance().GetResTaskMgr().Find(pstTaskInfo->m_dwTaskId);
        if(pResTask == NULL)
        {
            LOGERR("Uin<%lu> the pResTask<%u> is NULL", poPlayerData->m_ullUin, pstTaskInfo->m_dwTaskId);
            continue;
        }
        if(pResTask->m_bIsCntWhenLock == 0) //不是未开启计数,全部重置
        {
            pstTaskInfo->m_bIsDrawed = COMMON_AWARD_STATE_NONE;
            pstTaskInfo->m_dwProgress = 0;
            pstTaskInfo->m_bIsUnLock = 0;
            pstTaskInfo->m_dwStartTime = 0;
            pstTaskInfo->m_dwFinishTime = 0;
        }
        pstTaskInfo->m_bIsNeedSync = 1;
        TaskUnLock(poPlayerData, pstTaskInfo);
    }
}

bool Task::TaskUnLock(PlayerData* poPlayerData, DT_TASK_INFO* pstTaskInfo)
{
    if (!pstTaskInfo)
    {
        return false;
    }

    ResTaskMgr_t& rstResTaskMgr = CGameDataMgr::Instance().GetResTaskMgr();
	RESTASK* pResTask = rstResTaskMgr.Find(pstTaskInfo->m_dwTaskId);
    if (!pResTask)
    {
        LOGERR("Uin<%lu> pResTask<id %u> is null",poPlayerData->m_ullUin, pstTaskInfo->m_dwTaskId);
        return false;
    }

    if (pstTaskInfo->m_bIsDrawed != COMMON_AWARD_STATE_DRAWED /*未领取*/ && pstTaskInfo->m_bIsUnLock == 0 /*未解锁*/)
    {
        //*** 通用的开启条件检查
        // check guild cond
        if ((pResTask->m_bCondIsJoinGuild != 0) && (!poPlayerData->m_bIsJoinGuild))
        {
            // not satisfy cond
            return false;
        }

        // check pvp cond
        uint8_t bELOLvId = poPlayerData->GetELOInfo().m_stPvp6v6Info.m_stBaseInfo.m_bELOLvId;
        if ((pResTask->m_wCondELOId != 0) && (bELOLvId < pResTask->m_wCondELOId))
        {
            // not satisfy cond
            return false;
        }

        uint16_t wRank = 0 ; //poPlayerData->GetELOInfo().m_stPvp6v6Info.m_stBaseInfo.m_wRanking;
        if ((pResTask->m_wCondPVPRankLow != 0 && pResTask->m_wCondPVPRankHigh != 0)
            && (wRank < pResTask->m_wCondPVPRankLow || wRank > pResTask->m_wCondPVPRankHigh))
        {
            // not satisfy cond
            return false;
        }

        // check majest lv
        if ((pResTask->m_wCondMajestyLv != 0) && (poPlayerData->GetMajestyInfo().m_wLevel < pResTask->m_wCondMajestyLv))
        {
            // not satisfy cond
            return false;
        }

		// check vip lv 复合值 后台用取模100 为实际vip等级
		if ( ((pResTask->m_wCondVipLv % 100) != 0) && (poPlayerData->GetMajestyInfo().m_bVipLv < (pResTask->m_wCondVipLv % 100)) )
		{
			// not satisfy cond
			return false;
		}

        // check 前置任务未完成
        if ((pResTask->m_dwPrevTaskId != 0))
        {
            DT_TASK_INFO* pstTaskInfoPrev = _Find(poPlayerData, pResTask->m_dwPrevTaskId);
            if (!pstTaskInfoPrev)
            {
                return false;
            }

            if (pstTaskInfoPrev->m_bIsDrawed == COMMON_AWARD_STATE_NONE)
            {
                return false;
            }
        }

        //***特殊任务类型做单独的开启判断,主要是时间的判断
        bool bUnlock = false;
        do
        {
            if (TASK_TYPE_DAILY == pResTask->m_bType)
            {
                //  在该任务类型中,pResTask.m_fStartTime 代表几点开启
                uint64_t ullEndTime = 0;
                if (_CheckDailyTaskUnlock(poPlayerData, (uint32_t)pResTask->m_fStartTime, (uint32_t)pResTask->m_fFinishTime, ullEndTime))
                {
                    pstTaskInfo->m_dwFinishTime = ullEndTime; //结束时间
                    bUnlock = true;
                    break;
                }
            }
            //  嘉年华任务开启
            else if (TASK_TYPE_CARNIVAL == pResTask->m_bType)
            {
                //  在该任务类型中,pResTask.m_fStartTime 代表第几天开启
                uint64_t ullEndTime = 0;
                if (_CheckCarnivalTaskUnlock(poPlayerData, (uint32_t)pResTask->m_fStartTime, ullEndTime))
                {
                    pstTaskInfo->m_dwFinishTime = ullEndTime;  //  结束时间
                    bUnlock = true;
                    break;
                }
            }
            //  新手七日任务
            else if (TASK_TYPE_NEW7D == pResTask->m_bType)
            {
                //  在该任务类型中,pResTask.m_fStartTime 代表第几天开启
                uint64_t ullEndTime = 0;
                if (_CheckNew7TaskUnlock(poPlayerData, pResTask->m_dwParam2, (uint32_t)pResTask->m_fStartTime, ullEndTime))
                {
                    pstTaskInfo->m_dwFinishTime = ullEndTime;  //  结束时间
                    bUnlock = true;
                    LOGWARN("Uin<%lu> unlock the task<%u> of TASK_TYPE_NEW7D ActId<%u>",
                        poPlayerData->m_ullUin, pstTaskInfo->m_dwTaskId,
                        pResTask->m_dwParam2);
                    break;
                }
            }
            //  充值活动
            else if (TASK_TYPE_PAY == pResTask->m_bType)
            {
                //  在该任务类型中,pResTask.m_fStartTime 代表第几天开启, m_fFinishTime代表第几天结束
                uint64_t ullEndTime = 0;
                if (_CheckPayTaskUnlock(poPlayerData, (uint32_t)pResTask->m_fStartTime, (uint32_t)pResTask->m_fFinishTime, ullEndTime))
                {
                    pstTaskInfo->m_dwFinishTime = ullEndTime; //结束时间
                    bUnlock = true;
                    break;
                }
            }
            //  累计充值活动
            else if (TASK_TYPE_TOTAL_PAY == pResTask->m_bType || pResTask->m_bType == TASK_TYPE_LOTTERY)
            {
                //  在该任务类型中,pResTask.m_fStartTime 代表第几天开启, m_fFinishTime代表第几天结束
                uint64_t ullEndTime = 0;
                if (_CheckPayTaskUnlock(poPlayerData, (uint32_t)pResTask->m_fStartTime, (uint32_t)pResTask->m_fFinishTime, ullEndTime))
                {
                    pstTaskInfo->m_dwFinishTime = ullEndTime; //结束时间
                    bUnlock = true;
                    break;
                }
            }
            //  活动类型的任务
            else if (TASK_TYPE_ACT == pResTask->m_bType || TASK_TYPE_ACT_DAILY == pResTask->m_bType || TASK_TYPE_WEB_NORMAL==pResTask->m_bType||TASK_TYPE_WEB_ACCUMULATE==pResTask->m_bType)
            {
                //pResTask->m_dwParam2 表示活动Id
                uint64_t ullEndTime = 0, ullStartTime = 0;
                if (TaskAct::Instance().GetActState(poPlayerData, pResTask->m_dwParam2, ullStartTime, ullEndTime))
                {
                    pstTaskInfo->m_dwFinishTime = ullEndTime;
                    bUnlock = true;

                    LOGWARN("Uin<%lu> unlock the task<%u> ActId<%u>",
                        poPlayerData->m_ullUin, pstTaskInfo->m_dwTaskId,
                        pResTask->m_dwParam2);
					ModifyData(poPlayerData, TASK_VALUE_TYPE_LEVEL, poPlayerData->GetMajestyInfo().m_wLevel);
                }
            }

            else
            {
                bUnlock = true;
                break;
            }
        } while (0);

        if (bUnlock)
        {// unlock task
            pstTaskInfo->m_bIsUnLock = 1;
            pstTaskInfo->m_bIsNeedSync = 1;
            poPlayerData->GetTaskInfo().m_bIsNeedSync = 1;

            //  任务解锁就自动完成
            if (pResTask->m_bValueType == TASK_VALUE_TYPE_AUTO_FINISH)
            {
                pstTaskInfo->m_dwProgress = 1;
                pstTaskInfo->m_bIsDrawed = COMMON_AWARD_STATE_AVAILABLE;
            }

            //LOGRUN("Uin<%lu> unlock task id <%u>",poPlayerData->m_ullUin, pstTaskInfo->m_dwTaskId);
        }
		return bUnlock;
    }

    return false;
}

bool Task::TaskUnLock(PlayerData* poPlayerData, uint32_t dwId)
{
    ResTaskMgr_t& rstResTaskMgr = CGameDataMgr::Instance().GetResTaskMgr();
    RESTASK* pResTask = rstResTaskMgr.Find(dwId);

    if (pResTask == NULL)
    {
        LOGERR("Task<%u> is NULL but is called to unlock", dwId);
        return false;
    }

    DT_TASK_INFO* pstTaskInfo = _Find(poPlayerData, dwId);
    if (!pstTaskInfo)
    {
        pstTaskInfo = this->_Add(poPlayerData, dwId);
    }

    return TaskUnLock(poPlayerData, pstTaskInfo);
}


void Task::ReopenNew7DActTask(PlayerData* poPlayerData, uint32_t dwActId)
{
    DT_ROLE_TASK_INFO& rstTaskInfo = poPlayerData->GetTaskInfo();
    //结算上一次的活动
    _AutoSendAwardNew7D(poPlayerData);
    rstTaskInfo.m_bNew7DAutoSend = 0;
    rstTaskInfo.m_bNew7DFinalAward = 0;
    rstTaskInfo.m_wNew7DFinishedCnt = 0;
    rstTaskInfo.m_dwNew7DCurActId = dwActId;
    MapAct2Tasks_t::iterator iter = m_MapAct2Tasks.find(dwActId);
    if (iter == m_MapAct2Tasks.end())
    {
        LOGERR("Uin<%lu> the ActId<%u> has no tasks", poPlayerData->m_ullUin, dwActId);
        return;
    }
    RESTASK* pResTask = NULL;
    vector<uint32_t>& tmpVector = iter->second;
    for (int i = 0; i < (int)tmpVector.size(); i++)
    {
        DT_TASK_INFO* pstTaskInfo = this->_Find(poPlayerData, tmpVector[i]);
        if (!pstTaskInfo)
        {
            LOGERR("Uin(%lu) TaskActReopen taskid(%u) error, cant find it!", poPlayerData->m_ullUin, tmpVector[i]);
            continue;
        }
        pResTask = CGameDataMgr::Instance().GetResTaskMgr().Find(pstTaskInfo->m_dwTaskId);
        if(pResTask == NULL)
        {
            LOGERR("Uin<%lu> the pResTask<%u> is NULL", poPlayerData->m_ullUin, pstTaskInfo->m_dwTaskId);
            continue;
        }
        if(pResTask->m_bIsCntWhenLock == 0) //不是未开启计数,全部重置
        {
            pstTaskInfo->m_bIsDrawed = COMMON_AWARD_STATE_NONE;
            pstTaskInfo->m_dwProgress = 0;
            pstTaskInfo->m_bIsUnLock = 0;
            pstTaskInfo->m_dwStartTime = 0;
            pstTaskInfo->m_dwFinishTime = 0;
        }
        pstTaskInfo->m_bIsNeedSync = 1;
		if (pstTaskInfo->m_bIsDrawed == COMMON_AWARD_STATE_AVAILABLE)
		{
			rstTaskInfo.m_wNew7DFinishedCnt++;
		}
		//LOGWARN("Uin<%lu> TaskId<%u> DrawState<%hu> Process<%u> FinishCnt<%hu>", poPlayerData->m_ullUin, pstTaskInfo->m_dwTaskId, pstTaskInfo->m_bIsDrawed,
		//	pstTaskInfo->m_dwProgress, rstTaskInfo.m_wNew7DFinishedCnt);
        TaskUnLock(poPlayerData, pstTaskInfo);
    }

}

int Task::AddActivation(PlayerData* poPlayerData, uint32_t dwValue)
{
    DT_ROLE_TASK_INFO& rstRoleTaskInfo = poPlayerData->GetTaskInfo();
    rstRoleTaskInfo.m_wActivation += dwValue;
    this->ModifyData(poPlayerData, TASK_VALUE_TYPE_ACTIVATION, rstRoleTaskInfo.m_wActivation, 1);

    return rstRoleTaskInfo.m_wActivation;
}


DT_TASK_INFO* Task::_Find(PlayerData* poPlayerData, uint32_t dwId)
{
	m_stTaskInfo.m_dwTaskId = dwId;
	DT_ROLE_TASK_INFO& rstRoleTaskInfo = poPlayerData->GetTaskInfo();
	int iEqual = 0;
	int iIndex = MyBSearch(&m_stTaskInfo, rstRoleTaskInfo.m_astData, rstRoleTaskInfo.m_nCount, sizeof(DT_TASK_INFO), &iEqual, TaskInfoCmp);
	if (!iEqual)
	{
		return NULL;
	}

	return &rstRoleTaskInfo.m_astData[iIndex];
}

DT_TASK_INFO* Task::_Add(PlayerData* poPlayerData, uint32_t dwId)
{
	m_stTaskInfo.m_dwTaskId = dwId;
    m_stTaskInfo.m_bIsDrawed = COMMON_AWARD_STATE_NONE;
    m_stTaskInfo.m_dwStartTime = 0;
    m_stTaskInfo.m_dwFinishTime = 0;
    m_stTaskInfo.m_dwProgress = 0;
    m_stTaskInfo.m_bIsNeedSync = 1;
    m_stTaskInfo.m_bIsUnLock = 0;

	DT_ROLE_TASK_INFO& rstRoleTaskInfo = poPlayerData->GetTaskInfo();

	size_t nmemb = (size_t)rstRoleTaskInfo.m_nCount;
	if (nmemb >= MAX_NUM_TASK)
	{
		LOGERR("rstRoleTaskInfo.m_nCount<%d> reaches the max.", rstRoleTaskInfo.m_nCount);
		return NULL;
	}
	if (MyBInsert(&m_stTaskInfo, rstRoleTaskInfo.m_astData, &nmemb, sizeof(DT_TASK_INFO), 1, TaskInfoCmp))
	{
		rstRoleTaskInfo.m_nCount = (int32_t)nmemb;
	}

	return this->_Find(poPlayerData, dwId);
}

int Task::_Del(PlayerData* poPlayerData, uint32_t dwId)
{
	m_stTaskInfo.m_dwTaskId = dwId;
	DT_ROLE_TASK_INFO& rstInfo = poPlayerData->GetTaskInfo();
	size_t nmemb = (size_t)rstInfo.m_nCount;
	MyBDelete(&m_stTaskInfo, rstInfo.m_astData, &nmemb, sizeof(DT_TASK_INFO), TaskInfoCmp);
	rstInfo.m_nCount = (int)nmemb;
	return 0;
}

int Task::_DelAll(PlayerData* poPlayerData)
{
	DT_ROLE_TASK_INFO& rstRoleTaskInfo = poPlayerData->GetTaskInfo();
	rstRoleTaskInfo.m_nCount = 0;
	return 0;
}

int Task::_ModifyData(PlayerData* poPlayerData, RESTASK* pResTask, uint32_t dwValueChg)
{
	DT_TASK_INFO* pstTaskInfo = this->_Find(poPlayerData, pResTask->m_dwId);
	if (!pstTaskInfo)
	{
		LOGERR("Player(%s) Uin(%lu) Task id(%u) not found",
			poPlayerData->GetRoleBaseInfo().m_szRoleName, poPlayerData->m_ullUin, pResTask->m_dwId);
		return ERR_NOT_FOUND;
	}

	// 判断时间任务还能否继续做
	if ((pstTaskInfo->m_dwFinishTime != 0) && pstTaskInfo->m_dwFinishTime < (uint64_t)CGameTime::Instance().GetCurrSecond())
	{
		// 		LOGERR("Player(%s) Uin(%lu) Task id(%u) is finished, can't modify",
		//                 poPlayerData->GetRoleBaseInfo().m_szRoleName, poPlayerData->m_ullUin, pResTask->m_dwId);
		return ERR_NOT_SATISFY_COND;
	}

	DT_ROLE_TASK_INFO& rstRoleTaskInfo = poPlayerData->GetTaskInfo();
	int iRet = 0;
	if ((pstTaskInfo->m_bIsUnLock == 1 || pResTask->m_bIsCntWhenLock == 1)
		&& pstTaskInfo->m_bIsDrawed == COMMON_AWARD_STATE_NONE)
	{
		if (pResTask->m_bIsAccumulative == 1)
		{
			// 累加
			pstTaskInfo->m_dwProgress += dwValueChg;
		}
		else if (pResTask->m_bIsAccumulative == 0)
		{
			// 单次
			pstTaskInfo->m_dwProgress = dwValueChg;
			//LOGRUN("Uin(%lu)  taskid %d  is done", poPlayerData->m_ullUin, pResTask->m_dwId);
		}
		if (_CompFunc(pstTaskInfo->m_dwProgress, pResTask->m_dwValueDest, pResTask->m_bCompFunc))
		{
			pstTaskInfo->m_bIsDrawed = COMMON_AWARD_STATE_AVAILABLE;
			//LOGWARN("Uin(%lu)  taskid %d  is done", poPlayerData->m_ullUin, pResTask->m_dwId);
			iRet = 1;
			if (TASK_TYPE_CARNIVAL == pResTask->m_bType)
			{//嘉年华总进度增加
				rstRoleTaskInfo.m_wCarnivalFinishedCnt++;
				ZoneLog::Instance().WritePromotionalActivityLog(poPlayerData, 0, 0, rstRoleTaskInfo.m_wCarnivalFinishedCnt, 0, 0);
			}
			else if (TASK_TYPE_NEW7D == pResTask->m_bType)
			{	//新手7日总进度增加

				MapAct2Tasks_t::iterator iter = m_MapAct2Tasks.find(rstRoleTaskInfo.m_dwNew7DCurActId);
				if (iter != m_MapAct2Tasks.end())
				{
					vector<uint32_t>& tmpVector = iter->second;
					vector<uint32_t>::iterator result = find(tmpVector.begin(), tmpVector.end(), pResTask->m_dwId);
					if (result != tmpVector.end())
					{
						//如果是当前活动下的任务,才需要增加进度
						rstRoleTaskInfo.m_wNew7DFinishedCnt++;
						ZoneLog::Instance().WritePromotionalActivityLog(poPlayerData, 0, 0, 0, rstRoleTaskInfo.m_wNew7DFinishedCnt, dwValueChg);
					}
				}
				//LOGWARN("Uin<%lu> TaskId<%u> DrawState<%hu> Process<%u> FinishCnt<%hu>", poPlayerData->m_ullUin, pstTaskInfo->m_dwTaskId, pstTaskInfo->m_bIsDrawed,
				//	pstTaskInfo->m_dwProgress, rstRoleTaskInfo.m_wNew7DFinishedCnt);
			}
			
		}
	}
	// 需要同步到客户端
	pstTaskInfo->m_bIsNeedSync = 1;
	rstRoleTaskInfo.m_bIsNeedSync = 1;
	return iRet;
}

/*
* 0 小于目标值 无用
* 1 等于目标值 无用
* 2 大于目标值 无用
* 3 小于等于目标值 多数是单次比较
* 4 大于等于目标值 大多数普通情况，需要累积计数
*/
bool Task::_CompFunc(uint32_t& dwPara1, uint32_t& dwPara2, uint8_t bType)
{
    switch(bType)
    {
        case 0:
        {
            if (dwPara1 < dwPara2)
            {
                return true;
            }
            break;
        }
        case 1:
        {
            if (dwPara1 == dwPara2)
            {
                return true;
            }
            break;
        }
        case 2:
        {
            if (dwPara1 > dwPara2)
            {
                return true;
            }
            break;
        }
        case 3:
        {
            if (dwPara1 <= dwPara2)
            {
                return true;
            }
            break;
        }
        case 4:
        {
            if (dwPara1 >= dwPara2)
            {
                dwPara1 = dwPara2;  // 数字增长任务，保护上线值
                return true;
            }
            break;
        }
    }


    return false;
}


void Task::_UnlockActTask(PlayerData* poPlayerData)
{
    DT_TASK_INFO* pstTaskInfo = NULL;
    std::vector<uint32_t>::iterator iter = m_vectorTaskActDaily.begin();
    for (; iter != m_vectorTaskActDaily.end(); iter++)
    {
        pstTaskInfo = _Find(poPlayerData, *iter);
        if (!pstTaskInfo)
        {
            LOGERR("Uin<%lu> can't find the task<%u> in PlayerInfo", poPlayerData->m_ullUin, *iter);
            continue;
        }
        pstTaskInfo->m_bIsNeedSync = 1;
        pstTaskInfo->m_bIsDrawed = COMMON_AWARD_STATE_NONE;
        pstTaskInfo->m_dwProgress = 0;
        pstTaskInfo->m_bIsUnLock = 0;
        pstTaskInfo->m_dwStartTime = 0;
        pstTaskInfo->m_dwFinishTime = 0;
        TaskUnLock(poPlayerData, pstTaskInfo);

    }
}

bool Task::_CheckDailyTaskUnlock(PlayerData* poPlayerData, uint32_t dwStartTime, uint32_t dwEndTime, OUT uint64_t& rUllEndTime)
{
    // 在该任务类型中,pResTask.m_fStartTime 代表几点开启
    if ((uint32_t)CGameTime::Instance().GetCurrHour() < dwStartTime)
    {
        return false;
    }

    rUllEndTime = (dwEndTime ==0) ? 0 : CGameTime::Instance().GetSecOfHourInCurrDay(dwEndTime);
    return true;
}


bool Task::_CheckCarnivalTaskUnlock(PlayerData* poPlayerData, uint32_t dwIntervalDay, OUT uint64_t& rUllEndTime)
{
    //  在该任务类型中,pResTask.m_fStartTime 代表第几天开启
    uint64_t ullFirstLoginTime = poPlayerData->GetRoleBaseInfo().m_llFirstLoginTime;
    uint64_t ullNowTime = CGameTime::Instance().GetCurrSecond();
    uint64_t ullNewInitTime = CGameTime::Instance().GetSecOfCycleHourInSomeDay(ullFirstLoginTime, m_iDailyResetTime);
    uint64_t ullTaskEndTime = ullNewInitTime + CONST_CARNIVAL_DURATION_SEC;
    uint64_t ullTaskStartTime = ullNewInitTime + (dwIntervalDay - 1) * SECONDS_OF_DAY;
    if ( !(ullNowTime > ullTaskStartTime && ullNowTime < ullTaskEndTime) )
    {// 超过持续时间
        return false;
    }
    rUllEndTime = ullTaskEndTime;
    return true;
}

bool Task::_CheckNew7TaskUnlock(PlayerData* poPlayerData, uint32_t dwActId, uint32_t dwIntervalDay, OUT uint64_t& rUllEndTime)
{
    uint64_t ullActStartTime = 0;
     if (!TaskAct::Instance().GetActState(poPlayerData, dwActId, ullActStartTime, rUllEndTime))
     {
         return false;
     }
     uint64_t ullNow = CGameTime::Instance().GetCurrSecond();
     //dwIntervalDay 表示第几天开启
     if ( (ullActStartTime + (dwIntervalDay - 1) * SECONDS_OF_DAY) > ullNow)
     {
         return false;
     }
     rUllEndTime = ullActStartTime + dwIntervalDay * SECONDS_OF_DAY;
    return true;
}

bool Task::_CheckPayTaskUnlock(PlayerData* poPlayerData, uint32_t dwStartDay, uint32_t dwEndDay, OUT uint64_t& rUllEndTime)
{
    uint64_t ullFirstLoginTime = poPlayerData->GetRoleBaseInfo().m_llFirstLoginTime;
    uint64_t ullNowTime = CGameTime::Instance().GetCurrSecond();
    uint64_t ullNewInitTime = CGameTime::Instance().GetSecOfCycleHourInSomeDay(ullFirstLoginTime, m_iDailyResetTime);
    uint64_t ullTaskStartTime = ullNewInitTime + (dwStartDay - 1) * SECONDS_OF_DAY;
    uint64_t ullTaskEndTime = ullNewInitTime + (dwEndDay -1) * SECONDS_OF_DAY;
    if ( !(ullNowTime > ullTaskStartTime && ullNowTime < ullTaskEndTime) )
    {// 超过持续时间
        return false;
    }
    rUllEndTime = ullTaskEndTime;
    return true;
}

bool Task::_CheckTotalPayTaskUnlock(PlayerData* poPlayerData, uint32_t dwStartDay, uint32_t dwEndDay,  OUT uint64_t& rUllEndTime)
{
    uint64_t ullFirstLoginTime = poPlayerData->GetRoleBaseInfo().m_llFirstLoginTime;
    uint64_t ullNowTime = CGameTime::Instance().GetCurrSecond();
    uint64_t ullNewInitTime = CGameTime::Instance().GetSecOfCycleHourInSomeDay(ullFirstLoginTime, m_iDailyResetTime);
    uint64_t ullTaskStartTime = ullNewInitTime + (dwStartDay - 1) * SECONDS_OF_DAY;
    uint64_t ullTaskEndTime = ullNewInitTime + (dwEndDay -1) * SECONDS_OF_DAY;
    if ( !(ullNowTime > ullTaskStartTime && ullNowTime < ullTaskEndTime) )
    {// 超过持续时间
        return false;
    }
    rUllEndTime = ullTaskEndTime;
    return true;

}

void Task::_DailyTaskRandom()
{
    std::srand(std::time(0));

    // random_shuffle默认随机函数是使用std::rand(); 对m_vectorTaskDailyRandom进行乱序
    std::random_shuffle(m_vectorTaskDailyRandom.begin(), m_vectorTaskDailyRandom.end());

    m_vectorTaskDailyRandomSelect.clear();
    VectorTask_t::iterator iterRandom = m_vectorTaskDailyRandom.begin();

    // 就是随机取MAX_NUM_TASK_DAILY_RANDOM个
    for (int iCnt = 0; iterRandom != m_vectorTaskDailyRandom.end() && iCnt < MAX_NUM_TASK_DAILY_RANDOM; iterRandom++, iCnt++)
    {
        m_vectorTaskDailyRandomSelect.push_back(*iterRandom);
    }
}

void Task::_UnlockDailyRandomSelectedTask(PlayerData* poPlayerData)
{
    // 每日选择开启的随机任务
    VectorTask_t::iterator iterRandom = m_vectorTaskDailyRandomSelect.begin();
    for (; iterRandom != m_vectorTaskDailyRandomSelect.end(); iterRandom++)
    {
        RESTASK* pResTask = *iterRandom;
        DT_TASK_INFO* pstTaskInfo = this->_Find(poPlayerData, pResTask->m_dwId);
        if (!pstTaskInfo)
        {
            pstTaskInfo = this->_Add(poPlayerData, pResTask->m_dwId);
            if (!pstTaskInfo)
            {
                continue;
            }

        }

        pstTaskInfo->m_bIsNeedSync = 1;
        pstTaskInfo->m_bIsDrawed = COMMON_AWARD_STATE_NONE;
        pstTaskInfo->m_dwProgress = 0;
        pstTaskInfo->m_dwStartTime = CGameTime::Instance().GetSecOfHourInCurrDay(pResTask->m_fStartTime);
        pstTaskInfo->m_dwFinishTime = CGameTime::Instance().GetSecOfHourInCurrDay(pResTask->m_fFinishTime);

        // 解锁
        TaskUnLock(poPlayerData, pstTaskInfo);
    }
}

void Task::_UnlockCarnivalTask(PlayerData* poPlayerData)
{
    std::vector<uint32_t>::iterator iter = m_vectorTaskCarnival.begin();
    for (; iter != m_vectorTaskCarnival.end(); iter++)
    {
        TaskUnLock(poPlayerData, *iter);
    }
}

void Task::_UnlockNew7DTask(PlayerData* poPlayerData)
{
    std::vector<uint32_t>::iterator iter = m_vectorTaskNew7D.begin();
    for (; iter != m_vectorTaskNew7D.end(); iter++)
    {
        TaskUnLock(poPlayerData, *iter);
    }
}

void Task::_UnlockPayTask(PlayerData* poPlayerData)
{
    std::vector<uint32_t>::iterator iter = m_vectorTaskPay.begin();
    for (; iter != m_vectorTaskPay.end(); iter++)
    {
        //每天只解锁一个任务
        if (TaskUnLock(poPlayerData, *iter))
        {
            return;
        }
    }
}

void Task::_UnlockTotalPayTask(PlayerData* poPlayerData)
{
    std::vector<uint32_t>::iterator iter = m_vectorTaskTotalPay.begin();
    for (; iter != m_vectorTaskTotalPay.end(); iter++)
    {
        TaskUnLock(poPlayerData, *iter);
    }
}

int Task::GetTaskFinalAward(PlayerData* poPlayerData, uint8_t bType, SC_PKG_TASK_FINAL_AWARD_RSP& rstRsp)
{
    rstRsp.m_bType = bType;

    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    DT_ROLE_TASK_INFO& rstRoleTaskInfo = poPlayerData->GetTaskInfo();
    uint64_t ullFirstLoginTime = poPlayerData->GetRoleBaseInfo().m_llFirstLoginTime;
    uint64_t ullNewInitTime = CGameTime::Instance().GetSecOfCycleHourInSomeDay(ullFirstLoginTime, m_iDailyResetTime);
    uint64_t ullNowTime = CGameTime::Instance().GetCurrSecond();
    int iRet = ERR_NONE;

    do
    {
        if (TASK_TYPE_CARNIVAL == bType)
        {
            if (rstRoleTaskInfo.m_bCarnivalFinalAward != COMMON_DO_SOME_STATE_NONE)
            {
                iRet = ERR_NOT_SATISFY_COND;
                LOGERR("Uin<%lu> already got  CarnivalFinalAward", poPlayerData->m_ullUin);
                break;
            }

            uint32_t dwFinishedId = rstRoleTaskInfo.m_wCarnivalFinishedCnt * 10 / 100 ; //  百分之10 对应的奖励id为1;
            if (0 == dwFinishedId)
            {
                iRet = ERR_NOT_SATISFY_COND;
                LOGERR("Uin<%lu> the CarnivalTask finished cnt<%hu> less", poPlayerData->m_ullUin, rstRoleTaskInfo.m_wCarnivalFinishedCnt);
                break;
            }

            if ((ullNowTime < ullNewInitTime + CONST_CARNIVAL_DURATION_SEC - SECONDS_OF_DAY) && dwFinishedId < 10)
            {
                iRet = ERR_NOT_SATISFY_COND;
                LOGERR("Uin<%lu> get the carnival final awrad time too early", poPlayerData->m_ullUin);
                break;
            }
            RESTASKFINALAWARD* poResTaskFinalAward = CGameDataMgr::Instance().GetResTaskFinalAwardMgr().Find(dwFinishedId);
            if (NULL == poResTaskFinalAward)
            {
                iRet = ERR_SYS;
                LOGERR("Uin<%lu> get CarnivalTaskFinalAward error. poResTaskFinalAward<id %u> is null", poPlayerData->m_ullUin, dwFinishedId);
                break;
            }
            Item::Instance().RewardItem(poPlayerData, poResTaskFinalAward->m_bRewardType, poResTaskFinalAward->m_dwRewardId,
                poResTaskFinalAward->m_dwRewardNum, rstRsp.m_stSyncItemInfo, METHOD_TASK_CARNIVAL_FINAL_AWARD);
            rstRoleTaskInfo.m_bCarnivalFinalAward = COMMON_DO_SOME_STATE_FINISHED;
            LOGRUN("Uin<%lu> get CarnivalFinalAward OK", poPlayerData->m_ullUin);
            break;
        }
        else if (TASK_TYPE_NEW7D == bType)
        {
            if (rstRoleTaskInfo.m_bNew7DFinalAward != COMMON_DO_SOME_STATE_NONE)
            {// 领奖状态不对
                iRet = ERR_NOT_SATISFY_COND;
                LOGERR("Uin<%lu> has already got New7DFinalAward CurrActId<%u>", poPlayerData->m_ullUin, rstRoleTaskInfo.m_dwNew7DCurActId);
                break;
            }
            if (rstRoleTaskInfo.m_wNew7DFinishedCnt < CONST_NEW7D_FINISH_NUM)
            {// 次数不够
                iRet = ERR_NOT_SATISFY_COND;
                LOGERR("Uin<%lu> the New7DTask finished cnt<%hu> is less CurrActId<%u>", poPlayerData->m_ullUin, rstRoleTaskInfo.m_wNew7DFinishedCnt, rstRoleTaskInfo.m_dwNew7DCurActId);
                break;
            }
            uint32_t dwFinalAwardId = TaskAct::Instance().GetActPara(rstRoleTaskInfo.m_dwNew7DCurActId);
            RESTASKFINALAWARD* poResTaskFinalAward = CGameDataMgr::Instance().GetResTaskFinalAwardMgr().Find(dwFinalAwardId);
            if (NULL == poResTaskFinalAward)
            {
                iRet = ERR_SYS;
                LOGERR("Uin<%lu> get New7DTaskFinalAward error. poResTaskFinalAward<id %u> is null CurrActId<%u>", poPlayerData->m_ullUin, dwFinalAwardId, rstRoleTaskInfo.m_dwNew7DCurActId);
                break;
            }
            Item::Instance().RewardItem(poPlayerData, poResTaskFinalAward->m_bRewardType, poResTaskFinalAward->m_dwRewardId,
                poResTaskFinalAward->m_dwRewardNum, rstRsp.m_stSyncItemInfo, METHOD_TASK_NEW7D_FINAL_AWARD);
            rstRoleTaskInfo.m_bNew7DFinalAward = COMMON_DO_SOME_STATE_FINISHED;
            LOGRUN("Uin<%lu> get New7DFinalAward OK CurrId<%u>", poPlayerData->m_ullUin, rstRoleTaskInfo.m_dwNew7DCurActId);
        }
        else
        {
            iRet = ERR_SYS;
            break;
        }
    } while (0);

    return iRet;
}

void Task::_AutoSendAwardCarnival(PlayerData* pstData)
{
    //**    发送未领取奖励,并添加邮件附件
    RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(CONST_CARNIVAL_MAIL_ID);
    if (NULL == poResPriMail)
    {
        LOGERR("Uin<%lu> can't find the mail<id %u>", pstData->m_ullUin, CONST_CARNIVAL_MAIL_ID);
        return;
    }
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    m_stSsPkg.m_stBody.m_stMailAddReq.m_nUinCount = 1;
    m_stSsPkg.m_stBody.m_stMailAddReq.m_UinList[0] = pstData->m_ullUin;

    DT_MAIL_DATA& rstMailData = m_stSsPkg.m_stBody.m_stMailAddReq.m_stMailData;
    rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
    rstMailData.m_bState = MAIL_STATE_OPENED;
    StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
    StrCpy(rstMailData.m_szContent, poResPriMail->m_szContent, MAX_MAIL_CONTENT_LEN);
    rstMailData.m_ullFromUin = 0;
    rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();
    uint8_t& bAttachCnt = rstMailData.m_bAttachmentCount;
    bAttachCnt = 0;

    RESTASK* pResTask = NULL;
    DT_ROLE_TASK_INFO& rstRoleTaskInfo = pstData->GetTaskInfo();
    DT_TASK_INFO* pstTaskInfo = NULL;
    ResTaskMgr_t& rstResTaskMgr = CGameDataMgr::Instance().GetResTaskMgr();
    std::vector<uint32_t>::iterator iter = m_vectorTaskCarnival.begin();
    for (; iter != m_vectorTaskCarnival.end(); iter++)
    {
        pstTaskInfo = this->_Find(pstData, *iter);
        pResTask = rstResTaskMgr.Find(*iter);
        if (NULL == pstTaskInfo || NULL == pResTask)
        {
            LOGERR("Uin<%lu> the task<id %u> has problem.", pstData->m_ullUin, *iter);
            continue;
        }

        if (COMMON_AWARD_STATE_AVAILABLE != pstTaskInfo->m_bIsDrawed)
        {
            continue;
        }

        // 普通奖励
        for (int i = 0; i < pResTask->m_bRewardCnt && i < RES_MAX_TASK_REWARD_CNT; i++)
        {
            rstMailData.m_astAttachmentList[bAttachCnt].m_bItemType = pResTask->m_szRewardType[i];
            rstMailData.m_astAttachmentList[bAttachCnt].m_dwItemId = pResTask->m_rewardId[i];
            rstMailData.m_astAttachmentList[bAttachCnt].m_iValueChg = pResTask->m_rewardNum[i];
            bAttachCnt++;
            if (bAttachCnt >= MAX_MAIL_ATTACHMENT_NUM)
            {// 附件满了,发送,并重置为0
                ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
                LOGRUN("Uin<%lu>   _AutoSendAwardCarnival the email ok", pstData->m_ullUin);
                bAttachCnt = 0;
            }
        }
        pstTaskInfo->m_bIsDrawed = COMMON_DO_SOME_STATE_FINISHED;
    }

    //**    终极大奖发送
    do
    {
        if (rstRoleTaskInfo.m_bCarnivalFinalAward != COMMON_DO_SOME_STATE_NONE)
        {
            break;
        }

        uint32_t dwFinishedId = rstRoleTaskInfo.m_wCarnivalFinishedCnt * 10 / 100 ; //  百分之10 对应的奖励id为1;
        if (0 == dwFinishedId)
        {
            LOGERR("Uin<%lu> the CarnivalTask finished cnt<%hu> less", pstData->m_ullUin, rstRoleTaskInfo.m_wCarnivalFinishedCnt);
            break;
        }
        RESTASKFINALAWARD* poResTaskFinalAward = CGameDataMgr::Instance().GetResTaskFinalAwardMgr().Find(dwFinishedId);
        if (NULL == poResTaskFinalAward)
        {
            LOGERR("Uin<%lu> get CarnivalTaskFinalAward error. poResTaskFinalAward<id %u> is null", pstData->m_ullUin, dwFinishedId);
            break;
        }
        rstMailData.m_astAttachmentList[bAttachCnt].m_bItemType = poResTaskFinalAward->m_bRewardType;
        rstMailData.m_astAttachmentList[bAttachCnt].m_dwItemId = poResTaskFinalAward->m_dwRewardId;
        rstMailData.m_astAttachmentList[bAttachCnt].m_iValueChg = poResTaskFinalAward->m_dwRewardNum;
        bAttachCnt++;
        rstRoleTaskInfo.m_bCarnivalFinalAward = COMMON_DO_SOME_STATE_FINISHED;
    } while (0);

    if (bAttachCnt > 0)
    {
        ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
        LOGRUN("Uin<%lu>   _AutoSendAwardCarnival the email ok", pstData->m_ullUin);
    }
}

void Task::_AutoSendAwardNew7D(PlayerData* pstData)
{
    LOGWARN("Uin<%lu> AutoSendAwardNew7d start", pstData->m_ullUin);
    //**    发送未领取奖励,并添加邮件附件
    RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(CONST_NEW7D_MAIL_ID);
    if (NULL == poResPriMail)
    {
        LOGERR("Uin<%lu> can't find the mail<id %u> ", pstData->m_ullUin, CONST_NEW7D_MAIL_ID);
        return;
    }
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    m_stSsPkg.m_stBody.m_stMailAddReq.m_nUinCount = 1;
    m_stSsPkg.m_stBody.m_stMailAddReq.m_UinList[0] = pstData->m_ullUin;

    DT_MAIL_DATA& rstMailData = m_stSsPkg.m_stBody.m_stMailAddReq.m_stMailData;
    rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
    rstMailData.m_bState = MAIL_STATE_OPENED;
    StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
    StrCpy(rstMailData.m_szContent, poResPriMail->m_szContent, MAX_MAIL_CONTENT_LEN);
    rstMailData.m_ullFromUin = 0;
    rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();
    uint8_t& bAttachCnt = rstMailData.m_bAttachmentCount;
    bAttachCnt = 0;


    RESTASK* pResTask = NULL;
    DT_ROLE_TASK_INFO& rstRoleTaskInfo = pstData->GetTaskInfo();
    DT_TASK_INFO* pstTaskInfo = NULL;
    ResTaskMgr_t& rstResTaskMgr = CGameDataMgr::Instance().GetResTaskMgr();

    MapAct2Tasks_t::iterator ActIter = m_MapAct2Tasks.find(rstRoleTaskInfo.m_dwNew7DCurActId);
    if (ActIter == m_MapAct2Tasks.end())
    {
        LOGERR("Uin<%lu> the ActId<%u> has no tasks", pstData->m_ullUin, rstRoleTaskInfo.m_dwNew7DCurActId);
        return;
    }
    for (vector<uint32_t>::iterator iter = ActIter->second.begin(); iter != ActIter->second.end(); iter++)
    {
        pstTaskInfo = this->_Find(pstData, *iter);
        pResTask = rstResTaskMgr.Find(*iter);
        if (NULL == pstTaskInfo || NULL == pResTask)
        {
            LOGERR("Uin<%lu> the task<id %u> has problem.", pstData->m_ullUin, *iter);
            continue;
        }
        pstTaskInfo->m_bIsUnLock = 0;
        pstTaskInfo->m_bIsNeedSync = 1;
        if (COMMON_AWARD_STATE_AVAILABLE != pstTaskInfo->m_bIsDrawed)
        {
            continue;
        }
        // 普通奖励
        for (int i = 0; i < pResTask->m_bRewardCnt && i < RES_MAX_TASK_REWARD_CNT; i++)
        {
            rstMailData.m_astAttachmentList[bAttachCnt].m_bItemType = pResTask->m_szRewardType[i];
            rstMailData.m_astAttachmentList[bAttachCnt].m_dwItemId = pResTask->m_rewardId[i];
            rstMailData.m_astAttachmentList[bAttachCnt].m_iValueChg = pResTask->m_rewardNum[i];
            bAttachCnt++;
            if (bAttachCnt >= MAX_MAIL_ATTACHMENT_NUM)
            {// 附件满了,发送,并重置为0
                ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
                LOGRUN("Uin<%lu>   _AutoSendAwardNew7D the email ok", pstData->m_ullUin);
                bAttachCnt = 0;
            }
        }
        pstTaskInfo->m_bIsDrawed = COMMON_AWARD_STATE_DRAWED;
    }
    do
    {
        if (rstRoleTaskInfo.m_bNew7DFinalAward != COMMON_DO_SOME_STATE_NONE)
        {
            break;
        }
        uint32_t dwFinalAwardId = TaskAct::Instance().GetActPara(rstRoleTaskInfo.m_dwNew7DCurActId);
        RESTASKFINALAWARD* poResTaskFinalAward = CGameDataMgr::Instance().GetResTaskFinalAwardMgr().Find(dwFinalAwardId);
        if (NULL == poResTaskFinalAward)
        {
            LOGERR("Uin<%lu> get New7DTaskFinalAward error. poResTaskFinalAward<id %u> is null", pstData->m_ullUin, dwFinalAwardId);
            break;
        }
        if ( rstRoleTaskInfo.m_wNew7DFinishedCnt < CONST_NEW7D_FINISH_NUM )
        {// 次数不够  和 任务过期
            //LOGERR("Uin<%lu> the New7DTask finished cnt<%hu> is less ", pstData->m_ullUin, rstRoleTaskInfo.m_wNew7DFinishedCnt);
            break;
        }
        rstMailData.m_astAttachmentList[bAttachCnt].m_bItemType = poResTaskFinalAward->m_bRewardType;
        rstMailData.m_astAttachmentList[bAttachCnt].m_dwItemId = poResTaskFinalAward->m_dwRewardId;
        rstMailData.m_astAttachmentList[bAttachCnt].m_iValueChg = poResTaskFinalAward->m_dwRewardNum;
        bAttachCnt++;
        rstRoleTaskInfo.m_bNew7DFinalAward = COMMON_DO_SOME_STATE_FINISHED;
    } while (0);

    if (bAttachCnt > 0)
    {
        ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
        LOGRUN("Uin<%lu>   _AutoSendAwardNew7D the email ok", pstData->m_ullUin);
    }

    LOGWARN("Uin<%lu> AutoSendAwardNew7d end", pstData->m_ullUin);
}

