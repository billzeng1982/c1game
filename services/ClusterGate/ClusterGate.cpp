#include "ClusterGate.h"
#include "workdir.h"
#include "oi_misc.h"
#include "strutil.h"
#include "framework/ClusterGateMsgLayer.h"
#include "module/ServOnlineMgr.h"

extern unsigned char g_szMetalib_ClusterGateCfg[];

bool ClusterGate::AppInit()
{
	if (!ClusterGateMsgLayer::Instance().Init())
	{
		return false;
	}
    if (!ServOnlineMgr::Instance().Init())
	{
		return false;
	}

	return true;
}

void ClusterGate::AppFini()
{
	CAppFrame::AppFini();
}

void ClusterGate::AppUpdate()
{
	bool bIdle = true;

	// deal pkgs
	if (ClusterGateMsgLayer::Instance().DealPkg() > 0)
    {
		bIdle = false;
	}

	if (bIdle)
    {
        MsSleep(1);
    }
}

bool ClusterGate::ReloadTBusChannel()
{
    bool bRet = ClusterGateMsgLayer::Instance().RefreshBusChannel();
    if (!bRet)
    {
        LOGERR("reload tbus channel failed.");
        return false;
    }

    return true;
}

void ClusterGate::_SetAppCfg()
{
	m_stAppCfg.m_pszBuff = (char*) &m_stConfig;
	m_stAppCfg.m_dwLen = sizeof(m_stConfig);
	m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB) g_szMetalib_ClusterGateCfg;
	StrCpy(m_stAppCfg.m_szMetaName, "ClusterGateCfg", sizeof(m_stAppCfg.m_szMetaName));
	snprintf(m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/ClusterGate.xml", CWorkDir::string());
}

bool ClusterGate::_ReadCfg()
{
    if (!CAppFrame::_ReadCfg())
    {
		return false;
    }

    inet_aton(m_stConfig.m_szProcIDStr, (struct in_addr*) &m_stConfig.m_iProcID);
    inet_aton(m_stConfig.m_szMatchSvrIDStr, (struct in_addr*) &m_stConfig.m_iMatchSvrID);
    inet_aton(m_stConfig.m_szSerialNumSvrIDStr, (struct in_addr*) &m_stConfig.m_iSerialNumSvrID);   
    inet_aton(m_stConfig.m_szMineSvrIDStr, (struct in_addr*) &m_stConfig.m_iMineSvrID);
    inet_aton(m_stConfig.m_szClusterAccountSvrIDStr, (struct in_addr*) &m_stConfig.m_iClusterAccountSvrID);
	inet_aton(m_stConfig.m_szGuildExpeditionSvrIDStr, (struct in_addr*) &m_stConfig.m_iGuildExpeditionSvrID);
    return true;
}

