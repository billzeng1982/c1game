#pragma once

/*
	CoGetDataAction Ϊ���������������첽��ȡ����
	
	CoGetDataTrans ��װ CoGetDataAction, ���ж��GetDataAction����ͬʱִ��, �����л���ǰִ�ж�ջ
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

// ������첽GetDataAction��Ҫ�̳б���ʹ��
class CoGetDataAction : public IObject
{
public:
	CoGetDataAction() { this->Reset(); }
	virtual ~CoGetDataAction() {}
	
	/*
		0: �˶�����ûִ�л�ûִ�н������ڵȴ��첽ִ�н����
		1: �˶����Ѿ�ִ�н������ҳɹ�
		< 0: �˶����Ѿ�ִ�н�������ʧ��
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
	
	// ִ���첽��ȡ����
	virtual bool Execute( ) = 0;	

	//�����첽���ؽ��������,�ɿ�ܵ���
	void OnAsyncRspData( int iErrNo, void* pResult );

protected:
	// ���ݷ��غ󣬼���Ѿ�
	bool _IsInMem( void* pResult );
	bool _SaveInMem( void* pResult );

protected:
	/*
		0: 	�˶�����ûִ�л�ûִ�н������ڵȴ��첽ִ�н����
		1:	�˶����Ѿ�ִ�н������ҳɹ�
		< 0: �˶����Ѿ�ִ�н�������ʧ��
	*/
	int  	m_iFiniFlag;

	uint64_t 	m_ulToken;
	CoDataFrame* m_poDataFrame;
};

// --------------------------------------------------------------------------------------------------------

// ��װһ������ CoGetDataAction
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
		0: ������ûִ�л�ûִ�н���
		1: �������Ѿ�ִ�н������ҳɹ�
		< 0: �������Ѿ�ִ�н�������ʧ��
	*/
	int m_iFiniFlag;
};


