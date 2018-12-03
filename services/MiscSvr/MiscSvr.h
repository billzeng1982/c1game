#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/MiscSvrCfgDesc.h"

class MiscSvr : public CAppFrame, public TSingleton<MiscSvr>
{
public:
	MiscSvr() {}
	virtual ~MiscSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();
	virtual int GetProcID(){ return m_stConfig.m_iProcID; }

public:
	MISCSVRCFG& GetConfig(){ return m_stConfig; }

private:
	virtual void _SetAppCfg( );
	virtual bool _ReadCfg( );

private:
	MISCSVRCFG m_stConfig;
};

