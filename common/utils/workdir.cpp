#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "workdir.h"

char CWorkDir::m_szWorkDir[512];

// 传入的work dir到deploy一级!
bool CWorkDir::InitWorkDir( char* pszWorkDir )
{
	if( !pszWorkDir || pszWorkDir[0] == '\0' )
	{
		return false;
	}

	snprintf( m_szWorkDir, sizeof(m_szWorkDir), "%s", pszWorkDir );
	return true;
}

