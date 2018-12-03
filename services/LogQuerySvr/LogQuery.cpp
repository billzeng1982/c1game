#include "LogQuerySvr.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "LogMacros.h"
#include "LogQuerySvrMsgLayer.h"
#include "PythonBinding.h"
#include "RoleTableMgr.h"
#include <curl/curl.h>

extern unsigned char g_szMetalib_LogQuerySvrCfg[];

LogQuerySvr::LogQuerySvr()
{

}

bool LogQuerySvr::AppInit()
{
    if (!LogQuerySvrMsgLayer::Instance().Init())
    {
        return false;
    }

	if (!PythonBinding::Instance().Init())
	{
		return false;
	}

	if (!RoleTableMgr::Instance().Init(&m_stConfig))
	{
		return false;
	}

    return true;
}

void LogQuerySvr::AppUpdate()
{
    bool bIdle = true;

    if (LogQuerySvrMsgLayer::Instance().DealPkg() > 0)
    {
        bIdle = false;
    }

    if (bIdle)
    {
        MsSleep(1);
    }
}

void LogQuerySvr::AppFini()
{
    CAppFrame::AppFini();
}

void LogQuerySvr::_SetAppCfg()
{
    m_stAppCfg.m_pszBuff = (char*)&m_stConfig;
    m_stAppCfg.m_dwLen = sizeof(m_stConfig);
    m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB)g_szMetalib_LogQuerySvrCfg;
    StrCpy( m_stAppCfg.m_szMetaName, "LogQuerySvrCfg", sizeof(m_stAppCfg.m_szMetaName) );
    snprintf( m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/LogQuerySvr.xml", CWorkDir::string());
}

bool LogQuerySvr::_ReadCfg()
{
    if( !CAppFrame::_ReadCfg() )
    {
        return false;
    }

    inet_aton( m_stConfig.m_szProcIDStr, (struct in_addr* )&m_stConfig.m_iProcID );
    inet_aton( m_stConfig.m_szIdipAgentIDStr, (struct in_addr* )&m_stConfig.m_iIdipAgentID );

    return true;
}


