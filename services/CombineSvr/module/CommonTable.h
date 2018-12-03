#pragma once

#include "define.h"
#include "BaseTable.h"

/**
	通用表操作, 没有特殊处理
*/

class CommonTable: public BaseTable
{
public:
	CommonTable() {}
	virtual ~CommonTable() {}
	virtual int Combine();
	virtual int Work();
};




