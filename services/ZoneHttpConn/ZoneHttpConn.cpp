#include "ZoneHttpConn.h"
#include "framework/ZoneHttpConnMsgLayer.h"
#include "utils/oi_misc.h"
#include "utils/strutil.h"
#include "utils/workdir.h"
#include <curl/curl.h>

extern unsigned char g_szMetalib_ZoneHttpConnCfg[];

ZoneHttpConn::ZoneHttpConn()
{
}

ZoneHttpConn::~ZoneHttpConn()
{
}

bool ZoneHttpConn::AppInit()
{
    if (!ZoneHttpConnMsgLayer::Instance().Init())
    {
        return false;
    }

    return true;
}

void ZoneHttpConn::AppUpdate()
{
    bool bIdle = true;

    if (ZoneHttpConnMsgLayer::Instance().DealPkg() > 0)
    {
        bIdle = false;
    }

    if (bIdle)
    {
        MsSleep(1);
    }
}

void ZoneHttpConn::AppFini()
{
    CAppFrame::AppFini();
}

void ZoneHttpConn::_SetAppCfg()
{
    m_stAppCfg.m_pszBuff = (char*)&m_stConfig;
    m_stAppCfg.m_dwLen = sizeof(ZONEHTTPCONNCFG);
    m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB)g_szMetalib_ZoneHttpConnCfg;
    StrCpy(m_stAppCfg.m_szMetaName, "ZoneHttpConnCfg", sizeof(m_stAppCfg.m_szMetaName));
    snprintf(m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/ZoneHttpConn.xml", CWorkDir::string());
}

bool ZoneHttpConn::_ReadCfg()
{
    if (!CAppFrame::_ReadCfg())
    {
        return false;
    }

    inet_aton(m_stConfig.m_szProcIDStr, (struct in_addr*)&m_stConfig.m_iProcID);
    inet_aton(m_stConfig.m_szZoneSvrIDStr, (struct in_addr*)&m_stConfig.m_iZoneSvrID);
    return true;
}
