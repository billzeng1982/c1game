#include <pthread.h>
#include <string.h>
#include <errno.h>
#include "ThreadFrame.h"
#include "LogMacros.h"
#include "oi_misc.h"


CThreadFrame::CThreadFrame() : m_stReqQPeekBuf(), m_stRspQPeekBuf()
{
	m_iThreadIdx    = 0;
	m_iThreadId     = 0;
	m_iThreadStatus = 0;
	m_iWorkMode     = 0;
}


// 主线程调用
bool CThreadFrame::InitThread( int iThreadIdx, uint32_t dwThreadQSzie, int iWorkMode, void* pvAppData, key_t* piShmKey )
{
	assert( iThreadIdx >= 0 && dwThreadQSzie > 0 && iWorkMode > 0 );

	m_iThreadIdx = iThreadIdx;
	m_iWorkMode = iWorkMode;
	int iShmKey = piShmKey ? *piShmKey : 0;

	if( THREAD_QUEUE_MODE_REQ & m_iWorkMode)
	{
		if( !m_oReqQueue.Init( dwThreadQSzie, iShmKey ) )
		{
			LOGERR_r( "Init req queue failed!,  workthread [%d], shmkey [%d]", m_iThreadIdx, iShmKey );
			return false;
		}
		if( iShmKey != 0 ) ++iShmKey;
	}

	if( THREAD_QUEUE_MODE_RSP & m_iWorkMode )
	{
		if( !m_oRspQueue.Init( dwThreadQSzie, iShmKey ) )
		{
			LOGERR_r( "Init req queue failed!, shmkey <%d>", iShmKey );
			return false;
		}
		if( iShmKey != 0 ) ++iShmKey;
	}

	if( piShmKey ) *piShmKey = iShmKey;

	if( !this->_ThreadAppInit( pvAppData ) )
	{
		return false;
	}

	// 启动线程
	int iRet = pthread_create(&(m_iThreadId), NULL, WorkThreadFunc, (void *)this);
	if( iRet != 0 )
	{
		LOGERR_r( "Error:fail to start workthread [%d]!errorstring=[%s]", m_iThreadIdx, strerror(errno) );
		return false;
	}

	LOGRUN_r("Workthread start success! [idx=%d]", m_iThreadIdx);

	return true;
}


// 主线程调用
void CThreadFrame::FiniThread()
{
    this->_ThreadAppFini();

	void* pRet;

	if(m_iThreadStatus == THREAD_STATUS_RUNNING)
	{
		m_iThreadStatus = THREAD_STATUS_STOPPING;

		//等待线程退出
		pthread_join(m_iThreadId, &(pRet));
	}

	// work线程退出后...
	m_oReqQueue.Fini();
	m_oRspQueue.Fini();

	return ;
}


void CThreadFrame::_Idle()
{
	MsSleep(1);
}


/*
	Recv:
	work thread: req queue
	main thread: rsp queue
	返回收到包的字节数
	0 - no msg
	<0 - error
*/
int CThreadFrame::Recv( int iThreadType )
{
	int iRet = 0;
	MyTdrBuf* pstRecvBuf = NULL;
	CThreadQueue* poThreadQ = NULL;

	if( WORK_THREAD == iThreadType )
	{
		pstRecvBuf = &m_stReqQPeekBuf;
		poThreadQ  = &m_oReqQueue;
	}else if( MAIN_THREAD == iThreadType )
	{
		pstRecvBuf = &m_stRspQPeekBuf;
		poThreadQ  = &m_oRspQueue;
	}else
	{
		assert(false);
		return -1;
	}

	// delete pre msg if exists
	if( pstRecvBuf->m_szTdrBuf )
	{
		iRet = poThreadQ->PopMsg( );
		if ( THREAD_Q_SUCESS != iRet)
		{
			LOGERR_r( "thread queue delete msg error, [errno:%d]", iRet );
			return  -1;
		}

		// clean up
		pstRecvBuf->Reset();
	}

	// peek msg
	uint32_t dwPkgLen = 0;
	iRet = poThreadQ->PeekMsg( &(pstRecvBuf->m_szTdrBuf), &dwPkgLen );
	pstRecvBuf->m_uPackLen = dwPkgLen;
	if (THREAD_Q_SUCESS != iRet)
	{
		if ( THREAD_Q_EMPTY == iRet )
		{
			// no msg in channel
			return  0;
		}

		LOGERR_r( "thread queue peek msg error, [errno:%d]", iRet );
		return  -1;
	}

	return  (int)pstRecvBuf->m_uPackLen;
}


MyTdrBuf* CThreadFrame::GetRecvBuf( int iThreadType )
{
	if( WORK_THREAD == iThreadType )
	{
		return &m_stReqQPeekBuf;
	}else if( MAIN_THREAD == iThreadType )
	{
		return &m_stRspQPeekBuf;
	}else
	{
		assert(false);
		return NULL;
	}
}


/*工作线程入口函数*/
void* WorkThreadFunc( void* pvArg )
{
	assert( pvArg );

	CThreadFrame* poWorkThread =(CThreadFrame*)pvArg;

	poWorkThread->m_iThreadStatus = THREAD_STATUS_RUNNING;

	while( poWorkThread->m_iThreadStatus != THREAD_STATUS_STOPPING )
	{
		if( poWorkThread->_ThreadAppProc() <= 0 )
		{
			poWorkThread->_Idle();
		}
	}

	pthread_exit((void *)0);
}

