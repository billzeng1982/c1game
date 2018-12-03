#pragma once

/*
	�߼���Э��ϵͳ����hookϵͳ����
	��Э���߼����ܵ���yield
	Э��ջ���Ϊ128K !!
*/
#include <ucontext.h>
#include <string.h>
#include <map>
#include "../mng/mempool.h"
using namespace std;

#define DEFAULT_STACK_SZIE (1024*128) // Э��ջ��ȣ�Э�̺���������õ��Ӻ�����Ҫ��������ջ����(��SCPKG, SSPKG)������ջ�����崻�

enum CoroutineState
{
	CO_FREE, CO_RUNNABLE, CO_RUNNING, CO_SUSPEND
};

typedef void (*pfn_coroutine_t)( void* );

class CoroutineEnv; 

struct CoResumeEvent_t
{
	int m_iEventID;
	void* m_pEventArg; // �¼�����
};

struct Coroutine_t
{
	int				m_iID; // coroutine id
	CoroutineEnv* 		m_pEnv;
	pfn_coroutine_t	m_pfn; // Э��ִ�к���
	void*			m_pArg; //����
	enum CoroutineState m_iState;
	bool				m_bIsMain;			
	CoResumeEvent_t*	m_pResumeEvent;
	ucontext_t 		m_ctx; // ����Э�̵�context
	char 			m_szRunStack[ DEFAULT_STACK_SZIE ]; // Э������ʱջ, 128K

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


/* Э������ʱ����, ����Э��
   �ǶԳ�Э�� - ����Ƕ�״�����Э��.(�����õ�)
   Ϊ�˼�¼����Ƕ�״�����Э��, �Ա���Э���˳�ʱ��ȷ�ָ��������(�����λ�ڸ�Э����), ��Ҫ��¼����Ƕ�׵��ù���;
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

	// ����Э�̣�����Э��ID�������� <0
	int CoCreate( pfn_coroutine_t pf, void* arg );

	// ������ִ��Э��
	int StartCoroutine( pfn_coroutine_t pf, void* arg );

	// �ⲿ�¼�����resume, �¼������߼��¼�(�����ݵ���)���߼���ʱ�¼���. pSpecΪ�ⲿ��������
	void CoResume( int iCoID, CoResumeEvent_t* pResumeEvent = NULL );

	// �ж��Ƿ�����Э�̶�ִ����
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

	// ����Э��
	Coroutine_t* _CoCreate( pfn_coroutine_t pf, void* arg );


	Coroutine_t* _CoFind( int iCoID );
	void _CoDelIndexing( int iCoID );

private:
	Coroutine_t* m_pCallStack[ MAX_CALL_STACK_SIZE ]; //���߳�������Ƕ�״���128��Э��(��Э��1�ڴ���Э��2, Э��2�ڴ���Э��3... Э��127�ڴ���Э��128)
	int m_iCallStackLen;
	int m_iCoSeq;

	Id2CoMap_t m_oId2CoMap;
	CMemPool<Coroutine_t> m_oCoPool; // coroutine �ڴ��
};

