#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "LogQuerySvrCfgDesc.h"

class LogQuerySvr : public CAppFrame, public TSingleton<LogQuerySvr>
{
public:
	LogQuerySvr();
	virtual ~LogQuerySvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();
	virtual int GetProcID(){ return m_stConfig.m_iProcID; }

public:
	LOGQUERYSVRCFG& GetConfig(){ return m_stConfig; }

private:
	virtual void _SetAppCfg( );
	virtual bool _ReadCfg( );

private:
	LOGQUERYSVRCFG m_stConfig;
};

