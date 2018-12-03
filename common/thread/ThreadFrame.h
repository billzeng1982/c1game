#ifndef _THREAD_FRAME_H_
#define _THREAD_FRAME_H_

/*
	���߳̿��, ���̺߳ʹ��߳�ͨ��������ThreadQueue����ͨ��(ss��Ϣ)
	���߳�����ı���log��ע��ʹ�ô�"_r"�İ汾
	���߳�: write ReqQueue, read RspQueue
	���߳�: write RspQueue, read ReqQueue
*/

#include "ThreadQueue.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"

/*�����߳�״̬*/
typedef enum
{
	THREAD_STATUS_NULL			= 	0x0000,
    THREAD_STATUS_RUNNING    	=  	0x0001,   /*������*/
    THREAD_STATUS_STOPPING      = 	0x0002,   /*��Ҫ�˳�*/
} THREAD_STATUS;

#define THREAD_QUEUE_MODE_REQ 1 	// ����, �����߳�����߳�д����
#define THREAD_QUEUE_MODE_RSP 2  	// ����, �����߳������߳�д����
#define THREAD_QUEUE_DUPLEX (THREAD_QUEUE_MODE_REQ | THREAD_QUEUE_MODE_RSP)	// ˫��

class CThreadFrame
{
	friend void* WorkThreadFunc( void* pvArg );

public:
	const static int WORK_THREAD = 1;
	const static int MAIN_THREAD = 2;

public:
	CThreadFrame();
	virtual ~CThreadFrame(){}

	// ���̵߳���
	bool InitThread( int iThreadIdx, uint32_t dwThreadQSzie, int iWorkMode, void* pvAppData, key_t* piShmKey = NULL );
	
	// ���̵߳��� 
	void FiniThread(); 

	CThreadQueue* GetReqQueue() { return (m_iWorkMode & THREAD_QUEUE_MODE_REQ) ? &m_oReqQueue : NULL; }
	CThreadQueue* GetRspQueue() { return (m_iWorkMode & THREAD_QUEUE_MODE_RSP) ? &m_oRspQueue : NULL; }

	/*
		Recv:
		work thread: req queue
		main thread: rsp queue
		�����յ������ֽ���
		0 - no msg
		<0 - error
	*/
	int Recv( int iThreadType );
	MyTdrBuf* GetRecvBuf( int iThreadType );
protected:
	virtual bool _ThreadAppInit( void* pvAppData ) { return true; }
	virtual void _ThreadAppFini() {}
	
	// �߳��߼�����, ���ش������������
	virtual int _ThreadAppProc() = 0;

	void _Idle();

protected:
	CThreadQueue 	m_oReqQueue;	// work thread read
	CThreadQueue 	m_oRspQueue;	// work thread write	
	int 			m_iThreadIdx;	// ���
	pthread_t       m_iThreadId;    // Thread Id (tid)
	int        		m_iThreadStatus; //�߳�״̬

	MyTdrBuf 		m_stReqQPeekBuf; // work thread peek
	MyTdrBuf		m_stRspQPeekBuf; // main thread peek
	
	//PKGMETA::SSPKG 	m_stSsRecvPkg;  // �յ���ss pkg����
	int				m_iWorkMode;
};


/*�����߳���ں���*/
void* WorkThreadFunc( void* pvArg ); 

#endif

