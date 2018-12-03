#pragma once

#include "singleton.h"
#include "common_proto.h"
#include <map>
#include "DynMempool.h"

using namespace PKGMETA;
using namespace std;

class AsyncPvpRecordMgr : public TSingleton<AsyncPvpRecordMgr>
{
private:
    static const int RECORD_INIT_NUM = 2000;
    static const int RECORD_DELTA_NUM = 500;
    static const int RECORD_MAX_NUM = 100000;

    struct RecordNode
    {
        int8_t cRefCount_;
        DT_ASYNC_PVP_FIGHT_RECORD* pstRecord_;
    };

    typedef map<uint64_t, RecordNode> RecordMap_t;

public:
    bool Init();

    void Update();

    int AddRecord(DT_ASYNC_PVP_FIGHT_RECORD& rstRecord);

    int GetRecord(DT_ASYNC_PVP_FIGHT_RECORD& rstRecord);

    int DelRecord(uint64_t ullRecordNo);

private:
    RecordMap_t::iterator m_oIter;
    RecordMap_t m_oRecordMap;
    DynMempool<DT_ASYNC_PVP_FIGHT_RECORD> m_oRecordPool;
};