#ifndef _MYSQL_ROW_ITER_H_
#define _MYSQL_ROW_ITER_H_

// typedef char **MYSQL_ROW

#include <mysql.h>
#include "MysqlField.h"

class CMysqlRowIter
{
public:
	CMysqlRowIter();
	~CMysqlRowIter(){}

	void Begin( MYSQL_RES* result );
	
	void Next();
	
	bool IsEnd() { return ( NULL == m_row ); }
	
	MYSQL_ROW GetCurRow() { return m_row; }
	unsigned long* GetCurLengths() { return m_lengths; }

	CMysqlField* GetField( int iIndex );
	unsigned long GetFieldLength(int iIndex);
	void Reset();

private:
	MYSQL_ROW m_row; 			// 指示当前行
	unsigned long* m_lengths;
	MYSQL_RES* m_result;
	CMysqlField m_oField;
};

#endif

