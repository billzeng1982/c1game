
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include "RedisSyncHandler.h"
#include "og_comm.h"
#include "strutil.h"
#include "LogMacros.h"
bool RedisSyncHandler::Init( const char* ip, int port, int iTimeoutMs )
{
    if (NULL == ip || ip[0] == '\0')
    {
        return false;
    }
    m_iTimeOutMs = iTimeoutMs;
    m_iPort = port;
    StrCpy(m_szIp, ip, sizeof(m_szIp));
    m_bAlive = false;
    return true;
}



bool RedisSyncHandler::_Connect( const char* ip, int port, int iTimeoutMs )
{
    assert( iTimeoutMs > 0 );

    struct timeval timeout;
    bzero( &timeout, sizeof(timeout) );
    TvAddMs(&timeout, iTimeoutMs);
    m_pContext = redisConnectWithTimeout( ip, port, timeout );

    if( !m_pContext )
    {
        LOGERR_r("Connection error: can't allocate redis context\n");
        return false;
    }

    if( m_pContext->err )
    {
        LOGERR_r("Connection error: %s\n", m_pContext->errstr);
        this->Close();
        return false;
    }
    m_bAlive = true;
    return true;
}

void RedisSyncHandler::Close()
{
    this->_FreeReply();
    if(m_pContext)
    {
        redisFree(m_pContext);
        m_pContext = NULL;
    }
    m_bAlive = false;
}

void RedisSyncHandler::_FreeReply()
{
    if( m_pReply )
    {
        freeReplyObject(m_pReply);
        m_pReply = NULL;
    }
}
bool RedisSyncHandler::IsAlive()
{
    return m_bAlive;
}

bool RedisSyncHandler::Reconnect()
{
    this->Close();
    return _Connect(m_szIp, m_iPort, m_iTimeOutMs);
}

redisReply* RedisSyncHandler::SyncCommand( const char *format, ... )
{
    assert( m_pContext );

    this->_FreeReply();

    va_list ap;
    va_start(ap,format);
    m_pReply = (redisReply*)redisvCommand(m_pContext,format,ap);
    va_end(ap);
    
    return m_pReply;
}
//	argvlen是每个 参数的有效长度,不包含 szStr 的'\0',
redisReply* RedisSyncHandler::SyncCommandArgv(int argc, const char **argv, const size_t *argvlen)
{
    m_pReply =  (redisReply*) redisCommandArgv(m_pContext ,argc, argv, argvlen);
    return m_pReply;
}

redisReply* RedisSyncHandler::SyncCommandArgv()
{
    m_pReply =  (redisReply*) redisCommandArgv(m_pContext ,m_iArgvPushCnt , (const char **)m_ppbArgvBuff, m_pArgvSize);
    return m_pReply;
}




void RedisSyncHandler::Ping()
{
    m_pReply = SyncCommand("ping");
    if (NULL == m_pReply)
    {
        LOGERR_r("Redis disconnect");
        m_bAlive = false;
        return;
    }
    if (m_pReply->type == REDIS_REPLY_STATUS && strcasecmp(m_pReply->str,"PONG") == 0 )
    {
        m_bAlive = true;
        return;
    }
    LOGERR_r("reids disconnect ,Type<%d>, %s", m_pReply->type, m_pReply->str);
    m_bAlive = false;
    return;
}



bool RedisSyncHandler::Connect()
{
    return _Connect( m_szIp,  m_iPort, m_iTimeOutMs );

}

void RedisSyncHandler::PushArgv(const char* pStr)
{
    size_t iSize = strlen(pStr);
    if (m_iArgvPushCnt >= m_iArgvMaxCnt || iSize > m_iMaxArgvSize)
    {
        LOGERR_r("ArgvCnt full Cur<%d>  Max<%d> or CurDataLen<%lu> MaxDataLen<%lu>", m_iArgvPushCnt, m_iArgvMaxCnt, iSize, m_iMaxArgvSize);
        return;
    }
    m_pArgvSize[m_iArgvPushCnt] = strlen(pStr);
    memcpy(m_ppbArgvBuff[m_iArgvPushCnt], pStr, m_pArgvSize[m_iArgvPushCnt]);
    m_iArgvPushCnt++;
}

void RedisSyncHandler::PushArgv(const char* pStr, size_t iSize)
{
    if (m_iArgvPushCnt >= m_iArgvMaxCnt || iSize > m_iMaxArgvSize)
    {
        LOGERR_r("ArgvCnt full CurCnt<%d>  MaxCnt<%d> or CurDataLen<%lu> MaxDataLen<%lu>", m_iArgvPushCnt, m_iArgvMaxCnt, iSize, m_iMaxArgvSize);
        return;
    }
    memcpy(m_ppbArgvBuff[m_iArgvPushCnt], pStr, iSize);
    m_pArgvSize[m_iArgvPushCnt] = iSize;
    m_iArgvPushCnt++;
}



void RedisSyncHandler::PushArgv(const uint64_t ullValue)
{
    size_t iSize = sizeof(ullValue);
    if (m_iArgvPushCnt >= m_iArgvMaxCnt || iSize > m_iMaxArgvSize)
    {
        LOGERR_r("ArgvCnt full Cur<%d>  Max<%d> or CurDataLen<%lu> MaxDataLen<%lu>", m_iArgvPushCnt, m_iArgvMaxCnt, iSize, m_iMaxArgvSize);
        return;
    }
    m_pArgvSize[m_iArgvPushCnt] = snprintf(m_ppbArgvBuff[m_iArgvPushCnt], m_iMaxArgvSize, "%lu", ullValue);
    m_iArgvPushCnt++;

}

bool RedisSyncHandler::ArgvInit(int iCount, size_t iMaxArgvSize)
{
    m_iArgvMaxCnt = iCount;
    m_iArgvPushCnt = 0;
    m_iMaxArgvSize = iMaxArgvSize;
    m_ppbArgvBuff = new char*[m_iArgvMaxCnt];
    for (int i = 0; i < m_iArgvMaxCnt; i++)
    {
        m_ppbArgvBuff[i] = new char[m_iMaxArgvSize];
        if (!m_ppbArgvBuff[i])
        {
            LOGERR_r("init m_ppbArgvBuff faild! <%d> ", m_iArgvMaxCnt);
            return false;
        }
    }
    m_pArgvSize = new size_t[m_iArgvMaxCnt];
    if (!m_pArgvSize)
    {
        LOGERR_r("init m_pdwArgc faild! <%d> ", m_iArgvMaxCnt);
        return false;
    }
    return true;
}

void RedisSyncHandler::ArgvReset()
{
    m_iArgvPushCnt = 0;
}

void RedisSyncHandler::ArgvRelease()
{
    if (m_ppbArgvBuff != NULL)
    {
        for (int i = 0; i < m_iArgvMaxCnt; i++)
        {
            if (NULL != m_ppbArgvBuff[i])
            {
                delete[] m_ppbArgvBuff[i];
                m_ppbArgvBuff[i] = NULL;
            }
        }
        delete[] m_ppbArgvBuff;
        m_ppbArgvBuff = NULL;
    }
    if (NULL != m_pArgvSize)
    {
        delete[] m_pArgvSize;
        m_pArgvSize = NULL;
    }
}


