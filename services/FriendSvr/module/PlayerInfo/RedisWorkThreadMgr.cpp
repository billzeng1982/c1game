#include "RedisWorkThreadMgr.h"
#include "../FriendMgr.h"

#define THREAD_INDEX(UIN) (((UIN) & 0xff) % m_iWorkThreadNum)

bool RedisWorkThreadMgr::Init(FRIENDSVRCFG*	pstConfig)
{
	//初始化工作线程
	m_iWorkThreadNum = pstConfig->m_iRedisWorkThreadNum;
	m_astWorkThreads = new RedisWorkThread[m_iWorkThreadNum];
	if( !m_astWorkThreads )
	{
		LOGERR_r("Init RedisWorkThread failed.");
		return false;
	}
	key_t iShmKey = pstConfig->m_iRedisThreadQBaseShmKey;
	bool bRet = 0;
	for( int i = 0; i < m_iWorkThreadNum; i++ )
	{
		bRet = m_astWorkThreads[i].InitThread(i, pstConfig->m_dwRedisThreadQSize, THREAD_QUEUE_DUPLEX, (void*)pstConfig, &iShmKey);
		if( !bRet )
		{
			LOGERR_r("Init RedisWorkThread <%d> failed", i );
			return false;
		}
	}
	return true;

}


void RedisWorkThreadMgr::Fini()
{
    for (int i = 0; i < m_iWorkThreadNum; i++)
    {
        m_astWorkThreads[i].FiniThread();
    }
}

void RedisWorkThreadMgr::SendReq(DT_FRIEND_PLAYERINFO_REDIS_REQ& rstReq)
{
	m_astWorkThreads[ THREAD_INDEX(rstReq.m_stPlayerInfo.m_ullUin) ].SendReq(rstReq);
}

//	处理每个线程的回复队列
void RedisWorkThreadMgr::HandleRspMsg()
{
	for( int i=0; i < m_iWorkThreadNum; i++ )
	{
		RedisWorkThread& rstWorkThread = m_astWorkThreads[i];
		int iRecvBytes = rstWorkThread.Recv(RedisWorkThread::MAIN_THREAD);
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
		bzero(&m_stRsp, sizeof(DT_FRIEND_PLAYERINFO_REDIS_RSP));
		TdrError::ErrorType iRet = m_stRsp.unpack(pstRecvBuf->m_szTdrBuf, pstRecvBuf->m_uPackLen);
		if (iRet != TdrError::TDR_NO_ERROR)
		{
			LOGERR_r("unpack pkg failed! errno : %d", iRet);
			continue;
		}
		FriendMgr::Instance().HandleRedisThreadRsp(m_stRsp);
	}
}
