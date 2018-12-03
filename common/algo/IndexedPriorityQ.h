#pragma once

/*
	���ȶ������key-pos����, ��������ȶ������Ԫ�ؽ���ɾ, ��
	������ҪƵ��ɾ�����ȶ������Ԫ�ص�Ӧ�ã�ʹ�ñ����ݽṹ���ײ㹹��

	ע��: 
	1.���ȶ�����İ���key�ǿ����ظ���,�������õ�UniKey��Ψһ�ģ��Ҷ��߲�ͬ
	2.compare < 0 ��������,��Ĭ��ΪС����

	���ڿ���չ:
	UniKey2PosMap_tʹ�õ�Ĭ�� stl::less<T>, �����þ���չһ��ģ�����(functor)���ڶ���UniKey�ıȽ�

	�Ż�:
	key-posӳ��ĳ�key-pos*ӳ��, �ڶ������ƶ�Ԫ�ظı�posʱ, ����ȥmap�����, ֱ���õ�ַȥ��posֵ, �������Ч��
*/

#include "comparer.h"
#include "DynArray.h"
#include "PriorityQDefs.h"
#include "../mng/DynMempool.h"
#include <map>

using namespace std;

/* 
	T: Element, UniKey: used for index
*/
template < typename T, typename UniKey >
class IndexedPriorityQ
{
public:
	typedef std::map< UniKey, int* /* pos* */ > UniKey2PosMap_t;
	struct ElemNode
	{
		T 	m_elem;
		int *m_pPos;

		ElemNode()
		{
			m_pPos = NULL;
		}
	};
	
private:
	void _construct( Comparer<T>* pComparer, GetItemKey<T, UniKey>* pGetKey )
	{
		this->m_pComparer = ( pComparer == NULL ) ? ( new Comparer<T>() ) : pComparer;
		this->m_pGetKey = ( pGetKey == NULL ) ? ( new GetItemKey<T, UniKey>() ) : pGetKey;
		if( !m_oElemPosPool.Init( m_oArray.Capacity(), m_oArray.Capacity() ) )
		{
			assert( false );
		}
	}

public:
	IndexedPriorityQ() : m_oArray(16) { this->_construct(NULL, NULL); }
	IndexedPriorityQ( int capacity ) : m_oArray(capacity) { this->_construct(NULL, NULL); }
	IndexedPriorityQ( int capacity, Comparer<T>* pComparer, GetItemKey<T, UniKey>* pGetKey ) : m_oArray(capacity)
	{
		this->_construct( pComparer, pGetKey );
	}

	~IndexedPriorityQ()
	{	
	}

	bool Empty() { return m_oArray.Empty(); }

	void Clear() 
	{ 
		for( int i = 0; i < m_oArray.Length(); i++ )
		{
			m_oElemPosPool.Release( m_oArray[i].m_pPos );
		}
		m_oArray.Clear(); 
		m_oUniKey2PosMap.clear();
	}

	// Returns a reference to the element at the top of the priority_queue
	T& Top()
	{
		return m_oArray[0].m_elem;
	}

	// Inserts x into the priority_queue
	void Push( const T& rElem);

	// Removes the element at the top of the priority_queue
	void Pop();

	int Length() { return m_oArray.Length(); }

	// return pos
	int Find( const UniKey& rUniKey );
	
	// ����[], �����з�Χ���!
	T& operator[]( int n )
	{
		return m_oArray[n].m_elem;
	}

	// �ⲿ������pos�ϵ�itemȨֵ����Ҫupdate, ��������
	void Update( int iPos );

	// Removes the element with unique key
	void Erase( const UniKey& rUniKey );
	
private:
	void _UpdateKeyPos( ElemNode& rElemNode, int iPos ) { *(rElemNode.m_pPos) = iPos; }
	int  _SiftUp( int n );
	void _SiftDown( int n );

	// �������
	bool _AddIndex( ElemNode& rElemNode, int iPos );
	// ɾ������
	void _DelIndex( ElemNode& rElemNode );
	void _DelIndex( int*& pPos, typename UniKey2PosMap_t::iterator& it );

private:
	CDynArray<ElemNode> m_oArray;  // for priority Q
	Comparer<T>* 		m_pComparer; // for priority Q
	GetItemKey<T, UniKey>* m_pGetKey; 
	UniKey2PosMap_t 	m_oUniKey2PosMap;
	DynMempool<int>		m_oElemPosPool;

	char m_szErrMsg[256];
};

template < typename T, typename UniKey >
bool IndexedPriorityQ<T, UniKey>::_AddIndex( ElemNode& rElemNode, int iPos )
{
	int* pPos = m_oElemPosPool.Get();
	if( !pPos )	
	{
		assert( false );
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "m_oElemPosPool get a null pointer!" );
		return false;
	}

	rElemNode.m_pPos = pPos;
	*(rElemNode.m_pPos) = iPos;
	
	m_oUniKey2PosMap.insert( typename UniKey2PosMap_t::value_type( m_pGetKey->GetKey( rElemNode.m_elem) , rElemNode.m_pPos ) );
	
	// debug
	//printf("After Add, DynMempool capacity: %d, free %d\n",m_oElemPosPool.GetCapacity(), m_oElemPosPool.GetFreeNum());

	return true;
}

template < typename T, typename UniKey >
void IndexedPriorityQ<T, UniKey>::_DelIndex( ElemNode& rElemNode )
{
	// remove from map
	typename UniKey2PosMap_t::iterator it = m_oUniKey2PosMap.find( m_pGetKey->GetKey(rElemNode.m_elem) );
	this->_DelIndex( rElemNode.m_pPos, it );
}

template < typename T, typename UniKey >
void IndexedPriorityQ<T, UniKey>::_DelIndex( int*& pPos, typename UniKey2PosMap_t::iterator& it )
{
	assert( pPos != NULL );

	m_oUniKey2PosMap.erase( it );
	m_oElemPosPool.Release( pPos );

	// debug
	//printf("After Del, DynMempool capacity: %d, free %d\n",m_oElemPosPool.GetCapacity(), m_oElemPosPool.GetFreeNum());
}

template < typename T, typename UniKey >
void IndexedPriorityQ<T, UniKey>::Push( const T& rElem  )
{
	typename UniKey2PosMap_t::iterator it = m_oUniKey2PosMap.find( m_pGetKey->GetKey( rElem ) );
	if( it != m_oUniKey2PosMap.end() )
	{
		// unikey �Ѿ�����
		return;
	}

	ElemNode stElemNode;
	stElemNode.m_elem = rElem;
	stElemNode.m_pPos = NULL;

	if( !this->_AddIndex( stElemNode, m_oArray.Length() ) )
	{
		return;
	}

	m_oArray.PushBack( stElemNode );
	this->_SiftUp( m_oArray.Length()-1 );
}


template < typename T, typename UniKey >
void IndexedPriorityQ<T, UniKey>::Pop()
{
	if( Empty() )
	{
		return;
	}

	this->_DelIndex( m_oArray[0] );
		
	if( m_oArray.Length() > 1 )
	{
		m_oArray[0] = m_oArray[ m_oArray.Length()-1 ];
		this->_UpdateKeyPos(m_oArray[0], 0);	
	}

	m_oArray.PopBack();
	if( m_oArray.Length() > 1 )
	{
		this->_SiftDown(0);
	}
}


// return pos
template < typename T, typename UniKey >
int IndexedPriorityQ<T, UniKey>::Find( const UniKey& rUniKey )
{
	typename UniKey2PosMap_t::iterator it = m_oUniKey2PosMap.find( rUniKey );

	if( it == m_oUniKey2PosMap.end() )
	{
		return -1;
	}

	return *(it->second);
}

template < typename T, typename UniKey >
void IndexedPriorityQ<T, UniKey>::Update( int iPos )
{
	if( iPos < 0 || iPos >= m_oArray.Length() )
	{
		return;
	}

	int i = this->_SiftUp(iPos);
	if( i < iPos )
	{
		return;
	}

	this->_SiftDown(iPos);
}

template < typename T, typename UniKey >
void IndexedPriorityQ<T, UniKey>::Erase( const UniKey& rUniKey )
{
	typename UniKey2PosMap_t::iterator it = m_oUniKey2PosMap.find( rUniKey );
	if( it == m_oUniKey2PosMap.end() )
	{
		return;
	}

	int iPos = *(it->second);
	this->_DelIndex( m_oArray[iPos].m_pPos, it );

	// tail elem
	if( iPos == m_oArray.Length() - 1 )
	{
		m_oArray.PopBack();
		return;
	}

	m_oArray[iPos] = m_oArray[ m_oArray.Length()-1 ];
	m_oArray.PopBack();
	this->_UpdateKeyPos(m_oArray[iPos], iPos);

	this->Update(iPos);
}

template < typename T, typename UniKey >
int  IndexedPriorityQ<T, UniKey>::_SiftUp( int n )
{
	if( 0 == n )
	{
		// head
		return 0;
	}

	ElemNode stElemNode = m_oArray[n]; // a copy
	int iParent;

	for( iParent = PQ_PARENT(n); n > 0; n = iParent, iParent = PQ_PARENT(iParent) )
	{
		if( m_pComparer->Compare( stElemNode.m_elem, m_oArray[ iParent ].m_elem ) < 0 )
		{
			m_oArray[n] = m_oArray[iParent];
			this->_UpdateKeyPos( m_oArray[n], n );
		}else
		{
			break;
		}
	}

	m_oArray[n] = stElemNode;
	this->_UpdateKeyPos( m_oArray[n], n );

	return n;
}

template < typename T, typename UniKey >
void IndexedPriorityQ<T, UniKey>::_SiftDown( int n )
{
	if( n == m_oArray.Length() -1 )
	{
		// tail
		return;
	}

	int iChild;

	ElemNode stElemNode = m_oArray[n];
	int iLen = m_oArray.Length();

	for( iChild = PQ_LEFT_CHILD(n); iChild < iLen; n= iChild, iChild = PQ_LEFT_CHILD(iChild) )
	{
		if( iChild+1 < iLen &&
			m_pComparer->Compare( m_oArray[iChild+1].m_elem, m_oArray[iChild].m_elem ) < 0 )
		{
			// �Ƚ����������и��ŵ�һ������
			iChild++;
		}

		if( m_pComparer->Compare( m_oArray[iChild].m_elem, stElemNode.m_elem ) < 0 )
		{
			m_oArray[ n ] = m_oArray[iChild]; 
			this->_UpdateKeyPos( m_oArray[n], n );
		}
		else
		{
			break;
		}
	}

	m_oArray[n] = stElemNode;
	this->_UpdateKeyPos( m_oArray[n], n );
}



