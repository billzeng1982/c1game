#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/AsyncPvpSvrCfgDesc.h"

class AsyncPvpSvr : public CAppFrame, public TSingleton<AsyncPvpSvr>
{
public:
	AsyncPvpSvr();
	virtual ~AsyncPvpSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();
	virtual int GetProcID(){ return m_stConfig.m_iProcID; }
    ASYNCPVPSVRCFG& GetConfig() {return m_stConfig; }

private:
	virtual void _SetAppCfg();
	virtual bool _ReadCfg();

private:
    ASYNCPVPSVRCFG m_stConfig;
};

