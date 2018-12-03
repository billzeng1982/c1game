#pragma once

/*
	��������Դ�����,����Դ�����������,����̬��չ��Դ������
	ע��: 
	1. �������ڶ��Ϸ����
	2. ʹ��replacement new��ÿ��Get()��Ȼ����ù��캯��
	3. releaseʱ�������������
	4. ͬһ���ڴ���Data��next free pointer�临��
	billzeng, 2015.07.23
*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <new>
#include "log/LogMacros.h"

template<class T>
class DynMempool
{
public:

	struct PoolNode
	{
		PoolNode* m_next;
	};
	
	DynMempool()
	{
		m_stFreeList.m_next = NULL;
		m_iFreeNum = 0;
		m_iCapacity = 0;
		m_iInitNum = 0;
		m_iDelta = 0;
		m_iMaxCapacity = 0;
	}

	~DynMempool()
	{	
	}

	int GetFreeNum() { return m_iFreeNum; }

	int GetCapacity() { return m_iCapacity; }

	int GetMaxCapacity() { return m_iMaxCapacity; }

	//�ܷ��ȡ�µĽڵ�
	bool CanGetNewNode() { return m_iFreeNum > 0 || m_iCapacity < m_iMaxCapacity; }

	bool Init( int iInitNum, int iDelta, int iMaxCapacity = 0 );

	T* Get();
	void Release( T*& pElem );

private:
	bool _Alloc( int iNum );

private:
	PoolNode  m_stFreeList;
	int m_iFreeNum;
	int m_iCapacity;
	int m_iInitNum;  // ��ʼ����
	int m_iDelta; // > 0, ���нڵ㲻��ʱ, ���ӽڵ�
	int m_iMaxCapacity; // �������, =0 ��Ч
};


template<class T>
bool DynMempool<T>::Init( int iInitNum, int iDelta, int iMaxCapacity )
{
	assert( iInitNum > 0 && iDelta >= 0 );

	if( iMaxCapacity > 0 && iInitNum > iMaxCapacity )
	{
		LOGWARN( "iInitNum (%d) > iMaxCapacity (%d)", iInitNum, iMaxCapacity );
		iInitNum = iMaxCapacity;
	}
		
	m_iInitNum = iInitNum;
	m_iDelta = iDelta;
	m_iMaxCapacity = iMaxCapacity;

	return _Alloc( iInitNum );
}


template<class T>
T* DynMempool<T>::Get()
{
	if( !m_stFreeList.m_next )
	{
		// ������
		assert( 0 == m_iFreeNum );
		if( m_iDelta <= 0 )
		{
			return NULL;
		}

		if( !this->_Alloc(m_iDelta) )
		{
			return NULL;
		}
	}

	assert( m_stFreeList.m_next!= NULL );

	PoolNode* pFreeNode = m_stFreeList.m_next;
	m_stFreeList.m_next = pFreeNode->m_next;
	pFreeNode->m_next = NULL;
	--m_iFreeNum;
	
	return ::new( (char*)pFreeNode ) T;
}


template<class T>
void DynMempool<T>::Release( T*& pElem )
{
	if( !pElem )
	{
		assert( false );
		return;
	}

	pElem->~T();
	
	PoolNode* pFreeNode = (PoolNode*)pElem;
	pFreeNode->m_next = m_stFreeList.m_next;
	m_stFreeList.m_next = pFreeNode;

	++m_iFreeNum;
	pElem = NULL;
}


template<class T>
bool DynMempool<T>::_Alloc( int iNum )
{
	assert( iNum >= 0 );

	if( m_stFreeList.m_next != NULL )
	{
		assert( false );
		LOGERR( "Still has free nodes, can not alloc more!" );
		return false;
	}

	if( m_iMaxCapacity > 0 )
	{
		int iLeftCapa = m_iMaxCapacity - m_iCapacity;
		iNum = ( iNum <= iLeftCapa ? iNum : iLeftCapa );
	}

	if( iNum <= 0 )
	{
		LOGERR("Cannot alloc more nodes! capacity <%d>, max_capacity <%d>", m_iCapacity, m_iMaxCapacity);
		return false;
	}

	int iUnit = (sizeof(T) < sizeof(T*)) ? sizeof(T*) : sizeof(T);
	char* buff = (char*)malloc( iNum * iUnit );
	
	if( !buff )
	{
		LOGERR( "Allocate %d elements failed!", iNum );
		return false;
	}

	m_stFreeList.m_next = (PoolNode*)&buff[0];
	PoolNode* pNode = NULL;
	for( int i = 0; i < iNum-1; i++ )
	{	
		pNode = (PoolNode*)&buff[ i*iUnit ];
		pNode->m_next = (PoolNode*)&buff[ (i+1)*iUnit ];
	}
	// last one
	pNode = (PoolNode*)&buff[ (iNum-1)*iUnit ];
	pNode->m_next = NULL;

	m_iCapacity += iNum;
	m_iFreeNum += iNum;

	return true;
}

