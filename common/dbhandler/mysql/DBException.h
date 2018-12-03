#ifndef _DB_EXCETION_H_
#define _DB_EXCETION_H_

#include <stdio.h>

#define DB_EXCEPTION(type) CDBException( __FILE__, __FUNCTION__, __LINE__, type )

enum DB_EXP_TYPE
{
  DB_EXCEPTION_NONE        = 0,
  DB_EXCEPTION_CONN_FAIL   = 1,
  DB_EXCEPTION_SQL_ERR     = 2,
  DB_EXCEPTION_NULL_RECORD = 3,
  DB_EXCEPTION_NULL_FIELD  = 4,
};

class CDBException
{
public:
	CDBException( const char* pszFile, const char* pszFunc, const int iLineNum, DB_EXP_TYPE eType )
	{
		m_eType = eType;
		snprintf( m_szInfo, sizeof(m_szInfo), "%s: %s : %d :%d",
			pszFile,pszFunc, iLineNum, eType );
	}

	~CDBException(){}

	DB_EXP_TYPE GetType() { return m_eType; }
	char* GetInfo() { return m_szInfo; }
	
private:
	static const int DB_EXP_INFO_LEN = 512;
	char m_szInfo[ DB_EXP_INFO_LEN ];
 	DB_EXP_TYPE  m_eType;
};


#endif

