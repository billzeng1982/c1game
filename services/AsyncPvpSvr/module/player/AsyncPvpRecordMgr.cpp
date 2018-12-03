#include "AsyncPvpRecordMgr.h"
#include "LogMacros.h"

using namespace PKGMETA;

bool AsyncPvpRecordMgr::Init()
{
    if (!m_oRecordPool.Init(RECORD_INIT_NUM, RECORD_DELTA_NUM, RECORD_MAX_NUM))
    {
        LOGERR_r("AsyncPvpRecordMgr init false");
        return false;
    }

    return true;
}


void AsyncPvpRecordMgr::Update()
{
}


int AsyncPvpRecordMgr::AddRecord(DT_ASYNC_PVP_FIGHT_RECORD& rstRecord)
{
    m_oIter = m_oRecordMap.find(rstRecord.m_ullRecordNo);
    if (m_oIter == m_oRecordMap.end())
    {
        DT_ASYNC_PVP_FIGHT_RECORD* pstRecord = m_oRecordPool.Get();
        if (!pstRecord)
        {
            LOGERR_r("Get pstRecord from m_oRecordPoo failed");
            return ERR_SYS;
        }

        *pstRecord = rstRecord;

        RecordNode stNode;
        stNode.cRefCount_ = 1;
        stNode.pstRecord_ = pstRecord;

        m_oRecordMap.insert(RecordMap_t::value_type(rstRecord.m_ullRecordNo, stNode));
    }
    else
    {
        (m_oIter->second.cRefCount_)++;
    }

    return ERR_NONE;
}


int AsyncPvpRecordMgr::GetRecord(DT_ASYNC_PVP_FIGHT_RECORD& rstRecord)
{
    m_oIter = m_oRecordMap.find(rstRecord.m_ullRecordNo);
    if (m_oIter == m_oRecordMap.end())
    {
        return ERR_NOT_FOUND;
    }
    else
    {
        memcpy(&rstRecord, m_oIter->second.pstRecord_, sizeof(DT_ASYNC_PVP_FIGHT_RECORD));
    }

    return ERR_NONE;
}


int AsyncPvpRecordMgr::DelRecord(uint64_t ullRecordNo)
{
    m_oIter = m_oRecordMap.find(ullRecordNo);
    if (m_oIter == m_oRecordMap.end())
    {
        return ERR_NOT_FOUND;
    }
    else
    {
        RecordNode& rstNode = m_oIter->second;
        rstNode.cRefCount_--;
        if (rstNode.cRefCount_ <= 0)
        {
            m_oRecordPool.Release(rstNode.pstRecord_);
            m_oRecordMap.erase(m_oIter);
        }
    }
    return ERR_NONE;
}

