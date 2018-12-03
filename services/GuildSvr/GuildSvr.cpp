#include "GuildSvr.h"
#include "GameTime.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "LogMacros.h"
#include "TableDefine.h"
#include "framework/GuildSvrMsgLayer.h"
#include "transaction/GuildTransFrame.h"
#include "gamedata/GameDataMgr.h"
#include "module/Guild/GuildMgr.h"
#include "module/Player/PlayerMgr.h"
#include "module/Fight/GuildFightMgr.h"
#include "module/Fight/GFightArenaMgr.h"
#include "module/Guild/GuildRank.h"
#include "module/Guild/GuildDataDynPool.h"
#include "module/Guild/GuildBossMgr.h"

extern unsigned char g_szMetalib_GuildSvrCfg[];

GuildSvr::GuildSvr()
{

}

bool GuildSvr::AppInit()
{
    if (!GuildSvrMsgLayer::Instance().Init())
    {
        return false;
    }

    if (!CGameDataMgr::Instance().Init())
    {
        LOGERR("CGameDataMgr init error.");
        return false;
    }

    if (!GuildTransFrame::Instance().Init(1000, 1000))
    {
        LOGERR("GuildTransFrame Init failed");
        return false;
    }

    if (!GuildMgr::Instance().Init(&m_stConfig))
    {
        LOGERR("GuildMgr Init failed");
        return false;
    }

    if (!PlayerMgr::Instance().Init(&m_stConfig))
    {
        LOGERR("PlayerMgr Init failed");
        return false;
    }

    if (!GuildFightMgr::Instance().Init(m_stConfig.m_iGFightUptVal, m_stConfig.m_szFileName))
    {
        LOGERR("GuildFightMgr Init failed");
        return false;
    }

	if (!GuildBossMgr::Instance().Init(m_stConfig.m_szFileNameGuildBoss))
	{
		LOGERR("GuildBossMgr Init failed");
		return false;
	}

    if (!GFightArenaMgr::Instance().Init())
    {
        LOGERR("GFightArenaMgr Init failed");
        return false;
    }

    if (!GuildDataDynPool::Instance().Init())
    {
        LOGERR("GuildDataDynPool Init failed");
        return false;
    }

    return true;
}

void GuildSvr::AppFini()
{
    GuildMgr::Instance().Fini();
    PlayerMgr::Instance().Fini();
    GuildFightMgr::Instance().Fini();
	GuildBossMgr::Instance().Fini();
    CAppFrame::AppFini();
}

void GuildSvr::AppUpdate()
{
    bool bIdle = true;

    if (GuildSvrMsgLayer::Instance().DealPkg() > 0)
    {
        bIdle = false;
    }

    GuildMgr::Instance().Update();
    PlayerMgr::Instance().Update();
    GuildTransFrame::Instance().Update();

    if (m_stConfig.m_iGFightSwitch)
    {
        GuildFightMgr::Instance().Update();
        GFightArenaMgr::Instance().Update();
    }

    GuildBossMgr::Instance().Update();

    if (bIdle)
    {
        MsSleep(1);
    }
}

void GuildSvr::_SetAppCfg()
{
    m_stAppCfg.m_pszBuff = (char*) &m_stConfig;
    m_stAppCfg.m_dwLen = sizeof(m_stConfig);
    m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB) g_szMetalib_GuildSvrCfg;
    StrCpy(m_stAppCfg.m_szMetaName, "GuildSvrCfg", sizeof(m_stAppCfg.m_szMetaName));
    snprintf(m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/GuildSvr.xml", CWorkDir::string());
}

bool GuildSvr::_ReadCfg()
{
    if (!CAppFrame::_ReadCfg())
    {
        return false;
    }
    inet_aton(m_stConfig.m_szProcIDStr, (struct in_addr*) &m_stConfig.m_iProcID);
    inet_aton(m_stConfig.m_szZoneSvrIDStr, (struct in_addr*) &m_stConfig.m_iZoneSvrID);
    inet_aton(m_stConfig.m_szMailSvrIDStr, (struct in_addr*) &m_stConfig.m_iMailSvrID);
	inet_aton(m_stConfig.m_szRankSvrIDStr, (struct in_addr*) &m_stConfig.m_iRankSvrID);
	inet_aton(m_stConfig.m_szClusterGateIDStr, (struct in_addr*) &m_stConfig.m_iClusterGateID);
    return true;
}

