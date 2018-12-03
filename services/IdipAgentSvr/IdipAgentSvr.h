#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/IdipAgentSvrCfgDesc.h"
#include "thread/HttpClientThread.h"
#include "thread/HttpServerThread.h"
#include "logic/HttpReqMgr.h"
#include <set>
#include <map>

class IdipAgentSvr : public CAppFrame, public TSingleton<IdipAgentSvr>
{
public:
	IdipAgentSvr();
	virtual ~IdipAgentSvr();

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();
	virtual int GetProcID(){ return m_stConfig.m_iProcID; }

public:
    HttpClientThread* GetClientThread(int iPos) { return &m_astClientThreads[iPos]; }
    HttpServerThread* GetServerThread(int iPos) { return &m_astServerThreads[iPos]; }

public:
	IDIPAGENTSVRCFG& GetConfig(){ return m_stConfig; }

private:
    HttpClientThread* m_astClientThreads;
    HttpServerThread* m_astServerThreads;

private:
	virtual void _SetAppCfg( );
	virtual bool _ReadCfg( );

private:
	IDIPAGENTSVRCFG m_stConfig;

};

