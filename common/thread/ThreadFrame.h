#ifndef _THREAD_FRAME_H_
#define _THREAD_FRAME_H_

/*
	多线程框架, 主线程和从线程通过无锁的ThreadQueue进行通信(ss消息)
	多线程里面的本地log，注意使用带"_r"的版本
	主线程: write ReqQueue, read RspQueue
	从线程: write RspQueue, read ReqQueue
*/

#include "ThreadQueue.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"

/*工作线程状态*/
typedef enum
{
	THREAD_STATUS_NULL			= 	0x0000,
    THREAD_STATUS_RUNNING    	=  	0x0001,   /*运行中*/
    THREAD_STATUS_STOPPING      = 	0x0002,   /*需要退出*/
} THREAD_STATUS;

#define THREAD_QUEUE_MODE_REQ 1 	// 单工, 仅主线程向从线程写数据
#define THREAD_QUEUE_MODE_RSP 2  	// 单工, 仅从线程向主线程写数据
#define THREAD_QUEUE_DUPLEX (THREAD_QUEUE_MODE_REQ | THREAD_QUEUE_MODE_RSP)	// 双工

class CThreadFrame
{
	friend void* WorkThreadFunc( void* pvArg );

public:
	const static int WORK_THREAD = 1;
	const static int MAIN_THREAD = 2;

public:
	CThreadFrame();
	virtual ~CThreadFrame(){}

	// 主线程调用
	bool InitThread( int iThreadIdx, uint32_t dwThreadQSzie, int iWorkMode, void* pvAppData, key_t* piShmKey = NULL );
	
	// 主线程调用 
	void FiniThread(); 

	CThreadQueue* GetReqQueue() { return (m_iWorkMode & THREAD_QUEUE_MODE_REQ) ? &m_oReqQueue : NULL; }
	CThreadQueue* GetRspQueue() { return (m_iWorkMode & THREAD_QUEUE_MODE_RSP) ? &m_oRspQueue : NULL; }

	/*
		Recv:
		work thread: req queue
		main thread: rsp queue
		返回收到包的字节数
		0 - no msg
		<0 - error
	*/
	int Recv( int iThreadType );
	MyTdrBuf* GetRecvBuf( int iThreadType );
protected:
	virtual bool _ThreadAppInit( void* pvAppData ) { return true; }
	virtual void _ThreadAppFini() {}
	
	// 线程逻辑处理, 返回处理请求包个数
	virtual int _ThreadAppProc() = 0;

	void _Idle();

protected:
	CThreadQueue 	m_oReqQueue;	// work thread read
	CThreadQueue 	m_oRspQueue;	// work thread write	
	int 			m_iThreadIdx;	// 编号
	pthread_t       m_iThreadId;    // Thread Id (tid)
	int        		m_iThreadStatus; //线程状态

	MyTdrBuf 		m_stReqQPeekBuf; // work thread peek
	MyTdrBuf		m_stRspQPeekBuf; // main thread peek
	
	//PKGMETA::SSPKG 	m_stSsRecvPkg;  // 收到的ss pkg请求
	int				m_iWorkMode;
};


/*工作线程入口函数*/
void* WorkThreadFunc( void* pvArg ); 

#endif

