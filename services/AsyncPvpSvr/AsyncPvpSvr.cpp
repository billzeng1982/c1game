#include "AsyncPvpSvr.h"
#include "GameTime.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "LogMacros.h"
#include "AsyncPvpSvrMsgLayer.h"
#include "AsyncPvpPlayerMgr.h"
#include "AsyncPvpTeamMgr.h"
#include "AsyncPvpRank.h"
#include "AsyncPvpRecordMgr.h"
#include "AsyncPvpFightMgr.h"
#include "AsyncPvpTransFrame.h"
#include "GameDataMgr.h"
#include "module/rank/AsyncPvpWorship.h"

extern unsigned char g_szMetalib_AsyncPvpSvrCfg[];

AsyncPvpSvr::AsyncPvpSvr()
{

}

bool AsyncPvpSvr::AppInit()
{
    if (!CGameDataMgr::Instance().Init())
    {
        LOGERR_r("CGameDataMgr init error.");
        return false;
    }

    if (!AsyncPvpTransFrame::Instance().Init(100, 100))
	{
        LOGERR_r("AsyncPvpTransFrame init error.");
		return false;
	}

	if (!AsyncPvpSvrMsgLayer::Instance().Init())
	{
        LOGERR_r("AsyncPvpSvrMsgLayer init error.");
		return false;
	}

    if (!AsyncPvpFightMgr::Instance().Init())
    {
        LOGERR_r("AsyncPvpFightMgr init error.");
        return false;
    }

    if (!AsyncPvpRecordMgr::Instance().Init())
    {
        LOGERR_r("AsyncPvpRecordMgr init error.");
        return false;
    }

    if (!AsyncPvpPlayerMgr::Instance().Init(&m_stConfig))
    {
        LOGERR_r("AsyncPvpPlayerMgr init error.");
        return false;
    }

    if (!AsyncPvpTeamMgr::Instance().Init(&m_stConfig))
    {
        LOGERR_r("AsyncPvpTeamMgr init error.");
        return false;
    }

    if (!AsyncPvpRank::Instance().Init())
    {
        LOGERR_r("AsyncPvpRank init error.");
        return false;
    }

	if (!AsyncPvpWorship::Instance().Init())
	{
		LOGERR_r("AsyncPvpWorship init error.");
		return false;
	}

	LOGRUN_r( "AsyncPvpSvr::AppInit OK! " );
	return true;
}

void AsyncPvpSvr::AppFini()
{
    AsyncPvpPlayerMgr::Instance().Fini();
    AsyncPvpTeamMgr::Instance().Fini();
    AsyncPvpRank::Instance().Fini();
    CAppFrame::AppFini();
}

void AsyncPvpSvr::AppUpdate()
{
	bool bIdle = true;
	if (AsyncPvpSvrMsgLayer::Instance().DealPkg() > 0)
	{
		bIdle = false;
	}

    AsyncPvpPlayerMgr::Instance().Update();
    AsyncPvpTeamMgr::Instance().Update();
    AsyncPvpFightMgr::Instance().Update();
    AsyncPvpTransFrame::Instance().Update();
    AsyncPvpRecordMgr::Instance().Update();
    AsyncPvpRank::Instance().Update();
	AsyncPvpWorship::Instance().Update();

    if (bIdle)
	{
		MsSleep(1);
	}
}

void AsyncPvpSvr::_SetAppCfg( )
{
	m_stAppCfg.m_pszBuff = (char*)&m_stConfig;
	m_stAppCfg.m_dwLen = sizeof(m_stConfig);
	m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB)g_szMetalib_AsyncPvpSvrCfg;
	StrCpy( m_stAppCfg.m_szMetaName, "AsyncPvpSvrCfg", sizeof(m_stAppCfg.m_szMetaName) );
	snprintf( m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/AsyncPvpSvr.xml", CWorkDir::string());
}

bool AsyncPvpSvr::_ReadCfg( )
{
	if( !CAppFrame::_ReadCfg() )
	{
		return false;
	}

	inet_aton( m_stConfig.m_szProcIDStr, (struct in_addr* )&m_stConfig.m_iProcID );
	inet_aton( m_stConfig.m_szZoneSvrIDStr, (struct in_addr* )&m_stConfig.m_iZoneSvrID );
    inet_aton( m_stConfig.m_szMailSvrIDStr, (struct in_addr* )&m_stConfig.m_iMailSvrID );

	return true;
}

