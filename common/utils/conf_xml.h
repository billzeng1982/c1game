#pragma once
#include <string.h>

class CConfXml
{
public:
	CConfXml() 
	{
		bzero( m_szConfXml, sizeof(m_szConfXml) );
	}

	~CConfXml(){}
	
	// 传入的conf xml到deploy一级!
	static bool InitConfXml( char* pszConfXml );
	static char* string() { return m_szConfXml; }

private:
	static char m_szConfXml[256];
};

