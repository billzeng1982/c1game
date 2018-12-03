#pragma once

/*
	��Ҫ�������ݻ�ȡ�ͻ����һ����ܣ��Կ������߼��ṩͬ������ӽ�
	����������ڴ��л��棬����ڴ���ֱ�ӷ��أ������첽��ȡ���ݣ�����(yield)��ǰ�����߼���ֱ�����ݷ��ػ�ʱ����resume
	ע���������ݴ����߼���Ҫ�ŵ�Э����
	�ô������ݻ�ȡ���߼����̷��룬ͬһ��������ع��ܿ��Էֹ���д���������Ч�ʺʹ���ɶ���

	CoDataFrame��Ҫ�̳�ʹ��
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

	/*  	�첽get data action��ɣ����ⲿ�߼�����, ���ݻ�ȡ�ɹ����л�����Э��
		pvResultΪ�첽ȡ�õ�����, ����Ϊ��
	*/
	void AsyncGetDataDone( uint64_t ulToken, int iErrNo, void* pvResult );

	// ��Э���߼�����
	void* GetData( void* key);

    // ��Э���߼�����, ����ȡ���ݵ�ʵ�ʴ�����
    int GetData(void* key, OUT void*& rpData);

	// ��Э���߼�����, һ����Ҫ��ȡ������ݵ����
	int GetData( vector<void*>& rvKey, INOUT vector<void*>& rvOutData );

	void DebugCheck() { m_poCoroutineEnv->DebugCheck();}

protected:
	// ���ڴ滺���л�ȡ����
	virtual void* _GetDataInMem( void* key ) = 0;

	virtual CoGetDataAction* _CreateGetDataAction( void* key ) = 0;
	virtual void _ReleaseGetDataAction( CoGetDataAction* poAction ) = 0;
	
	void _ReleaseGetDataTrans( CoGetDataTrans* poTrans );

	// �����첽��ȡ���ݣ����л�����Э��
	bool _AsyncGetData( CoGetDataTrans* poTrans );
	
protected:
	CoGetDataTransMgr m_oTransMgr;
	IndexedPriorityQ< CoGetDataTrans*, uint32_t >* m_pTransTimeoutQ;
	CoroutineEnv* m_poCoroutineEnv;
};


