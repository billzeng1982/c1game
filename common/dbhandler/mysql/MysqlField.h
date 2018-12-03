#ifndef _MYSQL_FIELD_H_
#define _MYSQL_FIELD_H_

// 列迭代

#include <mysql.h>
#include <time.h>
#include "pal/ttypes.h"

class CMysqlField
{
public:
	CMysqlField();
	~CMysqlField(){}

	void AttachField( MYSQL_FIELD* field, char* data, unsigned long length );
	void DetachField();

	// int, short, char, etc
	unsigned long GetInteger();
	unsigned long long GetBigInteger();
	char* GetString();
	int GetBinData( char* pszBuff, uint32_t dwBufSize );
	char* GetBinDataPoint() { return m_data;	};
	int GetBinDataSize() { return m_data == NULL?  0 : m_length; };
	time_t GetTime();

	void Reset();

private:
	MYSQL_FIELD* m_field;
	char* m_data;
	unsigned long m_length;
};


#endif

