#include "ReplaySvr.h"
#include "workdir.h"
#include "oi_misc.h"
#include "strutil.h"
#include "framework/ReplaySvrMsgLayer.h"
#include "module/ReplayMgr.h"

extern unsigned char g_szMetalib_ReplaySvrCfg[];

bool ReplaySvr::AppInit()
{
	if (!ReplaySvrMsgLayer::Instance().Init())
	{
		return false;
	}
    if (!ReplayMgr::Instance().Init(m_stConfig.m_szURLRoot, m_stConfig.m_szRootDir,
                             m_stConfig.m_iCheckInterval, m_stConfig.m_iUpdateInterval, "ReplayList"))
	{
		return false;
	}

	return true;
}

void ReplaySvr::AppFini()
{
    ReplayMgr::Instance().AppFini();
	CAppFrame::AppFini();
}

void ReplaySvr::AppUpdate()
{
	bool bIdle = true;

	// deal pkgs
	if (ReplaySvrMsgLayer::Instance().DealPkg() > 0)
    {
		bIdle = false;
	}

    //update mgr
    ReplayMgr::Instance().Update();

	if (bIdle)
    {
        MsSleep(1);
    }
}

void ReplaySvr::_SetAppCfg()
{
	m_stAppCfg.m_pszBuff = (char*) &m_stConfig;
	m_stAppCfg.m_dwLen = sizeof(m_stConfig);
	m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB) g_szMetalib_ReplaySvrCfg;
	StrCpy(m_stAppCfg.m_szMetaName, "ReplaySvrCfg", sizeof(m_stAppCfg.m_szMetaName));
	snprintf(m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/ReplaySvr.xml", CWorkDir::string());
}

bool ReplaySvr::_ReadCfg()
{
	if (!CAppFrame::_ReadCfg())
    {
		return false;
	}

	inet_aton(m_stConfig.m_szProcIDStr, (struct in_addr*) &m_stConfig.m_iProcID);

	return true;
}

