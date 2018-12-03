#pragma once

#include "Guild.h"
#include "singleton.h"
#include "ss_proto.h"
#include <list>

using namespace PKGMETA;
using namespace std;

class GuildFightMgr : public TSingleton<GuildFightMgr>
{
public:
    GuildFightMgr() {}
    ~GuildFightMgr() {}

    bool Init(int iUptTimeVal, const char* pszFileName);

    void Fini();

    void Update();

    void Reset();

    void GetState(DT_GUILD_FIGHT_STATE_INFO& rstStateInfo)
    {
        rstStateInfo.m_bState = m_iState;
        rstStateInfo.m_ullTimeStamp = m_ullChgStateTime/1000; //客户端使用的单位是秒，服务器是毫秒，需要除1000
    }

    //状态相关函数，状态机使用
    int GetState() { return m_iState; }
    void SetState(int iNewState) { m_iState = iNewState; }

    //军团战报名
    int FightApply(Guild* poGuild, int iFund);

    //获取报名资金
    uint32_t GetFightApplyFund(uint64_t ullGuildId);

    //报名结束后，生成初次对阵表
    void GenerateAgainstList();

    //报名表变化同步到ZoneSvr
    void SendApplyListSyn();

    //对阵表变化同步到ZoneSvr
    void SendAgainstListSyn();

    //报名结束后的处理
    void OnApplyEnd();

	//每个阶段战斗打完后的奖励结算，邮件发送，在arena的settle阶段调用
	void FightSettle(DT_GUILD_FIGHT_ARENA_GUILD_INFO* stGuildInfoList, uint64_t ullWinnerId);

    //将进度保存到文件中
    bool _SaveSchedule();

private:
    //有人报名后，刷新报名表
    void RefreshApplyList(Guild* poGuild);

    //发送奖励
    void _SendReward(uint64_t ullGuildId, uint64_t ullWinnerId, bool bThird=false);

    //第一次初始化，开服时的初始化
    bool _FirstInit();

    //从文件初始化
    bool _InitFromFile();

public:
    //开始时间链表
    //list<uint64_t> m_TimeList;

    //报名开始时间
    uint64_t m_ullApplyStartTime;

    //报名持续时间
    uint64_t m_ullApplyTime;

    //报名结束到战斗开始的持续时间
    uint64_t m_ullApplyRestTime;

    // 首次休战标志
    bool m_bFirstRest;

    //战斗结束到下次战斗开始的持续时间（首次）
    uint64_t m_ullFightRestTime;

    //战斗结束到下次战斗开始的持续时间（后续）
    uint64_t m_ullFightRestTimeFollow;

    //战斗准备时间
    uint64_t m_ullFightPrepareTime;

    //一场战斗的持续时间
    uint64_t m_ullFightTime;

    //当前状态
    int m_iState;

    //下次切换状态的时间，供客户端显示使用，单位MS
    uint64_t m_ullChgStateTime;

    //当前的比赛阶段(可能的值为8、4、2,表示是8强赛,4强赛,决赛)
    int m_iFightStage;

    //报名表
    DT_GUILD_FIGHT_APPLY_LIST_INFO  m_stFightApplyList;

    //对阵表
    DT_GUILD_FIGHT_AGAINST_INFO   m_stFightAgainstList;

    //update的时间间隔
    int m_iUptTimeVal;

    uint64_t m_ullTimeMs;

    SSPkg m_stSsPkg;

    FILE* m_fp;

    //存储的文件名
    char m_szFileName[MAX_NAME_LENGTH];
};