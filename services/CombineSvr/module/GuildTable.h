#pragma once


#include <set>
#include <string>
#include "BaseTable.h"


using namespace std;
class GuildTable : public BaseTable
{
public:
	GuildTable();
	virtual ~GuildTable();

	virtual int Combine();
	virtual int Work();
	virtual bool Init(CMysqlHandler* poMysqlHandle, int iTableSvrId, const char* pstTableName, const char* pstTableColumn);

};
