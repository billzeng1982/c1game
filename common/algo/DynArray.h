#ifndef  _DYN_ARRAY_H_
#define _DYN_ARRAY_H_

/*
	动态数组, 在容量不够的情况下重新分配内存，copy先前的数据
	resize是耗时操作，不可频繁调用, only works for PODs, 按乘2增长
*/

#include <assert.h>
#include <string.h>
#include "define.h"

#define DYN_ARRAY_INIT_SIZE 200

template< typename T >
class CDynArray
{
public:
	CDynArray<T>()
	{
		m_iCapacity = DYN_ARRAY_INIT_SIZE;
		m_iLength = 0;
		m_paItems = new T[m_iCapacity];
		assert( m_paItems );
	}

	CDynArray<T>( int iInitSize )
	{
		m_iCapacity= iInitSize;
		m_iLength = 0;
		m_paItems = NULL;
		m_paItems = new T[m_iCapacity];
		assert( m_paItems );
	}

	~CDynArray()
	{
		if( m_paItems )
		{
			delete [] m_paItems;
			m_paItems = NULL;
		}
	}

	bool Resize();

	void PushBack( const T& rItem );

	void PopBack()
	{
		if( m_iLength > 0 )
		{
			--m_iLength;
		}
	}

	void PopBack( int iNum )
	{
		if( iNum <= 0 )
		{
			return;
		}
		
		m_iLength -= iNum;
		if( m_iLength < 0 )
		{
			m_iLength = 0;
		}
	}

	// 返回最后一个元素, 不检查最后一个元素是否存在
	T& Back() 
	{
		if( 0 == m_iLength )
		{
			return m_paItems[0];
		}else
		{
			return m_paItems[m_iLength-1];
		}
	}

	// 返回第一个元素, 不检查第一个元素是否存在
	T& Front() { return m_paItems[0]; }

	int Capacity( ) { return m_iCapacity; }

	int Length() const { return m_iLength; }

	// 重载[], 不进行范围检查!
	T& operator[]( int n )
	{
		return m_paItems[n];
	}

	const T& operator[](int n) const
	{
		return m_paItems[n];
	}

	void Clear( )
	{
		m_iLength = 0;
	}

	bool Empty()
	{
		return ( 0 == m_iLength );
	}

	int IndexOf( const T& rstItem );

	void Erase( int iPos );

private:	
	int m_iCapacity;
	int m_iLength; // 当前item数量
	T* 	m_paItems; // 数组首地址
};


template< typename T >
bool CDynArray<T>::Resize()
{
	int iNewCapacity = m_iCapacity * 2;
	T* paNewItems = new T[iNewCapacity];
	if( !paNewItems )
	{
		assert( false );
		return false;
	}
	
	memcpy( paNewItems, m_paItems, sizeof(T)*m_iLength ); // warning: not using constructors, only works for PODs

	delete [] m_paItems;
	m_paItems = paNewItems;
	m_iCapacity = iNewCapacity;
	
	return true;
}


template< typename T >
void CDynArray<T>::PushBack( const T& rItem )
{
	if( !m_paItems )
	{
		m_paItems = new T[m_iCapacity];
	}

	if( m_iLength >= m_iCapacity )
	{
		if( !Resize() )
		{
			return;
		}
	}

	m_paItems[m_iLength++] = rItem;
}


// low efficient !
template< typename T >
int CDynArray<T>::IndexOf( const T& rstItem )
{
	for( int i = 0; i < m_iLength; i++ )
	{
		if( m_paItems[i] == rstItem )
		{
			return i;
		}
	}

	return -1;
}


template< typename T >
void CDynArray<T>::Erase( int iPos )
{
	if( iPos < 0 || iPos >=m_iLength )
	{
		assert( false );
		return;
	}

	if( iPos != m_iLength - 1 ) 
	{
		m_paItems[iPos] = m_paItems[m_iLength - 1];  //只将最后一个压缩即可
	}

	m_iLength--;
	return;
}

#endif

