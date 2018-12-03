#include "DBWorkThreadMgr.h"
#include "../MessageMgr.h"
#define THREAD_INDEX(UIN) (((UIN) & 0xff) % m_iWorkThreadNum)


bool DBWorkThreadMgr::Init(MESSAGESVRCFG*	m_pstConfig)
{
	//初始化工作线程
	m_iWorkThreadNum = m_pstConfig->m_iWorkThreadNum;
	m_astWorkThreads = new CDBWorkThread[m_iWorkThreadNum];
	if( !m_astWorkThreads )
	{
		LOGERR_r("Init DBWorkThread failed.");
		return false;
	}
	key_t iShmKey = m_pstConfig->m_iThreadQBaseShmKey;
	bool bRet = 0;
	for( int i = 0; i < m_iWorkThreadNum; i++ )
	{
		bRet = m_astWorkThreads[i].InitThread(i, m_pstConfig->m_dwThreadQSize, THREAD_QUEUE_DUPLEX, (void*)m_pstConfig, &iShmKey);
		if( !bRet )
		{
			LOGERR_r("Init DBWorkThread <%d> failed", i );
			return false;
		}
	}
	return true;
}

//发送请求
void DBWorkThreadMgr::SendReq(DT_MESSAGE_DB_REQ& rstDBReq)
{
	int iIndex = THREAD_INDEX(rstDBReq.m_stWholeData.m_stBaseInfo.m_ullUin);;
	m_astWorkThreads[iIndex].SendReq(rstDBReq);
}		

void DBWorkThreadMgr::HandleDBThreadRsp()
{
	for (int i=0; i< m_iWorkThreadNum; i++)
	{
		CDBWorkThread& rstWorkThread = m_astWorkThreads[i];
		int iRecvBytes = rstWorkThread.Recv(CDBWorkThread::MAIN_THREAD);
		if (iRecvBytes < 0)
		{
			LOGERR_r("Main Thread Recv Failed, iRecvBytes=%d", iRecvBytes);
			continue;
		}
		else if (0 == iRecvBytes)
		{
			continue;
		}
		MyTdrBuf* pstRecvBuf = rstWorkThread.GetRecvBuf(CThreadFrame::MAIN_THREAD);
		TdrError::ErrorType iRet = m_stDBRsp.unpack(pstRecvBuf->m_szTdrBuf, pstRecvBuf->m_uPackLen);
		if (iRet != TdrError::TDR_NO_ERROR)
		{
			LOGERR_r("unpack pkg failed! errno : %d", iRet);
			continue;
		}
		MessageMgr::Instance().HandleDBThreadRsp(m_stDBRsp);
	}
}

void DBWorkThreadMgr::Fini()
{
    for (int i = 0; i < m_iWorkThreadNum; i++)
    {
        m_astWorkThreads[i].FiniThread();
    }
}


