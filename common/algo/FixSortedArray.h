#ifndef _FIX_SORTED_ARRAY_H_
#define _FIX_SORTED_ARRAY_H_
/*
	�̶���������������, ����������!
	Compare functor �ķ��ر�����:
	== 0: ���
	< 0:  С��
	> 0:  ����

	�������򣬲���ʹ�ö��ֲ��ҷ�
	����ⲿ��ָ�������׵�ַ,�������Լ�new����!
	ע��:
	1. ������"��"��, ��ζ�Ų����ɾ�����������������ƶ�
	2. �ⲿͨ��operator[]������pos�ϵ�item keyֵʱ, ��Ҫ����Update, �������г������!!

	�����ʽ: (Ϊ��֧�ֹ����ڴ�)
	----------------------
	| Length | Data ....
	----------------------
*/


#include <assert.h>

/* 
	GetKey ��ʾ��data�з���key������
	Capacity: �����С
	Unique: key�Ƿ�Ψһ, Ĭ��Ϊ����key�ظ�
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


	// �������һ��Ԫ��, ��������һ��Ԫ���Ƿ����
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

	// ���ص�һ��Ԫ��, ������һ��Ԫ���Ƿ����
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

	// ����[], �����з�Χ���!
	TData& operator[]( int n )
	{
		return m_paItems[n];
	}

	int Find( const TKey& key );

	// �ⲿ������pos�ϵ�item keyֵ����Ҫupdate, ��������
	void Update( int iPos );

	/* ���� >= 0, λ����Ч, < 0 ����
	   bFound = true��ʾ�ҵ�key��Ӧ��Ԫ�أ�= false��ʾû���ҵ�, �����ص�int��ʾ��keyӦ�ô��ڵ�λ��
	*/
	int BinarySearch( const TKey& key, bool& bFound ); 

private:
	char*	m_pszBuf;
	int 	m_iCapacity;
	int* 	m_piLength; // ��ǰitem����ָ��
	TData* 	m_paItems; // �����׵�ַ
	bool	m_bUnique;
	bool 	m_bSelfNew; // ����Ԫ���Ƿ��Լ�new����
};


/* ���� >= 0, λ����Ч, < 0 ����
   bFound = true��ʾ�ҵ�key��Ӧ��Ԫ�أ�= false��ʾû���ҵ�, �����ص�int��ʾ��keyӦ�ô��ڵ�λ��
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
		m_paItems[ iLeft+1 ] = m_paItems[ iLeft ]; // ����
		iLeft--;
	}
		
	if( iLeft < iPos - 1 )
	{
		m_paItems[ iLeft+1 ] = tmpData;
		return;
	}

	while( iRight < *m_piLength && stCompareFunc( tmpKey, stGetKeyFunc( m_paItems[iRight] ) ) > 0 )
	{
		m_paItems[ iRight-1 ] = m_paItems[ iRight ]; // ����
		iRight++;
	}

	if( iRight > iPos + 1 )
	{
		m_paItems[ iRight-1 ] = tmpData;
		return;
	}
}

#endif

