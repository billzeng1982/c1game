#include "MineDBSvr.h"
#include "GameTime.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "LogMacros.h"
#include "./framework/MineDBSvrMsgLayer.h"

extern unsigned char g_szMetalib_MineDBSvrCfg[];

bool MineDBSvr::AppInit()
{
	if( !MineDBSvrMsgLayer::Instance().Init() )
	{
		return false;
	}
	return true;
}


void MineDBSvr::AppFini()
{
    MineDBSvrMsgLayer::Instance().Fini();
	CAppFrame::AppFini();
}


void MineDBSvr::AppUpdate()
{
	bool bIdle = true;

	if (MineDBSvrMsgLayer::Instance().DealPkg() > 0)
	{
		bIdle = false;
	}

	if (bIdle)
	{
		MsSleep(1);
	}
}


void MineDBSvr::_SetAppCfg( )
{
	m_stAppCfg.m_pszBuff = (char*)&m_stConfig;
	m_stAppCfg.m_dwLen = sizeof(m_stConfig);
	m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB)g_szMetalib_MineDBSvrCfg;
	StrCpy( m_stAppCfg.m_szMetaName, "MineDBSvrCfg", sizeof(m_stAppCfg.m_szMetaName) );
	snprintf( m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/MineDBSvr.xml", CWorkDir::string());
	return;
}


bool MineDBSvr::_ReadCfg( )
{
	if( !CAppFrame::_ReadCfg() )
	{
		return false;
	}

	inet_aton( m_stConfig.m_szProcIDStr, (struct in_addr* )&m_stConfig.m_iProcID );
	inet_aton( m_stConfig.m_szMineSvrIDStr, (struct in_addr* )&m_stConfig.m_iMineSvrID );
	return true;
}


