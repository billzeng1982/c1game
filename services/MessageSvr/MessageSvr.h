#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/MessageSvrCfgDesc.h"
#include "./module/MessageInfo/MessageTable.h"

class MessageSvr: public CAppFrame, public TSingleton<MessageSvr>
{
public:
    MessageSvr();
    virtual ~MessageSvr() {}

    virtual bool AppInit();
    virtual void AppUpdate();
    virtual int GetProcID() { return m_stConfig.m_iProcID; }
    virtual void AppFini();

    MESSAGESVRCFG & GetConfig() { return m_stConfig; }

private:
    virtual void _SetAppCfg();
    virtual bool _ReadCfg();

private:
    MESSAGESVRCFG m_stConfig;
};

