#pragma once

/*
	逻辑层协程系统，不hook系统函数
	主协程逻辑不能调用yield
	协程栈深度为128K !!
*/
#include <ucontext.h>
#include <string.h>
#include <map>
#include "../mng/mempool.h"
using namespace std;

#define DEFAULT_STACK_SZIE (1024*128) // 协程栈深度，协程函数及其调用的子函数不要定义过大的栈变量(如SCPKG, SSPKG)，否则栈溢出，宕机

enum CoroutineState
{
	CO_FREE, CO_RUNNABLE, CO_RUNNING, CO_SUSPEND
};

typedef void (*pfn_coroutine_t)( void* );

class CoroutineEnv; 

struct CoResumeEvent_t
{
	int m_iEventID;
	void* m_pEventArg; // 事件参数
};

struct Coroutine_t
{
	int				m_iID; // coroutine id
	CoroutineEnv* 		m_pEnv;
	pfn_coroutine_t	m_pfn; // 协程执行函数
	void*			m_pArg; //参数
	enum CoroutineState m_iState;
	bool				m_bIsMain;			
	CoResumeEvent_t*	m_pResumeEvent;
	ucontext_t 		m_ctx; // 保存协程的context
	char 			m_szRunStack[ DEFAULT_STACK_SZIE ]; // 协程运行时栈, 128K

	Coroutine_t()
	{
		this->Clear();
	}

	void Clear()
	{
		m_iID = 0;
		m_pEnv = NULL;
		m_pfn = NULL;
		m_pArg = NULL;
		m_iState= CO_FREE;
		m_bIsMain = false;
		m_pResumeEvent = NULL;
		
		bzero(&m_ctx, sizeof(m_ctx));
		m_szRunStack[0] = '\0';
	}
};


/* 协程运行时环境, 管理协程
   非对称协程 - 允许嵌套创建子协程.(极少用到)
   为了记录这种嵌套创建的协程, 以便子协程退出时正确恢复到挂起点(挂起点位于父协程中), 需要记录这种嵌套调用过程;
*/
class CoroutineEnv
{
	typedef map<int, Coroutine_t*> Id2CoMap_t;

	const static int MAX_CALL_STACK_SIZE = 128;
	const static int DEFAULT_COROUTINE_NUM = 512;

public:
	CoroutineEnv();
	~CoroutineEnv()
	{
		m_oId2CoMap.clear();
	}
	
	bool Init( int iMaxCoNum = 0 );
	
	void CoYield();

	// 创建协程，返回协程ID，出错返回 <0
	int CoCreate( pfn_coroutine_t pf, void* arg );

	// 创建并执行协程
	int StartCoroutine( pfn_coroutine_t pf, void* arg );

	// 外部事件触发resume, 事件包括逻辑事件(如数据到达)、逻辑超时事件等. pSpec为外部传入数据
	void CoResume( int iCoID, CoResumeEvent_t* pResumeEvent = NULL );

	// 判断是否所有协程都执行完
	bool CoFinished();

	int GetCurrCoID() { return  m_pCallStack[ m_iCallStackLen - 1]->m_iID; }
	CoResumeEvent_t* GetCurrResumeEvent() { return m_pCallStack[ m_iCallStackLen - 1]->m_pResumeEvent; }

	void DebugCheck()
	{
		for( int i = 0; i < m_iCallStackLen; i++ )
	        assert( m_pCallStack[i]->m_iID > 0);	
	}

private:
	void _CoResume( Coroutine_t* pCo );
	void _CoRelease( Coroutine_t* pCo );
	int  _GetNextCoID();

	// 创建协程
	Coroutine_t* _CoCreate( pfn_coroutine_t pf, void* arg );


	Coroutine_t* _CoFind( int iCoID );
	void _CoDelIndexing( int iCoID );

private:
	Coroutine_t* m_pCallStack[ MAX_CALL_STACK_SIZE ]; //该线程内允许嵌套创建128个协程(即协程1内创建协程2, 协程2内创建协程3... 协程127内创建协程128)
	int m_iCallStackLen;
	int m_iCoSeq;

	Id2CoMap_t m_oId2CoMap;
	CMemPool<Coroutine_t> m_oCoPool; // coroutine 内存池
};

