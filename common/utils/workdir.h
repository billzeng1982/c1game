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
	
	// 传入的work dir到deploy一级!
	static bool InitWorkDir( char* pszWorkDir );
	static char* string() { return m_szWorkDir; }

private:
	static char m_szWorkDir[512];
};

#endif

