#ifndef _SEQ_NO_GEN_H_
#define _SEQ_NO_GEN_H_

/*
	序列号生成器，主要用于产生object id, 64位
*/

#include <assert.h>
#include "define.h"

class CSeqNoGen
{
public:
	CSeqNoGen()
	{
		m_ulLowerBound = 1;
		m_ulUpperBound = 0xFFFFFFFF; 
		m_ulCurrSeqNo  = 0;
	}

	CSeqNoGen( uint64_t ulLowerBound, uint64_t ulUpperBound )
	{
		assert( ulUpperBound > ulLowerBound );
		m_ulLowerBound = ulLowerBound;
		m_ulUpperBound = ulUpperBound;
		m_ulCurrSeqNo  = m_ulLowerBound;
	}
	
	~CSeqNoGen() {}

	void SetLowerBound( uint64_t ulLowerBound )
	{
		m_ulLowerBound = ulLowerBound;
	}

	void SetUpperBound( uint64_t ulUpperBound )
	{
		m_ulUpperBound = ulUpperBound;
	}

	void SetCurrSeqNo( uint64_t ulCurrSeq )
	{
		if( ulCurrSeq < m_ulLowerBound )
		{
			assert( false );
			m_ulCurrSeqNo = m_ulLowerBound;
		}
		else if( ulCurrSeq > m_ulUpperBound )
		{
			assert( false );
			m_ulCurrSeqNo = m_ulUpperBound;
		}
		else
		{
			 m_ulCurrSeqNo = ulCurrSeq;
		}
	}

	uint64_t GetCurrSeqNo( )
	{
		return m_ulCurrSeqNo;
	}

	uint64_t GetNextSeqNo()
	{	
		if( m_ulCurrSeqNo < m_ulLowerBound )
		{
			m_ulCurrSeqNo = m_ulLowerBound;
		}
		else
		{
			++m_ulCurrSeqNo;
			if( m_ulCurrSeqNo == m_ulUpperBound )
			{
				m_ulCurrSeqNo = m_ulLowerBound; // 回绕
			}
		}

		return m_ulCurrSeqNo;
	}
	
private:
	uint64_t m_ulLowerBound;
	uint64_t m_ulUpperBound;
	uint64_t m_ulCurrSeqNo;	
};

#endif

