#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "./cfg/ClusterDBSvrCfgDesc.h"

class ClusterDBSvr : public CAppFrame, public TSingleton<ClusterDBSvr>
{
public:
	ClusterDBSvr() {}
	virtual ~ClusterDBSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();
	virtual int GetProcID(){ return m_stConfig.m_iProcID; }

public:
	CLUSTERDBSVRCFG& GetConfig(){ return m_stConfig; }

private:
	virtual void _SetAppCfg( );
	virtual bool _ReadCfg( );

private:
	CLUSTERDBSVRCFG m_stConfig;
};

