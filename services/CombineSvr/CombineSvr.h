#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/CombineSvrCfgDesc.h"
#include "thread/MysqlWorkThread.h"


class CombineSvr : public CAppFrame, public TSingleton<CombineSvr>
{
public:
	CombineSvr();
	virtual ~CombineSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual int GetProcID() { return 9999; }
	virtual void AppFini();

	COMBINESVRCFG & GetConfig() { return m_stConfig; }
	//RedisWorkThread* GetWorkThread(int iPos){ return &m_astWorkThreads[iPos]; } //调用者负责合法性检查

private:
	virtual void _SetAppCfg();
	virtual bool _ReadCfg();

private:
	COMBINESVRCFG m_stConfig;

};
