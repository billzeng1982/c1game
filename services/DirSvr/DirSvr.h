#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/DirSvrCfgDesc.h"

class DirSvr: public CAppFrame, public TSingleton<DirSvr>
{
public:
	DirSvr() {}
	virtual ~DirSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual int GetProcID() { return m_stConfig.m_iProcID; }
	virtual void AppFini();

	//重载数据
	virtual bool ReloadGamedata();

	DIRSVRCFG& GetConfig() { return m_stConfig; }

private:
	virtual void _SetAppCfg();
	virtual bool _ReadCfg();

private:
	DIRSVRCFG m_stConfig;
};

