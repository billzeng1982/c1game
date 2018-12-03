#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/ClusterAccountSvrCfgDesc.h"
#include "DBThread/DBWorkThread.h"

class ClusterAccountSvr : public CAppFrame, public TSingleton<ClusterAccountSvr>
{
public: 
	ClusterAccountSvr();
	virtual ~ClusterAccountSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();
	virtual int GetProcID(){ return m_stConfig.m_iProcID; }

public:
	CLUSTERACCOUNTSVRCFG& GetConfig(){ return m_stConfig; }
	CDBWorkThread* GetWorkThread(int iPos){ return &m_astWorkThreads[iPos]; } //调用者负责合法性检查

private:
	virtual void _SetAppCfg( );
	virtual bool _ReadCfg( );

private:
	CLUSTERACCOUNTSVRCFG m_stConfig;
	CDBWorkThread* 	m_astWorkThreads;
};

