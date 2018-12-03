#pragma once

/*
	同步接口包装
*/


#include <vector>
#include <string>
#include <stdint.h>
#include "hiredis.h"

class RedisSyncHandler
{
public:
	const static int DEFAULT_CONNECT_TIMEOUT_MS = 2000;
	
	RedisSyncHandler() : m_pContext(NULL), m_pReply(NULL)
	{
	}

	~RedisSyncHandler()
	{
		this->Close();
        this->ArgvRelease();
	}

    void Close();
    bool Init( const char* ip, int port, int iTimeoutMs = DEFAULT_CONNECT_TIMEOUT_MS );
    bool ArgvInit(int iCount, size_t iMaxArgvSize);
    void ArgvReset();
    void ArgvRelease();
    bool Connect();
    bool Reconnect();
    bool IsAlive();
    void Ping();
	// When an error occurs, the return value is `NULL`
	redisReply* SyncCommand( const char *format, ... );
    redisReply* SyncCommandArgv(int argc, const char **argv, const size_t *argvlen); //主要用来同步多个二进制数据
    redisReply* SyncCommandArgv();
    void PushArgv(const char* ptr);
    void PushArgv(const char* ptr, size_t size);
    void PushArgv(const uint64_t ullValue);
private:
	// 执行command后,reply需要用freeReplyObject()释放, 否则内存泄漏!!
	void _FreeReply();
    bool _Connect( const char* ip, int port, int iTimeoutMs );
private:
	redisContext* 	m_pContext;
	redisReply* 	m_pReply;
    char m_szIp[30];
    int m_iPort;
    int m_iTimeOutMs;
    bool m_bAlive;

    char** m_ppbArgvBuff;   //参数Buff
    size_t m_iMaxArgvSize;      //单个参数最大长度;
    size_t* m_pArgvSize;    //记录每个参数的长度大小
    int m_iArgvPushCnt;		//已使用参数个数
    int m_iArgvMaxCnt;      //参数最大个数
    
 
};

