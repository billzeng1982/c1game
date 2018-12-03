#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/AccountSvrCfgDesc.h"
#include "thread/DBWorkThread.h"

class AccountSvr : public CAppFrame, public TSingleton<AccountSvr>
{
public:
	AccountSvr();
	virtual ~AccountSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();
	virtual int GetProcID(){ return m_stConfig.m_iProcID; }

public:
	ACCOUNTSVRCFG& GetConfig(){ return m_stConfig; }
	CDBWorkThread* GetWorkThread(int iPos){ return &m_astWorkThreads[iPos]; } //调用者负责合法性检查

private:
	virtual void _SetAppCfg( );
	virtual bool _ReadCfg( );

private:
	ACCOUNTSVRCFG m_stConfig;
	CDBWorkThread* m_astWorkThreads;
};

