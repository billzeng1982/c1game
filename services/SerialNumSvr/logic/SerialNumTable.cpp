#include "SerialNumTable.h"
#include "BitMap.h"

SerialNumTable::SerialNumTable()
{

}

SerialNumTable::~SerialNumTable()
{

}

bool SerialNumTable::Init( RedisSyncHandler* poRedisHandler)
{
	m_poRedisHandler = poRedisHandler;
	return true;
}

int SerialNumTable::CheckSerial(uint32_t dwSerialId, char* pszSerial)
{
    m_poReply = m_poRedisHandler->SyncCommand("SISMEMBER %s_%u %s", REDIS_SERIAL_NUM_TABLE_NAME, dwSerialId, pszSerial);
    if (NULL == m_poReply || REDIS_REPLY_INTEGER  != m_poReply->type)
    {
    	LOGERR_r("Reply is null ");
		return ERR_REDIS_ERROR;
    }

    return m_poReply->integer;
}

int SerialNumTable::DelSerial(uint32_t dwSerialId, char* pszSerial)
{
    m_poReply = m_poRedisHandler->SyncCommand("SREM %s_%u %s", REDIS_SERIAL_NUM_TABLE_NAME, dwSerialId, pszSerial);
    if (NULL == m_poReply || REDIS_REPLY_INTEGER  != m_poReply->type)
    {
    	LOGERR_r("Reply is null ");
		return ERR_REDIS_ERROR;
    }

    return m_poReply->integer;
}

