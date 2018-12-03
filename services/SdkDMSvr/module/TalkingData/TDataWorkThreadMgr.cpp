#include "TDataWorkThreadMgr.h"


bool TDataWorkThreadMgr::Init(SDKDMSVRCFG* pstConfig)
{
    if (pstConfig->m_iWorkThreadNum <= 0)
    {
        assert(false);
        return false;
    }
    TDataClient::InitGloabal();
    //初始化工作线程
    m_iWorkThreadNum = pstConfig->m_iWorkThreadNum;
    m_iWorkThreadIter = 0;
    m_astWorkThreads = new TDataWorkThread[m_iWorkThreadNum];
    if( !m_astWorkThreads )
    {
        LOGERR_r("Init TDataWorkThread failed.");
        return false;
    }
    key_t iShmKey = pstConfig->m_iThreadQBaseShmKey;
    bool bRet = 0;
    for( int i = 0; i < m_iWorkThreadNum; i++ )
    {
        bRet = m_astWorkThreads[i].InitThread(i, pstConfig->m_dwThreadQSize, THREAD_QUEUE_DUPLEX, (void*)pstConfig, &iShmKey);
        if( !bRet )
        {
            LOGERR_r("Init TDataWorkThread <%d> failed", i );
            return false;
        }
    }
    return true;
}

void TDataWorkThreadMgr::SendReq(DT_TDATA_ODER_INFO& rstReq)
{
    m_astWorkThreads[m_iWorkThreadIter].SendReq(rstReq);
    m_iWorkThreadIter = (m_iWorkThreadIter + 1) % m_iWorkThreadNum;
}

void TDataWorkThreadMgr::Fini()
{
    TDataClient::ReleaseGlobal();
}



