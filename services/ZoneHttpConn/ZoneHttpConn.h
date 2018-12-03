#pragma once

#ifndef ZONE_HTTP_CONN_H_
#define ZONE_HTTP_CONN_H_

#include "sys/AppFrame.h"
#include "utils/singleton.h"
#include "cfg/ZoneHttpConnCfgDesc.h"

class ZoneHttpConn : public CAppFrame, public TSingleton<ZoneHttpConn>
{
public:
    ZoneHttpConn();

    ~ZoneHttpConn();

    bool AppInit();
    void AppUpdate();
    void AppFini();
    int GetProcID() { return m_stConfig.m_iProcID; }

    inline ZONEHTTPCONNCFG& GetConfig() { return m_stConfig; }

private:
    void _SetAppCfg();
    bool _ReadCfg();

private:
    ZONEHTTPCONNCFG m_stConfig;
};

#endif
