#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/GuildSvrCfgDesc.h"

class GuildSvr: public CAppFrame, public TSingleton<GuildSvr>
{
public:
    GuildSvr();
    virtual ~GuildSvr() {}

    virtual bool AppInit();
    virtual void AppUpdate();
    virtual int GetProcID() { return m_stConfig.m_iProcID; }
    virtual void AppFini();

    GUILDSVRCFG & GetConfig() { return m_stConfig; }

private:
    virtual void _SetAppCfg();
    virtual bool _ReadCfg();

private:
    GUILDSVRCFG m_stConfig;
};

