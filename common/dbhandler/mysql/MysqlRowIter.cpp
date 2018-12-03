#include <assert.h>
#include "MysqlRowIter.h"

CMysqlRowIter::CMysqlRowIter()
{
    m_row = NULL;
    m_lengths = NULL;
    m_result = NULL;
}

void CMysqlRowIter::Begin( MYSQL_RES* result )
{
    m_row = NULL;
    
    if( !result )
    {
        assert( false );
        return;
    }

    m_result = result;
    m_row = mysql_fetch_row( m_result );
    if( m_row )
    {
        m_lengths = mysql_fetch_lengths( m_result ); // Returns the lengths of the columns of the current row within a result set
    }else
    {
        m_lengths = NULL;
    }
}


void CMysqlRowIter::Next()
{
    m_row = mysql_fetch_row( m_result );
    if( m_row )
    {
        m_lengths = mysql_fetch_lengths( m_result );
    }else
    {
        m_lengths = NULL;
    }
}


CMysqlField* CMysqlRowIter::GetField( int iIndex )
{
    if( NULL == m_row )
    {
        assert( false );
        return NULL;
    }

    if( (unsigned int)iIndex >= mysql_num_fields( m_result ) )
    {
        assert( false );
        return NULL;
    }

    MYSQL_FIELD *field = mysql_fetch_field_direct( m_result, iIndex );
    if( NULL == field )
    {
        return NULL;
    }

    m_oField.AttachField( field, m_row[iIndex], m_lengths[iIndex]);

    return &m_oField;
}


unsigned long CMysqlRowIter::GetFieldLength(int iIndex)
{
	if (!m_lengths)
	{
		return 0;
	}
	return m_lengths[iIndex];
}

void CMysqlRowIter::Reset()
{
    m_row = NULL;
    m_lengths = NULL;
    m_result = NULL;

    m_oField.Reset();
}

