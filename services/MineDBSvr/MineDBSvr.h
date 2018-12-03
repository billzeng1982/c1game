#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "./cfg/MineDBSvrCfgDesc.h"

class MineDBSvr : public CAppFrame, public TSingleton<MineDBSvr>
{
public:
	MineDBSvr() {}
	virtual ~MineDBSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();
	virtual int GetProcID(){ return m_stConfig.m_iProcID; }

public:
	MINEDBSVRCFG& GetConfig(){ return m_stConfig; }

private:
	virtual void _SetAppCfg( );
	virtual bool _ReadCfg( );

private:
    MINEDBSVRCFG m_stConfig;
};

