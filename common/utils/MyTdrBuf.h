#ifndef _MY_TDR_BUF_H_
#define _MY_TDR_BUF_H_

struct MyTdrBuf
{
	size_t m_uPackLen;
	size_t m_uSize;
	char* m_szTdrBuf;

	MyTdrBuf()
	{
		m_uSize = 0;
		m_uPackLen = 0;
		m_szTdrBuf = NULL;
	}
	
	MyTdrBuf(size_t uSize)
	{
		m_uPackLen = 0;
		m_uSize = uSize;
		if( uSize > 0 )
		{
			m_szTdrBuf = new char[uSize];
		}else
		{
			m_szTdrBuf = NULL;
		}
	}

	~MyTdrBuf()
	{
		if( m_szTdrBuf )
		{
			delete[] m_szTdrBuf;
			m_szTdrBuf = NULL;
		}
	}

	void Reset()
	{
		if( m_uSize > 0 )
		{
			m_szTdrBuf[0] = '\0';
			m_uPackLen = 0;
		}
		else
		{
			m_szTdrBuf = NULL;
			m_uPackLen = 0;
		}
	}
};

#endif

