#pragma once
#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include <list>

using namespace std;

class League : public TSingleton<League>
{
public:
    League(){};
    virtual ~League(){};

    bool Init();
    void UpdateServer();
    void InitPlayerData(PlayerData* pstData);
    int CheckMatch(PlayerData* pstData);
    int WeekLeagueApply(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo);
    int WeekLeagueRecvReward(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo);
    bool IsReward(PlayerData* pstData , const DT_ROLE_WEEK_LEAGUE_INFO& m_stWeekLeagueInfo);
private:
    uint64_t m_ullTimestamp;
    bool m_bIsStart;//是否开赛

    uint32_t m_dwLastTime;//每次联赛的持续时间

    list<uint64_t> m_TimeList;//开始时间链表

    uint8_t m_bWInTimesLimit;//胜利次数上限
    uint8_t m_bLoseTimesLimit;//失败次数上限

    uint32_t m_dwTicketId; //门票ID
    uint8_t m_dwTicketConsume;//门票一次消耗的量
};
