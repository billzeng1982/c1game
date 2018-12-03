#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "./cfg/MineSvrCfgDesc.h"

class MineSvr : public CAppFrame, public TSingleton<MineSvr>
{
public:
    MineSvr() {};
	virtual ~MineSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();
	virtual int GetProcID(){ return m_stConfig.m_iProcID; }

public:
	MINESVRCFG& GetConfig(){ return m_stConfig; }

private:
	virtual void _SetAppCfg( );
	virtual bool _ReadCfg( );

private:
    MINESVRCFG m_stConfig;
};

