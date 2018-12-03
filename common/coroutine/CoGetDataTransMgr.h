#pragma once

/*
	管理 CoGetDataTrans对象
*/

#include "CoGetDataTrans.h"
#include <map>
#include <vector>
#include "../mng/mempool.h"

using namespace std;

class CoGetDataTransMgr
{
	typedef map< uint32_t, CoGetDataTrans* > Id2TransMap_t;
	const static int DEFAULT_MAX_TRANS_NUM = 1000;	
	
public:
	CoGetDataTransMgr()
	{
		m_dwTransIDSeq = 0;
		m_oId2TransMap.clear();
	}
	~CoGetDataTransMgr()
	{
		m_oId2TransMap.clear();
	}

	bool Init( int iMaxTrans );

	CoGetDataTrans* New();
	void Release( CoGetDataTrans* poTrans );
	CoGetDataTrans* Find( uint32_t dwTransID );

private:
	uint32_t _GetNextTransID();

private:
	CMemPool< CoGetDataTrans > m_oTransPool;
	Id2TransMap_t	m_oId2TransMap;

	uint32_t m_dwTransIDSeq;	
};

