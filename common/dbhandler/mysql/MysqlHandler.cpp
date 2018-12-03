#include "MysqlHandler.h"
#include "pal/tstring.h"
#include "comm_func.h"
#include "LogMacros.h"
#include "GameTime.h"
#include <string.h>
#include <stdarg.h>
#include <assert.h>

CMysqlHandler::CMysqlHandler()
{
    m_mysql = NULL;
    m_result = NULL;
    bzero( &m_stConnInfo, sizeof( m_stConnInfo ) );
    m_szSql = NULL;
    m_iSqlLen = 0;
    bzero( m_szErrMsg, sizeof(m_szErrMsg) );
    m_lLastPingTime = 0;
    m_bConnAlive = false;
    m_iPingFreq = 0;
}


CMysqlHandler::~CMysqlHandler()
{
    this->CloseConn();
    SAFE_DEL( m_szSql );
}

bool CMysqlHandler::ConnectDB( const char* pszDBAddr, int iPort, const char* pszDBName, 
	const char* pszUser, const char* pszPasswd, const char* pszCharacterSet, const char* pszUnixSocket )
{
    if( !pszDBAddr || !pszDBName || !pszUser || !pszPasswd || 0 == iPort )
    {
        return false;
    }

    m_szSql = new char[DB_SQL_STR_SIZE];
    if( !m_szSql )
    {
        return false;
    }

    m_mysql = mysql_init( NULL );
    if( !m_mysql )
    {
        return false;
    }
	if (pszCharacterSet == NULL)
	{
		STRNCPY(m_stConnInfo.m_szCharacterSet, "latin1", sizeof(m_stConnInfo.m_szCharacterSet));
	}
	else
	{
		STRNCPY(m_stConnInfo.m_szCharacterSet, pszCharacterSet, sizeof(m_stConnInfo.m_szCharacterSet));
	}
	//默认utf8连接
	mysql_options(m_mysql, MYSQL_SET_CHARSET_NAME, m_stConnInfo.m_szCharacterSet);//utf8

    STRNCPY( m_stConnInfo.m_szDBAddr, pszDBAddr, sizeof(m_stConnInfo.m_szDBAddr) );
    m_stConnInfo.m_iPort = iPort;
    STRNCPY( m_stConnInfo.m_szDBName, pszDBName, sizeof( m_stConnInfo.m_szDBName) );
    STRNCPY( m_stConnInfo.m_szUser, pszUser, sizeof(m_stConnInfo.m_szUser) );
    STRNCPY( m_stConnInfo.m_szPasswd, pszPasswd, sizeof(m_stConnInfo.m_szPasswd) );
    if( pszUnixSocket && pszUnixSocket[0] )
    {
        STRNCPY( m_stConnInfo.m_szUnixSocket, pszUnixSocket, sizeof(m_stConnInfo.m_szUnixSocket) );
    }else
    {
        m_stConnInfo.m_szUnixSocket[0] = '\0';
    }
    m_stConnInfo.m_ulFlag = CLIENT_FOUND_ROWS;

    if( !mysql_real_connect( m_mysql, pszDBAddr, pszUser, pszPasswd, pszDBName, iPort,
                               (m_stConnInfo.m_szUnixSocket[0] ? m_stConnInfo.m_szUnixSocket : NULL),
                               m_stConnInfo.m_ulFlag ) )
    {
        LOGERR_r("Failed to connect to database: Error: %s", mysql_error(m_mysql));
        return false;
    }

    m_bConnAlive = true;

    return true;
}


bool CMysqlHandler::ReconnectDB()
{
    // 先关掉旧的链接
    this->CloseConn();

    m_mysql = mysql_init( NULL );
    if( !m_mysql )
    {
        return false;
    }
	mysql_options(m_mysql, MYSQL_SET_CHARSET_NAME, m_stConnInfo.m_szCharacterSet);//utf8
    char *psUnixSockFile = NULL;
    if( m_stConnInfo.m_szUnixSocket[0] )
    {
        psUnixSockFile = m_stConnInfo.m_szUnixSocket;
    }

    if( !mysql_real_connect( m_mysql,
                             m_stConnInfo.m_szDBAddr,
                             m_stConnInfo.m_szUser,
                             m_stConnInfo.m_szPasswd,
                             m_stConnInfo.m_szDBName,
                             m_stConnInfo.m_iPort,
                             psUnixSockFile,
                             m_stConnInfo.m_ulFlag ) )
    {
        LOGERR_r("Failed to connect to database: Error: %s", mysql_error(m_mysql));
        return false;
    }

    m_bConnAlive = true;

    return true;
}


void CMysqlHandler::Ping()
{
    if ( mysql_ping( m_mysql ) != 0 )
    {
        m_bConnAlive = false;
    }

    return;
}


void CMysqlHandler::CloseConn()
{
    if( !m_mysql )
    {
        return;
    }

    this->FreeResult();

    mysql_close( m_mysql );
    m_mysql = NULL;

    m_bConnAlive = false;
}


int CMysqlHandler::Execute( const char* pszSql )
{
    if( !pszSql || '\0' == pszSql[0] )
    {
        assert( false );
        return -1;
    }

    int iSqlLen = strnlen( pszSql, DB_SQL_STR_SIZE - 1 );
    if( 0 != mysql_real_query( m_mysql, pszSql, iSqlLen ) )
    {
        // 连接出错，重连后再执行一次
        ReconnectDB();
        if( 0 != mysql_real_query( m_mysql, pszSql, iSqlLen ) )
        {
            snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Error:%s", mysql_error(m_mysql) );
            this->_CheckConnAlive();
            return -1;
        }
    }

    return 0;
}


int CMysqlHandler::Execute()
{
    assert( m_iSqlLen > 0 );

    if( 0 != mysql_real_query( m_mysql, m_szSql, m_iSqlLen ) )
    {
        // 连接出错，重连后再执行一次
        ReconnectDB();
        if( 0 != mysql_real_query( m_mysql, m_szSql, m_iSqlLen ) )
        {
            snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Error:%s", mysql_error(m_mysql) );
            this->_CheckConnAlive();
            return -1;
        }
    }

    return 0;
}


// select之后, store result, 返回row num
int CMysqlHandler::StoreResult()
{
    if( m_result )
    {
        // query 打开后没有关闭
        this->FreeResult();
    }

    m_result = mysql_store_result( m_mysql );
    if( !m_result )
    {
        return -1;
    }

    return (int)mysql_num_rows(m_result);
}


void CMysqlHandler::FreeResult()
{
    if( !m_result )
    {
        return;
    }

    m_oRowIter.Reset();
    mysql_free_result( m_result );
    m_result = NULL;
}


// 注意BufSize要足够大，You must allocate the to buffer to be at least length*2+1 bytes long
bool CMysqlHandler::BinToStr( const char* pszBin, int iBinSize, char* pszBuf, int iBufSize )
{
    if( !pszBin || !pszBuf )
    {
        return false;
    }

    if( iBufSize < iBinSize*2 + 1 )
    {
        return false;
    }

    int iLen = mysql_real_escape_string( m_mysql, pszBuf, pszBin, iBinSize );
    if( iLen >= iBufSize )
    {
        return false;
    }

    return true;
}


bool CMysqlHandler::FormatSql(const char szSql[], ... )
{
    va_list ap;
    va_start( ap, szSql );
    m_iSqlLen = vsnprintf( m_szSql, DB_SQL_STR_SIZE, szSql, ap );
    va_end(ap);

    m_szSql[DB_SQL_STR_SIZE-1] = '\0';

    if( m_iSqlLen >= DB_SQL_STR_SIZE )
    {
        return false;
    }

    return true;
}


bool CMysqlHandler::IsTableExist(const char* pszTableName )
{
    if( !pszTableName || '\0' == pszTableName[0] )
    {
        assert( false );
        return false;
    }

    // 必须再查询前要清空
    this->FreeResult();

    m_result = mysql_list_tables( m_mysql, pszTableName );
    if( !m_result )
    {
        return false;
    }

    // move to first row
    m_oRowIter.Begin( m_result );

    bool bExist = false;
    int iRow = mysql_num_rows( m_result );
    int iNameLen = strlen( pszTableName );
    for( int i = 0; i < iRow; i++ )
    {
        if( 0 == strncmp( pszTableName, m_oRowIter.GetField(i)->GetString(), iNameLen ) )
        {
                bExist = true;
                break;
        }
    }

    this->FreeResult();
    return bExist;
}


void CMysqlHandler::_CheckConnAlive()
{
    if( NULL == m_mysql )
    {
        m_bConnAlive = false;
        return;
    }

    int iErrNo =  mysql_errno( m_mysql ) ;
    if ( CR_SERVER_GONE_ERROR == iErrNo || CR_SERVER_LOST == iErrNo )
    {
        m_bConnAlive = false;
    }

    return;
}

int CMysqlHandler::CheckConn()
{
    if( 0 == m_iPingFreq )
        return 0;

     // check alive
    if ( !this->IsConnAlive() )
    {
        if ( !this->ReconnectDB() )
        {
            return -1; //change to idle
        }
    }
    
    time_t lCurTime = CGameTime::Instance().GetCurrSecond();
    if (lCurTime - this->GetLastPingTime() >= m_iPingFreq)
    {
        // ping mysql
        this->Ping();
        this->SetLastPingTime(lCurTime);
    }

    return 1;
}

