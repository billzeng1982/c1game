#pragma once

/*
	主要用于数据获取和缓存的一个框架，对框架外的逻辑提供同步编程视角
	如果数据在内存中缓存，则从内存中直接返回，否者异步获取数据，挂起(yield)当前处理逻辑，直到数据返回或超时，再resume
	注意整个数据处理逻辑需要放到协程中
	好处是数据获取和逻辑流程分离，同一个数据相关功能可以分工编写，提高生产效率和代码可读性

	CoDataFrame需要继承使用
*/
#include "CoGetDataTransMgr.h"
#include "IndexedPriorityQ.h"
#include "coroutine.h"
#include "og_comm.h"
#include <vector>
using namespace std;

template<> 
class Comparer<CoGetDataTrans*>
{
public:
    int Compare( CoGetDataTrans* x, CoGetDataTrans* y ) 
    {
    		struct timeval* xTimeout = x->GetTimeout();
		struct timeval* yTimeout = y->GetTimeout();	
        	if( TvBefore( xTimeout, yTimeout ) )
        	{
            	return -1;
        	}
        	if( xTimeout->tv_sec  == yTimeout->tv_sec &&
		    xTimeout->tv_usec == yTimeout->tv_usec )
        	{
            	return 0;
        	}
        	return 1;
    }
};

template<> 
class GetItemKey<CoGetDataTrans*, uint32_t>
{
public: 
    uint32_t GetKey( CoGetDataTrans* elem ) 
    { 
        return elem->GetTransID();
    }
};
	

class CoDataFrame
{
	const static int UPT_TIMEOUT_Q_PER_LOOP = 20;
	const static int DEFAULT_TIMEOUT_TIME_MS = 10000; // 10s
		
public:
	// errno define
	const static int ERRNO_SUCCESS = 0;
	const static int ERRNO_TIMEOUT = -1;
	const static int ERRNO_GETDATAFAILED = -3;
	const static int ERRNO_SAVE_IN_MEM_FAILED = -4;
	const static int ERRNO_GET_DATA_FAILED = -5;

public:
	static uint64_t MakeActionToken( uint32_t dwTransID, int iActionIdx );
	static void ParseActionToken(const uint64_t ulToken, uint32_t& dwTransID, int& iActionIdx );
	
public:
	CoDataFrame();
	virtual ~CoDataFrame(){}

	virtual bool IsInMem( void* pResult ) { return false; }
	virtual bool SaveInMem( void* pResult ) = 0;	
	bool BaseInit( int iMaxTrans );	

	void SetCoroutineEnv( CoroutineEnv* poCoroutineEnv ) { m_poCoroutineEnv = poCoroutineEnv; }
	void Update();

	/*  	异步get data action完成，由外部逻辑调用, 数据获取成功，切换至子协程
		pvResult为异步取得的数据, 可能为空
	*/
	void AsyncGetDataDone( uint64_t ulToken, int iErrNo, void* pvResult );

	// 子协程逻辑调用
	void* GetData( void* key);

    // 子协程逻辑调用, 返回取数据的实际错误码
    int GetData(void* key, OUT void*& rpData);

	// 子协程逻辑调用, 一次需要获取多个数据的情况
	int GetData( vector<void*>& rvKey, INOUT vector<void*>& rvOutData );

	void DebugCheck() { m_poCoroutineEnv->DebugCheck();}

protected:
	// 从内存缓存中获取数据
	virtual void* _GetDataInMem( void* key ) = 0;

	virtual CoGetDataAction* _CreateGetDataAction( void* key ) = 0;
	virtual void _ReleaseGetDataAction( CoGetDataAction* poAction ) = 0;
	
	void _ReleaseGetDataTrans( CoGetDataTrans* poTrans );

	// 发起异步获取数据，并切换至主协程
	bool _AsyncGetData( CoGetDataTrans* poTrans );
	
protected:
	CoGetDataTransMgr m_oTransMgr;
	IndexedPriorityQ< CoGetDataTrans*, uint32_t >* m_pTransTimeoutQ;
	CoroutineEnv* m_poCoroutineEnv;
};


