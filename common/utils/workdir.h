#ifndef _WORK_DIR_
#define _WORK_DIR_
#include <string.h>

class CWorkDir
{
public:
	CWorkDir() 
	{
		bzero( m_szWorkDir, sizeof(m_szWorkDir) );
	}

	~CWorkDir(){}
	
	// �����work dir��deployһ��!
	static bool InitWorkDir( char* pszWorkDir );
	static char* string() { return m_szWorkDir; }

private:
	static char m_szWorkDir[512];
};

#endif

