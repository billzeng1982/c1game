#ifndef _PRIORITY_QUEUE_H_
#define _PRIORITY_QUEUE_H_

/*
	优先队列
*/

#include "DynArray.h"
#include "PriorityQDefs.h"

template < typename T, typename Compare >
class PriorityQueue
{
public:
	PriorityQueue() : m_oArray( 16 )
	{
	}

	PriorityQueue( int iInitSize ) : m_oArray( iInitSize )
	{
	}

	~PriorityQueue( )
	{
	}

	bool Empty() { return m_oArray.Empty(); }

	void Clear() { m_oArray.Clear(); }

	// Returns a reference to the element at the top of the priority_queue
	T& Top()
	{
		return m_oArray.Front();
	}

	// Inserts x into the priority_queue
	void Push( const T& rItem );

	// Removes the element at the top of the priority_queue
	void Pop();

	// 外部更新了pos上的item权值，需要update, 重新排列
	void Update( int iPos );

	// low efficient 
	int IndexOf( const T& rstItem )
	{
		return m_oArray.IndexOf( rstItem );
	}

	// 重载[], 不进行范围检查!
	T& operator[]( int n )
	{
		return m_oArray[n];
	}

	int Length() { return m_oArray.Length(); }

private:
	int _SiftUp( int n );
	void _SiftDown( int n );

private:
	CDynArray<T> m_oArray;
};


template < typename T, typename Compare >
void PriorityQueue<T, Compare>::Push( const T& rItem )
{
	m_oArray.PushBack( rItem );

	this->_SiftUp( m_oArray.Length()-1 );
}


template < typename T, typename Compare >
void PriorityQueue<T, Compare >::Pop()
{
	if( Empty() )
	{
		return;
	}

	m_oArray[0] = m_oArray[ m_oArray.Length()-1 ];
	m_oArray.PopBack();
	if( m_oArray.Length() > 1 )
	{
		this->_SiftDown(0);
	}
}


// 外部更新了pos上的item权值，需要update, 重新排列
template < typename T, typename Compare >
void PriorityQueue<T, Compare >::Update( int iPos )
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

template < typename T, typename Compare >
int PriorityQueue<T, Compare >::_SiftUp( int n )
{
	if( 0 == n )
	{
		return 0;
	}

	T item = m_oArray[n]; // a copy

	Compare stCompareFunc;
	int iParent;

	for( iParent = PQ_PARENT(n); n > 0; n = iParent, iParent = PQ_PARENT(iParent) )
	{
		if( stCompareFunc( item, m_oArray[ iParent ] ) )
		{
			m_oArray[n] = m_oArray[iParent];
		}else
		{
			break;
		}
	}

	m_oArray[n] = item;

	return n;
}

template < typename T, typename Compare >
void PriorityQueue<T, Compare >::_SiftDown( int n )
{
	int iChild;

	T item = m_oArray[n];
	Compare stCompareFunc;
	int iLen = m_oArray.Length();

	for( iChild = PQ_LEFT_CHILD(n); iChild < iLen; n= iChild, iChild = PQ_LEFT_CHILD(iChild) )
	{
		if( iChild+1 < iLen &&
			stCompareFunc( m_oArray[iChild+1], m_oArray[iChild] ) )
		{
			// 比较左右子树中更优的一个交换
			iChild++;
		}

		if( stCompareFunc( m_oArray[iChild], item ) )
		{
			m_oArray[ n ] = m_oArray[iChild]; 
		}
		else
		{
			break;
		}
	}

	m_oArray[n] = item;
}

#endif

