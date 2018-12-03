#include "MatchSvr.h"
#include "workdir.h"
#include "oi_misc.h"
#include "strutil.h"
#include "ObjectUpdatorMgr.h"
#include "framework/GameObjectPool.h"
#include "framework/MatchSvrMsgLayer.h"
#include "module/MatchMgr.h"
#include "gamedata/GameDataMgr.h"

extern unsigned char g_szMetalib_MatchSvrCfg[];

bool MatchSvr::AppInit()
{
	if (!MatchSvrMsgLayer::Instance().Init())
	{
		return false;
	}

	if (!CGameDataMgr::Instance().Init())
    {
		return false;
	}

    if (!MatchWeekMgr::Instance().Init())
    {
        return false;
    }

    if (!Match6v6Mgr::Instance().Init())
    {
        return false;
    }

    if (!MatchLeisureMgr::Instance().Init())
    {
        return false;
    }

    if (!MatchDailyChallengeMgr::Instance().Init())
    {
        return false;
    }

    if (!MatchPeakArenaMgr::Instance().Init())
    {
       return false;
    }

	return true;
}

void MatchSvr::AppFini()
{
	CAppFrame::AppFini();
}

void MatchSvr::AppUpdate()
{
	bool bIdle = true;

    MatchWeekMgr::Instance().Update();
    Match6v6Mgr::Instance().Update();
    MatchLeisureMgr::Instance().Update();
    MatchDailyChallengeMgr::Instance().Update();
    MatchPeakArenaMgr::Instance().Update();

	// deal pkgs
	if (MatchSvrMsgLayer::Instance().DealPkg() > 0)
    {
		bIdle = false;
	}

	if (bIdle)
    {
        MsSleep(1);
    }
}

void MatchSvr::_SetAppCfg()
{
	m_stAppCfg.m_pszBuff = (char*) &m_stConfig;
	m_stAppCfg.m_dwLen = sizeof(m_stConfig);
	m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB) g_szMetalib_MatchSvrCfg;
	StrCpy(m_stAppCfg.m_szMetaName, "MatchSvrCfg", sizeof(m_stAppCfg.m_szMetaName));
	snprintf(m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/MatchSvr.xml", CWorkDir::string());
}

bool MatchSvr::_ReadCfg()
{
	if (!CAppFrame::_ReadCfg())
    {
		return false;
	}

	inet_aton(m_stConfig.m_szProcIDStr, (struct in_addr*) &m_stConfig.m_iProcID);
    inet_aton(m_stConfig.m_szClusterGateIDStr, (struct in_addr*) &m_stConfig.m_iClusterGateID);

	return true;
}

