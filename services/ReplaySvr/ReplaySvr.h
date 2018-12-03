#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/ReplaySvrCfgDesc.h"

class ReplaySvr: public CAppFrame, public TSingleton<ReplaySvr>
{
public:
	ReplaySvr() {}
	virtual ~ReplaySvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual int GetProcID() { return m_stConfig.m_iProcID; }
	virtual void AppFini();

	REPLAYSVRCFG & GetConfig() { return m_stConfig; }

private:
	virtual void _SetAppCfg();
	virtual bool _ReadCfg();

private:
	REPLAYSVRCFG m_stConfig;
};

