#pragma once

#include <map>
#include "singleton.h"
#include "list_i.h"
#include "DynMempool.h"
#include "AsyncPvpPlayer.h"
#include "ss_proto.h"

using namespace std;

class AsyncPvpFightMgr : public TSingleton<AsyncPvpFightMgr>
{
public:
    static const uint64_t BASE_TIMESTAMP = 1480521600;
    static const uint8_t MAX_NUM_CREATE_ONE_SEC = 128;
    static const int DUNGEON_INIT_NUM = 100;
    static const int DUNGEON_DELTA_NUM = 10;
    static const int FIGHT_TIME = 180;

    struct Dungeon
    {
        uint64_t m_ullRecordNo;
        uint64_t m_ullTimestamp;
        AsyncPvpPlayer* poAttacker;
        AsyncPvpPlayer* poDefender;
        TLISTNODE m_stTimeListNode;
    };

    typedef map<uint64_t, Dungeon*> Id2DungeonMap_t;

public:
    bool Init();

    void Update();

    int Create(AsyncPvpPlayer* poAttacker, AsyncPvpPlayer* poDefender, uint64_t* pRecordNo);

    int Settle(uint64_t ullRecordNo, uint8_t bWinGroup, DT_ASYNC_PVP_FIGHT_RECORD& rstRecord);

private:
    uint64_t _GenRecordNo();

private:
    TLISTNODE m_stTimeListHead;

    uint64_t m_ullLastUptTimestamp;

    DynMempool<Dungeon> m_oDungeonPool;

    Id2DungeonMap_t::iterator m_oIter;
    Id2DungeonMap_t m_oDungeonMap;

    uint8_t m_bSeq;

    SSPKG m_stSsPkg;
};
