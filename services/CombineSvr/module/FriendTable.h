#pragma once
#include <set>
#include <string>
#include "BaseTable.h"


using namespace std;

class FriendTable : public BaseTable
{
public:
	FriendTable();
	virtual ~FriendTable();
	virtual int Combine();
	virtual int Work();
	virtual bool Init(CMysqlHandler* poMysqlHandle, int iTableSvrId, const char* pstTableName, const char* pstTableColumn);
};

