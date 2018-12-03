#ifndef _BIT_MAP_H_
#define _BIT_MAP_H_

/*
	位图, 主要用于各种旗标运算

	注意:
	只支持位宽(bitwise)为 1, 2, 4, 8(退化为数组)的位图!
*/

#include <string.h>
#include <assert.h>
#include "define.h"

// 默认为一个byte
template< int BITWISE = 8 >
class CBitMap
{
public:
	CBitMap()
	{
		assert( 1==BITWISE || 2==BITWISE || 4==BITWISE || 8==BITWISE );
		m_cBitwise = BITWISE;
		m_szBitBuf = NULL;
		m_iBitBufSize = 0;
		m_bSelfNew	= false; 

		m_byMaxBitVal = (uint8_t)(int(1<<m_cBitwise)-1);
		m_bySlotsPerByte = 8 / m_cBitwise;
	}
	
	CBitMap( int iBitBufSize )
	{
		assert( 1==BITWISE || 2==BITWISE || 4==BITWISE || 8==BITWISE);
		m_szBitBuf = NULL;
		m_cBitwise = BITWISE;
		m_iBitBufSize = iBitBufSize;
		m_bSelfNew	= false;
		
		m_byMaxBitVal = (uint8_t)(int(1<<m_cBitwise)-1);
		m_bySlotsPerByte = 8 / m_cBitwise;
	}

	CBitMap( char* pszBitBuf, int iBitBufSize )
	{
		assert( 1==BITWISE || 2==BITWISE || 4==BITWISE || 8==BITWISE );
		m_szBitBuf = pszBitBuf;
		m_cBitwise = BITWISE;
		m_iBitBufSize = iBitBufSize;
		m_bSelfNew	= false;
		
		m_byMaxBitVal = (uint8_t)(int(1<<m_cBitwise)-1);
		m_bySlotsPerByte = 8 / m_cBitwise;
	}

	~CBitMap()
	{
		if( m_bSelfNew && m_szBitBuf )
		{
			delete [] m_szBitBuf;
			m_szBitBuf = NULL;
		}
	}

	// buf来自外部
	void SetBitMap( char* pszBitBuf, int iBufSize )
	{
		if( m_bSelfNew && m_szBitBuf )
		{
			delete [] m_szBitBuf;
		}
		m_szBitBuf = pszBitBuf;
		m_iBitBufSize = iBufSize;
	}

	// iBitPos 从0开始!!
	void SetVal( int iBitPos, uint8_t byVal );
	
	uint8_t GetVal( int iBitPos );

	void Clear()
	{
		if( m_szBitBuf )
		{
			memset( m_szBitBuf, 0x00, m_iBitBufSize );
		}
	}
	
private:
	char m_cBitwise;
	char* m_szBitBuf;
	int	 m_iBitBufSize; // char数组的大小
	bool m_bSelfNew;
	
	uint8_t m_byMaxBitVal;
	uint8_t m_bySlotsPerByte;
};


// iBitPos 从0开始!!
template < int BITWISE >
void CBitMap<BITWISE>::SetVal( int iBitPos, uint8_t byVal )
{
	if( !m_szBitBuf )
	{
		m_szBitBuf = new char[m_iBitBufSize];
		this->Clear();
		m_bSelfNew = true;
	}

	if( iBitPos < 0	)
	{
		assert( false );
		return;
	}

	if( byVal > m_byMaxBitVal )
	{
		assert( false );
		return;
	}

	int iQuotient = iBitPos / m_bySlotsPerByte;
	int iRemainder = iBitPos % m_bySlotsPerByte;
	if( iQuotient >= m_iBitBufSize )	
	{
		return;
	}

	uint8_t byTmp = 0;
	byTmp = ( m_szBitBuf[iQuotient] & ~( m_byMaxBitVal << (iRemainder*m_cBitwise) ) ) | ( byVal << (iRemainder*m_cBitwise) );

	m_szBitBuf[iQuotient] = byTmp;
}


template < int BITWISE >
uint8_t CBitMap<BITWISE>::GetVal( int iBitPos )
{
	if( iBitPos < 0	)
	{
		assert( false );
		return 0;
	}

	if( NULL == m_szBitBuf)
	{
	    return 0;
	}

	int iQuotient = iBitPos / m_bySlotsPerByte;
	int iRemainder = iBitPos % m_bySlotsPerByte;

	if( iQuotient >= m_iBitBufSize )	
	{
		return 0;
	}

	uint8_t byRet = 0;
	byRet = ( m_szBitBuf[iQuotient] & ( m_byMaxBitVal << (iRemainder*m_cBitwise) ) ) >> (iRemainder*m_cBitwise);

	return byRet;
}

#endif

