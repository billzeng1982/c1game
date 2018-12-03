#include "ClusterDBSvr.h"
#include "GameTime.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "LogMacros.h"
#include "./framework/ClusterDBSvrMsgLayer.h"

extern unsigned char g_szMetalib_ClusterDBSvrCfg[];

bool ClusterDBSvr::AppInit()
{
	if( !ClusterDBSvrMsgLayer::Instance().Init() )
	{
		return false;
	}
	return true;
}


void ClusterDBSvr::AppFini()
{
    ClusterDBSvrMsgLayer::Instance().Fini();
	CAppFrame::AppFini();
}


void ClusterDBSvr::AppUpdate()
{
	bool bIdle = true;

	if (ClusterDBSvrMsgLayer::Instance().DealPkg() > 0)
	{
		bIdle = false;
	}

	if (bIdle)
	{
		MsSleep(1);
	}
}


void ClusterDBSvr::_SetAppCfg( )
{
	m_stAppCfg.m_pszBuff = (char*)&m_stConfig;
	m_stAppCfg.m_dwLen = sizeof(m_stConfig);
	m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB)g_szMetalib_ClusterDBSvrCfg;
	StrCpy( m_stAppCfg.m_szMetaName, "ClusterDBSvrCfg", sizeof(m_stAppCfg.m_szMetaName) );
	snprintf( m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/ClusterDBSvr.xml", CWorkDir::string());
	return;
}


bool ClusterDBSvr::_ReadCfg( )
{
	if( !CAppFrame::_ReadCfg() )
	{
		return false;
	}

	inet_aton( m_stConfig.m_szProcIDStr, (struct in_addr* )&m_stConfig.m_iProcID );
	inet_aton( m_stConfig.m_szGuildExpeditionSvrIDStr, (struct in_addr* )&m_stConfig.m_iGuildExpeditionSvrID );
	return true;
}


