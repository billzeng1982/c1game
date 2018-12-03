#include "MiscSvr.h"
#include "GameTime.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "LogMacros.h"
#include "framework/MiscSvrMsgLayer.h"

extern unsigned char g_szMetalib_MiscSvrCfg[];

bool MiscSvr::AppInit()
{
	if( !MiscSvrMsgLayer::Instance().Init() )
	{
		return false;
	}
	return true;
}


void MiscSvr::AppFini()
{
    MiscSvrMsgLayer::Instance().Fini();
	CAppFrame::AppFini();
}


void MiscSvr::AppUpdate()
{
	bool bIdle = true;

	if (MiscSvrMsgLayer::Instance().DealPkg() > 0)
	{
		bIdle = false;
	}

	if (bIdle)
	{
		MsSleep(1);
	}
}


void MiscSvr::_SetAppCfg( )
{
	m_stAppCfg.m_pszBuff = (char*)&m_stConfig;
	m_stAppCfg.m_dwLen = sizeof(m_stConfig);
	m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB)g_szMetalib_MiscSvrCfg;
	StrCpy( m_stAppCfg.m_szMetaName, "MiscSvrCfg", sizeof(m_stAppCfg.m_szMetaName) );
	snprintf( m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/MiscSvr.xml", CWorkDir::string());
	return;
}


bool MiscSvr::_ReadCfg( )
{
	if( !CAppFrame::_ReadCfg() )
	{
		return false;
	}

	inet_aton( m_stConfig.m_szProcIDStr, (struct in_addr* )&m_stConfig.m_iProcID );
	inet_aton( m_stConfig.m_szZoneSvrIDStr, (struct in_addr* )&m_stConfig.m_iZoneSvrID );
    inet_aton(m_stConfig.m_szCloneBattleSvrIDStr, (struct in_addr*)&m_stConfig.m_iCloneBattleSvrID);
	return true;
}


