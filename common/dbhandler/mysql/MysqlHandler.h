#ifndef _MYSQL_HANDLER_H_
#define _MYSQL_HANDLER_H_

#include <mysql.h>
#include <errmsg.h>
#include "MysqlRowIter.h"
#include "pal/ttypes.h"
#include "DBHandlerDef.h"


class CMysqlHandler
{
public:
	CMysqlHandler();
	~CMysqlHandler();

	bool ConnectDB(const char* pszDBAddr, int iPort, const char* pszDBName, const char* pszUser, const char* pszPasswd, 
		const char* pszCharacterSet = NULL, const char* pszUnixSocket = NULL );
	bool ReconnectDB();
	void Ping();
	void CloseConn(); // 关闭联接

	// 返回: 0 : success, <0 : error
	int Execute(const char* pszSql );
	int Execute();

	// “-1”表示查询返回错误，或者，对于SELECT查询，在调用mysql_store_result()之前调用了mysql_affected_rows()
	int AffectedRows() { return (int)mysql_affected_rows( m_mysql ); }

	// select之后, store result, 返回row num
	int StoreResult();
	void FreeResult();

	CMysqlRowIter& GetRowIter() { return m_oRowIter; }

	bool BinToStr( const char* pszBin, int iBinSize, char* pszBuf, int iBufSize );

	bool FormatSql( const char szSql[], ... );

	bool IsTableExist( const char* pszTableName );

	// Returns the value generated for an AUTO_INCREMENT column by the previous INSERT or UPDATE statement.
	uint64_t GetInsertID() { return mysql_insert_id( m_mysql ); }

	MYSQL_RES* GetResult() { return m_result; }

	char* GetLastErrMsg() { return m_szErrMsg; }

	char* GetSql() { return m_szSql; }

	void SetLastPingTime( time_t lTime ) { m_lLastPingTime = lTime; }
	time_t GetLastPingTime() { return m_lLastPingTime; }

	bool IsConnAlive() { return m_bConnAlive; }

	void SetPingFreq( int iPingFreq ) { m_iPingFreq = iPingFreq; }
	int CheckConn();

private:
	void _CheckConnAlive();

private:
	MYSQL* m_mysql;
	MYSQL_RES* m_result;
	CMysqlRowIter m_oRowIter;

	struct
	{
		char m_szUser[ DB_USER_LEN ];
		char m_szPasswd[ DB_PASSWD_LEN ];
		char m_szDBName[ DB_NAME_LEN  ];
		char m_szDBAddr[ DB_ADDR_LEN ];
		int  m_iPort;
		char m_szUnixSocket[ DB_UNIX_SOCKECT_LEN ];
		unsigned long m_ulFlag;
		char m_szCharacterSet[DB_CHARACTER_SET_LEN];
	}m_stConnInfo;  // 连接信息

	int m_iSqlLen;
	char* m_szSql;
	char m_szErrMsg[10240];

	time_t m_lLastPingTime;
	bool m_bConnAlive; 	// 链接活跃

	int m_iPingFreq;
};


#endif


