#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "workdir.h"

char CWorkDir::m_szWorkDir[512];

// �����work dir��deployһ��!
bool CWorkDir::InitWorkDir( char* pszWorkDir )
{
	if( !pszWorkDir || pszWorkDir[0] == '\0' )
	{
		return false;
	}

	snprintf( m_szWorkDir, sizeof(m_szWorkDir), "%s", pszWorkDir );
	return true;
}

