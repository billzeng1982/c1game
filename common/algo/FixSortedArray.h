#ifndef _FIX_SORTED_ARRAY_H_
#define _FIX_SORTED_ARRAY_H_
/*
	固定容量的有序数组, 按升序排列!
	Compare functor 的返回必须是:
	== 0: 相等
	< 0:  小于
	> 0:  大于

	由于有序，查找使用二分查找法
	如果外部不指定数组首地址,则本容器自己new出来!
	注意:
	1. 数组是"紧"的, 意味着插入和删除操作会引起数据移动
	2. 外部通过operator[]更新了pos上的item key值时, 需要调用Update, 重新排列成有序的!!

	数组格式: (为了支持共享内存)
	----------------------
	| Length | Data ....
	----------------------
*/


#include <assert.h>

/* 
	GetKey 表示从data中返回key的引用
	Capacity: 数组大小
	Unique: key是否唯一, 默认为允许key重复
*/
template < typename TKey, typename TData, typename Compare, typename GetKey, bool Unique = false  >
class CFixSortedArray
{
public:
	CFixSortedArray( )
	{
		m_iCapacity = 0;
		m_pszBuf = NULL;
		m_piLength = NULL;
		m_paItems = NULL;
		m_bUnique = Unique; 
		m_bSelfNew = false;
	}
	
	CFixSortedArray( int iCapacity )
	{
		m_iCapacity = iCapacity;
		m_pszBuf   = NULL;
		m_piLength = NULL;
		m_paItems = NULL;
		m_bUnique = Unique; 
		m_bSelfNew = false;
	}

	CFixSortedArray( char* szBuf, int iCapacity )
	{
		m_iCapacity = iCapacity;
		m_pszBuf = szBuf;
		m_piLength = (int*)m_pszBuf;
		m_paItems = (TData*)(m_pszBuf + sizeof(int));
		m_bUnique = Unique; 
		m_bSelfNew = false;
	}

	~CFixSortedArray()
	{
		if( m_bSelfNew && m_pszBuf )
		{
			delete [] m_pszBuf;
		}
	}

	void SetArray( char* szBuf, int iCapacity )
	{
		if( m_bSelfNew && m_pszBuf )
		{
			delete [] m_pszBuf;
		}

		m_pszBuf = szBuf;
		m_piLength = (int*)m_pszBuf;
		m_paItems = (TData*)(m_pszBuf + sizeof(int));
		m_iCapacity = iCapacity;
	}


	// 返回最后一个元素, 不检查最后一个元素是否存在
	TData& Back()
	{
		if( 0 == *m_piLength )
		{
			return m_paItems[0];
		}else
		{
			return m_paItems[*m_piLength-1];
		}
	}

	// 返回第一个元素, 不检查第一个元素是否存在
	TData& Front() { return m_paItems[0]; }

	bool Insert( const TData& x );

	void Erase( const TKey& key );

	void EraseByPos( int iPos );

	void Clear( ) 
	{ 
		if( m_piLength )
		{
			*m_piLength = 0; 
		}
	}

	int Capacity( ) { return m_iCapacity; }

	int Length() { return *m_piLength; }
	
	bool Empty() {	return ( 0 == *m_piLength ); }

	// 重载[], 不进行范围检查!
	TData& operator[]( int n )
	{
		return m_paItems[n];
	}

	int Find( const TKey& key );

	// 外部更新了pos上的item key值，需要update, 重新排列
	void Update( int iPos );

	/* 返回 >= 0, 位置有效, < 0 出错
	   bFound = true表示找到key对应的元素，= false表示没有找到, 但返回的int表示该key应该存在的位置
	*/
	int BinarySearch( const TKey& key, bool& bFound ); 

private:
	char*	m_pszBuf;
	int 	m_iCapacity;
	int* 	m_piLength; // 当前item数量指针
	TData* 	m_paItems; // 数组首地址
	bool	m_bUnique;
	bool 	m_bSelfNew; // 数组元素是否自己new出来
};


/* 返回 >= 0, 位置有效, < 0 出错
   bFound = true表示找到key对应的元素，= false表示没有找到, 但返回的int表示该key应该存在的位置
*/
template < typename TKey, typename TData, typename Compare, typename GetKey, bool Unique >
int CFixSortedArray<TKey, TData, Compare, GetKey, Unique>::BinarySearch( const TKey& key, bool& bFound )
{
	assert( m_paItems );

	bFound = false;
	if( 0 == *m_piLength )
	{
		return 0;
	}

	Compare stCompareFunc;
	GetKey  stGetKeyFunc;

	int iLow = 0;
	int iHigh = *m_piLength;
	int iMid = 0;
	int iCmpVal = 0;

	if( 0 ==  stCompareFunc( key, stGetKeyFunc( m_paItems[iLow] ) ) )
	{
		bFound = true;
		return iLow;
	}

	if( 0 ==  stCompareFunc( key, stGetKeyFunc( m_paItems[iHigh-1] ) ) )
	{
		bFound = true;
		return iHigh-1;
	}

	while( iLow < iHigh )
	{
		iMid = iLow + (( iHigh - iLow ) >> 1);

		iCmpVal = stCompareFunc( key, stGetKeyFunc( m_paItems[iMid] ) );

		if( 0 == iCmpVal )
		{
			bFound = true;
			return iMid;
		}
		else if( iCmpVal < 0 )
		{
			if( 0 == iMid )
			{
				return iMid;
			}

			if( stCompareFunc(key, stGetKeyFunc( m_paItems[iMid-1] ) ) > 0 )
			{
				return iMid;
			}

			iHigh = iMid;
		}
		else
		{
			iLow = iMid + 1;
		}
	}

	return iHigh;
}


template < typename TKey, typename TData, typename Compare, typename GetKey, bool Unique >
bool CFixSortedArray<TKey, TData, Compare, GetKey, Unique>::Insert( const TData& x )
{
	if( !m_paItems )
	{
		m_pszBuf = new char[ sizeof(int) + sizeof( TData ) * m_iCapacity ];
		assert( m_pszBuf );
		m_piLength = (int*)m_pszBuf;
		m_paItems = (TData*)(m_pszBuf + sizeof(int));
		m_bSelfNew = true;
	}

	if( *m_piLength >= m_iCapacity )
	{
		return false;
	}
	
	GetKey  stGetKeyFunc;
	
	int iInsPos = 0;
	bool bFound = false;

	iInsPos = BinarySearch( stGetKeyFunc(x), bFound );

	if( iInsPos < 0 )
	{
		return false;
	}

	if( bFound && m_bUnique )
	{
		return false;
	}

	if( iInsPos < *m_piLength )
	{
		for( int i = *m_piLength; i > iInsPos; i-- )
		{
			m_paItems[ i ] = m_paItems[ i - 1 ];
		}
	}

	m_paItems[ iInsPos ] = x;
	++(*m_piLength);

	return true;
}


template < typename TKey, typename TData, typename Compare, typename GetKey, bool Unique >
void CFixSortedArray<TKey, TData, Compare, GetKey, Unique>::Erase( const TKey& key )
{
	int iDelPos = 0;
	bool bFound = false;

	iDelPos = BinarySearch( key, bFound );

	if( !bFound  )
	{
		return;
	}

	if( iDelPos < *m_piLength )
	{
		for( int i = iDelPos; i < ( *m_piLength - 1 ); i++ )
		{
			m_paItems[i] = m_paItems[i+1];
		}
	}

	--(*m_piLength);
	return;
}


template < typename TKey, typename TData, typename Compare, typename GetKey, bool Unique >
void CFixSortedArray<TKey, TData, Compare, GetKey, Unique>::EraseByPos( int iPos )
{
	if( iPos < 0 || iPos >= *m_piLength )
	{
		return;
	}

	for( int i = 0; i < ( *m_piLength - 1 ); i++ )
	{
		m_paItems[i] = m_paItems[i+1];
	}

	--(*m_piLength);
	return;
}


template < typename TKey, typename TData, typename Compare, typename GetKey, bool Unique >
int CFixSortedArray<TKey, TData, Compare, GetKey, Unique>::Find( const TKey& key )
{
	int iPos = 0;
	bool bFound = false;

	iPos = BinarySearch( key, bFound );

	if( bFound )
	{
		return iPos;
	}
	else
	{
		return -1;
	}
}


template < typename TKey, typename TData, typename Compare, typename GetKey, bool Unique >
void CFixSortedArray<TKey, TData, Compare, GetKey, Unique>::Update( int iPos )
{
	if( iPos < 0 || iPos >= *m_piLength )
	{
		return;
	}

	Compare stCompareFunc;
	GetKey  stGetKeyFunc;

	int iLeft = iPos - 1;
	int iRight = iPos + 1;

	TData tmpData = m_paItems[iPos];
	const TKey& tmpKey = stGetKeyFunc( tmpData );

	while( iLeft >= 0 && stCompareFunc( tmpKey, stGetKeyFunc( m_paItems[iLeft] ) ) < 0 )
	{
		m_paItems[ iLeft+1 ] = m_paItems[ iLeft ]; // 右移
		iLeft--;
	}
		
	if( iLeft < iPos - 1 )
	{
		m_paItems[ iLeft+1 ] = tmpData;
		return;
	}

	while( iRight < *m_piLength && stCompareFunc( tmpKey, stGetKeyFunc( m_paItems[iRight] ) ) > 0 )
	{
		m_paItems[ iRight-1 ] = m_paItems[ iRight ]; // 左移
		iRight++;
	}

	if( iRight > iPos + 1 )
	{
		m_paItems[ iRight-1 ] = tmpData;
		return;
	}
}

#endif

