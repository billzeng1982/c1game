#ifndef _DATA_TABLE_H_
#define _DATA_TABLE_H_

#include "MysqlHandler.h"
#include <assert.h>
#include "strutil.h"


class CDataTable
{
public:
	CDataTable()
	{ 
		m_szTableName[0] = '\0'; 
		m_poMysqlHandler = NULL;
	}

	virtual ~CDataTable() {}

	bool Init(const char* pszTableName, CMysqlHandler* poMysqlHandler, int iTableNum = 1 )
	{
		assert( pszTableName );
		StrCpy( m_szTableName, pszTableName, DATA_TABLE_NAME_SIZE );
		m_poMysqlHandler = poMysqlHandler;
		m_iTableNum = iTableNum;
		return true;
	}
	
protected:
	static const int DATA_TABLE_NAME_SIZE = 32;
	
	char m_szTableName[ DATA_TABLE_NAME_SIZE ]; // 名字不带后缀
	CMysqlHandler* m_poMysqlHandler;
	int m_iTableNum;
};

#endif

