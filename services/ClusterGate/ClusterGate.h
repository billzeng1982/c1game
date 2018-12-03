#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/ClusterGateCfgDesc.h"

class ClusterGate: public CAppFrame, public TSingleton<ClusterGate>
{
public:
	ClusterGate() {}
	virtual ~ClusterGate() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual int GetProcID() { return m_stConfig.m_iProcID; }
	virtual void AppFini();

	CLUSTERGATECFG & GetConfig() { return m_stConfig; }
	virtual bool ReloadTBusChannel();
	
private:
	virtual void _SetAppCfg();
	virtual bool _ReadCfg();

private:
	CLUSTERGATECFG m_stConfig;
};

