#pragma once

#include "define.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "LogMacros.h"
#include "redis/RedisSyncHandler.h"
#include <time.h>
using namespace PKGMETA;

#define REDIS_SERIAL_NUM_TABLE_NAME  "Serial"

class SerialNumTable
{
public:
	SerialNumTable();
	~SerialNumTable();

	bool Init(RedisSyncHandler* poRedisHandler) ;

	//检查序列号是否存在，1-存在，0-不存在
	int CheckSerial(uint32_t dwSerialId, char* pszSerial);

	//删除序列号
	int DelSerial(uint32_t dwSerialId, char* pszSerial);

private:
	RedisSyncHandler* m_poRedisHandler;
	redisReply* m_poReply;

private:
	DT_SERIAL_NUM_BLOB m_stSerialNumBlob;
};

