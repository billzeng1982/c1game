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
	
	// �����conf xml��deployһ��!
	static bool InitConfXml( char* pszConfXml );
	static char* string() { return m_szConfXml; }

private:
	static char m_szConfXml[256];
};

