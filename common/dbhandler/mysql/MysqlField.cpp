#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "MysqlField.h"

CMysqlField::CMysqlField()
{
    m_data = NULL;
    m_field = NULL;
    m_length = 0;
}

void CMysqlField::AttachField( MYSQL_FIELD* field, char* data, unsigned long length )
{
    if( !field )
    {
        assert( false );
        return;
    }
	//这里不考虑data的有效性，对Field的每种类型取值时在做相应的处理
    m_field = field;
    m_data = data;
    m_length = length;
}

void CMysqlField::DetachField()
{
    m_data = NULL;
    m_field = NULL;
    m_length = 0;
}

// int, short, char, etc
unsigned long CMysqlField::GetInteger()
{
    assert( IS_NUM( m_field->type ) );
    return ( m_data ? strtoul( m_data, (char**)NULL, 10 ) : 0 );
}


unsigned long long CMysqlField::GetBigInteger()
{
    assert( FIELD_TYPE_LONGLONG == m_field->type );
    return ( m_data ? strtoull( m_data, (char**)NULL, 10 ) : 0 );
}


char* CMysqlField::GetString()
{
    assert( FIELD_TYPE_STRING     == m_field->type ||
		    FIELD_TYPE_VAR_STRING == m_field->type ||
		    //FIELD_TYPE_BLOB       == m_field->type ||
		    FIELD_TYPE_DATETIME   == m_field->type );

    static char szVoidString[2];
    szVoidString[0] = '\0';

    return ( m_data ? m_data : szVoidString );
}


int CMysqlField::GetBinData( char* pszBuff, uint32_t dwBufSize )
{
    assert( FIELD_TYPE_BLOB == m_field->type );

    if( dwBufSize < m_length )
    {
        return -1;
    }

    if( !m_data || 0 == m_length )
    {
        return 0;
    }

    memcpy( pszBuff, m_data, m_length );

    return (int)m_length;
}


/* convert mysql date-time to time_t
   NOTE: the date-time string format must be: yyyy-mm-dd hh:mm:ss
*/
time_t CMysqlField::GetTime()
{
    assert( FIELD_TYPE_DATETIME == m_field->type );
	if (!m_data)
	{
		return 0;
	}
    struct tm tmTime;
    time_t lTime;

    strptime( m_data, "%Y-%m-%d %H:%M:%S", &tmTime );

    lTime =mktime(&tmTime);
    return lTime;
}

void CMysqlField::Reset()
{
    m_field = NULL;
    m_data= NULL;
    m_length = 0;
}

