#include "GuildExpeditionSvr.h"
#include "GameTime.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "LogMacros.h"
#include "./framework/GuildExpeditionSvrMsgLayer.h"
#include "./framework/GameDataMgr.h"
#include "./module/DataMgr.h"
#include "./module/LogicMgr.h"

extern unsigned char g_szMetalib_GuildExpeditionSvrCfg[];

bool GuildExpeditionSvr::AppInit()
{
	if (!CGameDataMgr::Instance().Init())
	{
		return false;
	}
	if( !GuildExpeditionSvrMsgLayer::Instance().Init() )
	{
		return false;
	}
	if (!DataMgr::Instance().Init(&m_stConfig))
	{
		return false;
	}
	if (!LogicMgr::Instance().Init())
	{
		return false;
	}
	return true;
}


void GuildExpeditionSvr::AppFini()
{
	LogicMgr::Instance().Fini();

	DataMgr::Instance().Fini();

    
	CAppFrame::AppFini();
}


void GuildExpeditionSvr::AppUpdate()
{
	bool bIdle = true;

	if (GuildExpeditionSvrMsgLayer::Instance().DealPkg() > 0)
	{
		bIdle = false;
	}

	DataMgr::Instance().Update(bIdle);

	LogicMgr::Instance().Update();
	if (bIdle)
	{
		MsSleep(1);
	}
}


void GuildExpeditionSvr::_SetAppCfg( )
{
	m_stAppCfg.m_pszBuff = (char*)&m_stConfig;
	m_stAppCfg.m_dwLen = sizeof(m_stConfig);
	m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB)g_szMetalib_GuildExpeditionSvrCfg;
	StrCpy( m_stAppCfg.m_szMetaName, "GuildExpeditionSvrCfg", sizeof(m_stAppCfg.m_szMetaName) );
	snprintf( m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/GuildExpeditionSvr.xml", CWorkDir::string());
	return;
}


bool GuildExpeditionSvr::_ReadCfg( )
{
	if( !CAppFrame::_ReadCfg() )
	{
		return false;
	}

	inet_aton( m_stConfig.m_szProcIDStr,			(struct in_addr* )&m_stConfig.m_iProcID );
	inet_aton( m_stConfig.m_szClusterDBSvrIDStr,	(struct in_addr* )&m_stConfig.m_iClusterDBSvrID );
	inet_aton(m_stConfig.m_szClusterGateIDStr,		(struct in_addr*)&m_stConfig.m_iClusterGateID);
	return true;
}


