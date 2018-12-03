#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/FightSvrCfgDesc.h"
#include "ss_proto.h"

class CFightSvr : public CAppFrame, public TSingleton<CFightSvr>
{
public:
	CFightSvr();
	virtual ~CFightSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual int GetProcID() { return m_stConfig.m_iProcID; }
	virtual void AppFini();

	FIGHTSVRCFG& GetConfig() { return m_stConfig; }

private:
	virtual void _SetAppCfg( );
	virtual bool _ReadCfg( );

	void _NotifyToCluster();

	void _InitGameObjPool();
	void _InitGameObjUpdator();

private:
	FIGHTSVRCFG m_stConfig;
	PKGMETA::SSPKG m_stSsPkg;
	uint32_t m_dwIntervalPass;
	uint64_t m_ullLastUptTime;
};


