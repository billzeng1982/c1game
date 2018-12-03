#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/RankSvrCfgDesc.h"

class RankSvr : public CAppFrame, public TSingleton<RankSvr>
{
public:
	RankSvr();
	virtual ~RankSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();
	virtual int GetProcID(){ return m_stConfig.m_iProcID; }

public:
	RANKSVRCFG& GetConfig(){ return m_stConfig; }

private:
	virtual void _SetAppCfg( );
	virtual bool _ReadCfg( );

private:
	RANKSVRCFG m_stConfig;
};

