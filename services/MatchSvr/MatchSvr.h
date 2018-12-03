#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/MatchSvrCfgDesc.h"

class MatchSvr: public CAppFrame, public TSingleton<MatchSvr> {
public:
	MatchSvr() {}
	virtual ~MatchSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual int GetProcID() { return m_stConfig.m_iProcID; }
	virtual void AppFini();

	MATCHSVRCFG& GetConfig() { return m_stConfig; }

private:
	virtual void _SetAppCfg();
	virtual bool _ReadCfg();

private:
	MATCHSVRCFG m_stConfig;
};

