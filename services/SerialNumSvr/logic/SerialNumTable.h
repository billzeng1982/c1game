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

	//������к��Ƿ���ڣ�1-���ڣ�0-������
	int CheckSerial(uint32_t dwSerialId, char* pszSerial);

	//ɾ�����к�
	int DelSerial(uint32_t dwSerialId, char* pszSerial);

private:
	RedisSyncHandler* m_poRedisHandler;
	redisReply* m_poReply;

private:
	DT_SERIAL_NUM_BLOB m_stSerialNumBlob;
};

