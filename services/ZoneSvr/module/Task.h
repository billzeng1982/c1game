#pragma once
#include <list>
#include <map>
#include <set>
#include <vector>
#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "PriorityQueue.h"
#include "player/PlayerData.h"
#include "../gamedata/GameDataMgr.h"


// 任务系统
// 1、通过任务类型、任务参数1、任务参数2、任务参数3来索引具体的任务ID
// 2、根据任务ID修改相应的玩家数据
// 3、任务参数对于不同任务有不同的意义
// 4、任务参数1、任务参数2、任务参数3，这几个参数和传过来参数比对，如果都能匹配上，那么就算找到了一个任务，并对该任务起作用，起作用有个ValueChg值用于起多大作用
// 5、玩家任务信息是使用数组存放的，完成的任务并没有清除，该处待优化

class Task : public TSingleton<Task>
{
public:
	Task();
    ~Task();
public:
    static const uint32_t CONST_PVP_NONE = 255;              //无效类型
    static const uint32_t CONST_PVP_ALL = 3;                 //所有PVP类型
    static const uint32_t CONST_PVP_6V6 = 1;                 //排位
    static const uint32_t CONST_PVP_GUILD_FIGHT = 2;         //军团战
    static const uint32_t CONST_PVP_WEKK_LEAGUE = 4;         //周末挑战赛
    static const uint32_t CONST_PVP_LEISURE = 6;           //休闲赛
	static const uint32_t CONST_PVP_PEAK_ARENA = 6;           //巅峰竞技
    static const uint32_t CONST_PVP_DAILY_CHALLENGE = 5;     //每天挑战|极限挑战
    static const uint32_t CONST_CARNIVAL_DURATION_SEC = 8 * 24 * 3600;         //  嘉年华持续时间(秒)


    static const uint32_t CONST_NEW7D_FINISH_NUM = 5;                          //  新手7日完成N个任务才能领取终极大奖
    static const uint32_t CONST_CARNIVAL_MAIL_ID = 10004;                      //  嘉年华自动发奖邮件ID
    static const uint32_t CONST_NEW7D_MAIL_ID = 10005;                         //  新手7日自动发奖邮件ID

public:
	uint64_t m_ullLastUpdateTimeSec;	// 上次更新相关任务的时间（秒）
	uint64_t m_ullLastSyncTimeSec;		// 上次同步相关任务的时间（秒）

private:
	typedef std::list<RESTASK*> ListTask_t;
	typedef std::vector<RESTASK*> VectorTask_t;
	typedef std::map<int /* 任务类型 */, ListTask_t* /* 任务链表 */> MapTask_t;
	typedef std::set<uint32_t /* 任务Id */> SetTaskCond_t;

	typedef std::vector<RESTASKBONUS*> VectorTaskBonus_t;
	typedef std::map<uint32_t /* 礼包Id */, VectorTaskBonus_t /* 礼包奖励数组 */> MapTaskBonus_t;
    typedef std::map<uint32_t, std::vector<uint32_t> > MapAct2Tasks_t;
	MapTask_t m_dictTask;																// 所有玩家的所有任务字典索引
	ListTask_t m_listTaskDaily;															// 所有每日任务
	ListTask_t m_listTaskWeb;
    ListTask_t m_listTaskPassGift;                                                      //通关任务
	VectorTask_t m_vectorTaskDailyRandom;												// 随机每日任务
    VectorTask_t m_vectorTaskDailyRandomSelect;											// 随机每日任务筛选出来的

    std::vector<uint32_t> m_vectorTaskCarnival;											// 嘉年华任务
    std::vector<uint32_t> m_vectorTaskNew7D;											// 新手7日任务
    std::vector<uint32_t> m_vectorTaskPay;                                              // 新玩家付费活动
    std::vector<uint32_t> m_vectorTaskTotalPay;                                         // 累计充值活动
    std::vector<uint32_t> m_vectorTaskAct;                                              // 普通活动任务
    std::vector<uint32_t> m_vectorTaskActDaily;                                         // 每天重置的活动任务
	std::vector<uint32_t> m_vectorTaskWeb;												//网页任务

    MapAct2Tasks_t m_MapAct2Tasks;                                                      // 活动Id对应的任务
	SetTaskCond_t m_setTaskMajestyLvCond;												// 主公等级相关任务
	SetTaskCond_t m_setTaskGuildCond;													// 军团相关任务
	SetTaskCond_t m_setTaskPvpCond;														// 匹配相关任务
	SetTaskCond_t m_setTaskVipLvCond;													// VIP等级相关任务
	MapTaskBonus_t m_mapTaskBonus;														// 任务礼包奖励数组
    int m_iDailyResetTime;                                                              // 每日重置时间
    int m_iTaskCount;                                                                   // 任务总数

private:
	struct TaskTimeLine
	{
		uint32_t m_dwTaskId;		// 任务ID
		uint64_t m_ullTimeLineSec;	// 时间线(秒)

		bool operator <(const TaskTimeLine& other) const
		{
			return m_ullTimeLineSec < other.m_ullTimeLineSec;
		}
	};

	//PriorityQueue<TaskTimeLine, std::less<TaskTimeLine> > m_pqTimeLine;		// 时间线优先队列（小顶堆）

private:
    PKGMETA::SCPKG m_stScPkg;
    PKGMETA::SSPKG m_stSsPkg;

	DT_TASK_INFO m_stTaskInfo;

public:
	bool Init();
	bool InitPlayerData(PlayerData* poPlayerData);		// 初始化玩家任务信息

	int UpdateServer();								// 更新服务器任务
	int UpdatePlayerData(PlayerData* poPlayerData);	// 更新具体玩家的信息
    int UpdatePlayerTask(PlayerData* poPlayerData);  //当玩家任务与数据档不一致时,更新玩家任务
	PKGMETA::DT_TASK_INFO *GetPlayerTaskById(PlayerData* poPlayerdata, uint32_t dwTaskId);

    //BUGFIX
    //重新解锁一次特殊类型的任务
    void UnlockTask(PlayerData* poPlayerData, int iTaskType);

	void GetWebTask(uint64_t ullUin, Player* player, list<DT_TASK_INFO*> &list);

	//int ModifyWebTask(PlayerData* poPlayerData,uint32_t dwValueChg,)
	// 修改玩家任务信息
	int ModifyData(PlayerData* poPlayerData, int iTaskValueType, uint32_t dwValueChg, uint32_t dwPara1 = 0, uint32_t dwPara2 = 0, uint32_t dwPara3 = 0);

	// 通知玩家任务更新
	int NotifyClient(PlayerData* poPlayerData, bool bJustTime = false);

	// 处理任务领奖
	int HandleDrawBonus(PlayerData* poPlayerData, uint32_t dwId, PKGMETA::SC_PKG_TASK_DRAW_RSP& rstPkgBodyRsp);

    //  领取最终奖励
    int GetTaskFinalAward(PlayerData* poPlayerData, uint8_t bType, PKGMETA::SC_PKG_TASK_FINAL_AWARD_RSP& rstRsp);
	void TaskCondTrigger(PlayerData* poPlayerData, int iTaskCondType);                          //  任务触发器,一些特殊条件触发解锁任务
    void TaskActReopen(PlayerData* poPlayerData, uint32_t dwActId);                        //  任务触发器,有活动开放时触发
	bool TaskUnLock(PlayerData* poPlayerData, DT_TASK_INFO* pstTaskInfo);                       //  解锁任务
	bool TaskUnLock(PlayerData* poPlayerData, uint32_t dwId);                                   //  解锁任务
    void ReopenNew7DActTask(PlayerData* poPlayerData, uint32_t dwActId);                        //  重新开启七日目标任务

	int AddActivation(PlayerData* poPlayerData, uint32_t dwValue);	//增加活跃度

private:
	bool _InitClassifyTask(RESTASK* pResTask);                                                  //  初始分类任务
	//bool _InitDailyTaskTimeLine();
	bool _InitTaskIndex(RESTASK* pResTask);                                                     //  初始加载任务索引
	bool _InitTaskCond(RESTASK* pResTask);                                                      //  初始加载任务条件判断
	bool _InitTaskBonus();                                                                      //  初始加载任务礼包/奖励等

    void _DailyTaskRandom();                                                                    //  随机每日任务
    void _UnlockDailyRandomSelectedTask(PlayerData* poPlayerData);                              //  解锁系统已选择的随机任务
    void _UnlockCarnivalTask(PlayerData* poPlayerData);                                         //  解锁嘉年华任务
    void _UnlockNew7DTask(PlayerData* poPlayerData);                                            //  解锁新手7日任务
    void _UnlockPayTask(PlayerData* poPlayerData);                                              //  解锁充值任务
    void _UnlockTotalPayTask(PlayerData* poPlayerData);                                         //  解锁累计充值任务
    void _UnlockActTask(PlayerData* poPlayerData);                                              //  解锁活动每日任务


    bool _CheckDailyTaskUnlock(PlayerData* poPlayerData, uint32_t dwStartTime, uint32_t dwEndTime, OUT uint64_t& rUllEndTime);
    bool _CheckCarnivalTaskUnlock(PlayerData* poPlayerData, uint32_t dwIntervalDay, OUT uint64_t& rUllEndTime);
    bool _CheckNew7TaskUnlock(PlayerData* poPlayerData, uint32_t dwActId, uint32_t dwIntervalDay, OUT uint64_t& rUllEndTime);
    bool _CheckPayTaskUnlock(PlayerData* poPlayerData, uint32_t dwStartDay, uint32_t dwEndDay,  OUT uint64_t& rUllEndTime);
    bool _CheckTotalPayTaskUnlock(PlayerData* poPlayerData, uint32_t dwStartDay, uint32_t dwEndDay,  OUT uint64_t& rUllEndTime);

    void _AutoSendAwardCarnival(PlayerData* pstData);                                           //  嘉年华活动结束自动通过邮件发送未领取奖励
    void _AutoSendAwardNew7D(PlayerData* pstData);                                              //  新手7日活动结束自动通过邮件发送未领取奖励

    // 玩家任务信息操作
	DT_TASK_INFO* _Find(PlayerData* poPlayerData, uint32_t dwId);                               //  查找任务
	DT_TASK_INFO* _Add(PlayerData* poPlayerData, uint32_t dwId);                                          //  增加
	int _Del(PlayerData* poPlayerData, uint32_t dwId);                                          //  删除
	int _DelAll(PlayerData* poPlayerData);                                                      //  删除全部
	int _ModifyData(PlayerData* poPlayerData, RESTASK* pResTask, uint32_t dwValueChg);          //  修改任务完成度
	bool _CompFunc(uint32_t& dwPara1, uint32_t& dwPara2, uint8_t bType);                        //  任务是否完成
};

