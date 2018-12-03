#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/SerialNumSvrCfgDesc.h"
#include "./logic/SerialNumTable.h"
#include "redis/RedisSyncHandler.h"
#include "thread/RedisWorkThread.h"

class SerialNumSvr: public CAppFrame, public TSingleton<SerialNumSvr>
{
public:
    SerialNumSvr();
    virtual ~SerialNumSvr() {}

    virtual bool AppInit();
    virtual void AppUpdate();
    virtual int GetProcID() { return m_stConfig.m_iProcID; }
    virtual void AppFini();

    SERIALNUMSVRCFG & GetConfig() { return m_stConfig; }
	RedisWorkThread* GetWorkThread(int iPos){ return &m_astWorkThreads[iPos]; } //调用者负责合法性检查

private:
    virtual void _SetAppCfg();
    virtual bool _ReadCfg();

private:
    SERIALNUMSVRCFG m_stConfig;
	RedisWorkThread* m_astWorkThreads;
};

