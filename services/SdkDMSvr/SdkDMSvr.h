#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/SdkDMSvrCfgDesc.h"


class SdkDMSvr : public CAppFrame, public TSingleton<SdkDMSvr>
{
public:
	SdkDMSvr();
	virtual ~SdkDMSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();
	virtual int GetProcID(){ return m_stConfig.m_iProcID; }

public:
	SDKDMSVRCFG& GetConfig(){ return m_stConfig; }
	//CDBWorkThread* GetWorkThread(int iPos){ return &m_astWorkThreads[iPos]; } //调用者负责合法性检查

private:
	virtual void _SetAppCfg( );
	virtual bool _ReadCfg( );

private:
	SDKDMSVRCFG m_stConfig;
	//CDBWorkThread* m_astWorkThreads;
};

