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
    bool m_bIsStart;//�Ƿ���

    uint32_t m_dwLastTime;//ÿ�������ĳ���ʱ��

    list<uint64_t> m_TimeList;//��ʼʱ������

    uint8_t m_bWInTimesLimit;//ʤ����������
    uint8_t m_bLoseTimesLimit;//ʧ�ܴ�������

    uint32_t m_dwTicketId; //��ƱID
    uint8_t m_dwTicketConsume;//��Ʊһ�����ĵ���
};
