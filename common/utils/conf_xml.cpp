#include <stdio.h>
#include <string.h>
#include "conf_xml.h"

char CConfXml::m_szConfXml[256];

// �����conf xml��deployһ��!
bool CConfXml::InitConfXml( char* pszConfXml )
{
	if( !pszConfXml || pszConfXml[0] == '\0' )
	{
		return false;
	}

	snprintf( m_szConfXml, sizeof(m_szConfXml), "%s", pszConfXml );
	return true;
}

