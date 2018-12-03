#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/CloneBattleSvrCfgDesc.h"

class CloneBattleSvr : public CAppFrame, public TSingleton<CloneBattleSvr>
{
public:
    CloneBattleSvr() {};
	virtual ~CloneBattleSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();
	virtual int GetProcID(){ return m_stConfig.m_iProcID; }

public:
	CLONEBATTLESVRCFG& GetConfig(){ return m_stConfig; }

private:
	virtual void _SetAppCfg( );
	virtual bool _ReadCfg( );

private:
    CLONEBATTLESVRCFG m_stConfig;
};

