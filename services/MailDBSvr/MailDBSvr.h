#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/MailDBSvrCfgDesc.h"
#include "thread/DBWorkThread.h"

class MailDBSvr : public CAppFrame, public TSingleton<MailDBSvr>
{
public:
    MailDBSvr();
    virtual ~MailDBSvr() {}
     
    virtual bool AppInit();
    virtual void AppUpdate();
    virtual void AppFini();
    virtual int GetProcID(){ return m_stConfig.m_iProcID; }

public:
    MAILDBSVRCFG& GetConfig(){ return m_stConfig; }
    CDBWorkThread* GetWorkThread(int iPos){ return &m_astWorkThreads[iPos]; } //调用者负责合法性检查

private:
    virtual void _SetAppCfg( );
    virtual bool _ReadCfg( );

private:
    MAILDBSVRCFG m_stConfig;
    CDBWorkThread* m_astWorkThreads;
};

