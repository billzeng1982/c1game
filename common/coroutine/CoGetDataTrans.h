#pragma once

/*
	CoGetDataAction 为从其他服务器上异步获取数据
	
	CoGetDataTrans 包装 CoGetDataAction, 若有多个GetDataAction，则同时执行, 并且切换当前执行堆栈
*/

#include "object.h"
#include <vector>
#include <time.h>
#include "coroutine.h"
#include "../log/LogMacros.h"
#include <time.h>
#include <string.h>

using namespace std;

class CoDataFrame;

// 具体的异步GetDataAction需要继承本类使用
class CoGetDataAction : public IObject
{
public:
	CoGetDataAction() { this->Reset(); }
	virtual ~CoGetDataAction() {}
	
	/*
		0: 此动作还没执行或没执行结束（在等待异步执行结果）
		1: 此动作已经执行结束，且成功
		< 0: 此动作已经执行结束，且失败
	*/
	int  GetFiniFlag() { return m_iFiniFlag; }
	void SetFiniFlag( int iVal ) { m_iFiniFlag = iVal; }

	void SetToken( uint64_t ulToken ) { m_ulToken = ulToken; }
	uint64_t GetToken() { return m_ulToken; }

	void SetDataFrame(CoDataFrame* poDataFrame) { m_poDataFrame = poDataFrame; }
	
	virtual void Reset( )
	{
		m_iFiniFlag 	= 0;
		m_ulToken 	= 0;
		m_poDataFrame  = NULL;
	}
	
	// 执行异步获取数据
	virtual bool Execute( ) = 0;	

	//处理异步返回结果的数据,由框架调用
	void OnAsyncRspData( int iErrNo, void* pResult );

protected:
	// 数据返回后，检查已经
	bool _IsInMem( void* pResult );
	bool _SaveInMem( void* pResult );

protected:
	/*
		0: 	此动作还没执行或没执行结束（在等待异步执行结果）
		1:	此动作已经执行结束，且成功
		< 0: 此动作已经执行结束，且失败
	*/
	int  	m_iFiniFlag;

	uint64_t 	m_ulToken;
	CoDataFrame* m_poDataFrame;
};

// --------------------------------------------------------------------------------------------------------

// 包装一个或多个 CoGetDataAction
class CoGetDataTrans
{
public:
	CoGetDataTrans()
	{
		this->Clear();
	}
	
	~CoGetDataTrans(){}

	void Clear()
	{
		m_vActions.clear();
		m_iCoroutineID = 0;
		bzero( &m_stTimeout, sizeof(m_stTimeout) );
		m_iFiniFlag = 0;
		m_dwTransID = 0;
	}

	void AddAction( CoGetDataAction* poAction ) { m_vActions.push_back( poAction ); }
	//void AddAction( vector<CoGetDataAction*>& rvActions );	

	struct timeval* GetTimeout() { return &m_stTimeout; }
	void SetTimeout( struct timeval& rstTimeout ) { m_stTimeout = rstTimeout; }
	
	bool Execute( CoDataFrame* poDataFrame );
	
	int GetFiniFlag();
	void SetFiniFlag( int iFiniFlag ) { m_iFiniFlag = iFiniFlag; }

	CoGetDataAction* GetAction( int index );
	int GetActionCount() { return (int)m_vActions.size(); }
	
	void SetTransID(uint32_t dwTransID) { m_dwTransID = dwTransID; }
	uint32_t GetTransID() { return m_dwTransID; }

	void SetCoroutineID( int iCoroutineID ) { m_iCoroutineID = iCoroutineID; }
	int GetCoroutineID() { return m_iCoroutineID; }

private:
	int m_iCoroutineID;
	vector<CoGetDataAction*> m_vActions;
	struct timeval m_stTimeout;
	uint32_t m_dwTransID;

	/*
		0: 此事务还没执行或没执行结束
		1: 此事务已经执行结束，且成功
		< 0: 此事务已经执行结束，且失败
	*/
	int m_iFiniFlag;
};


