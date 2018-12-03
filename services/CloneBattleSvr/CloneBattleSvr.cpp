#include "CloneBattleSvr.h"
#include "GameTime.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "LogMacros.h"
#include "framework/CloneBattleSvrMsgLayer.h"
#include "gamedata/GameDataMgr.h"
#include "module/CloneBattleMgr.h"


extern unsigned char g_szMetalib_CloneBattleSvrCfg[];


bool CloneBattleSvr::AppInit()
{


    if (!CGameDataMgr::Instance().Init())
    {
        return false;
    }
    if (!CloneBattleSvrMsgLayer::Instance().Init())
    {
        return false;
    }
    if (!CloneBattleMgr::Instance().Init(&m_stConfig))
    {
        return false;
    }

    return true;
}

void CloneBattleSvr::AppFini()
{
	CloneBattleMgr::Instance().Fini();
    CAppFrame::AppFini();
}

void CloneBattleSvr::AppUpdate()
{
    bool bIdle = true;

    if (CloneBattleSvrMsgLayer::Instance().DealPkg() > 0)
    {
        bIdle = false;
    }

    CloneBattleMgr::Instance().Update(bIdle);
    if (bIdle)
    {
        MsSleep(1);
    }
}

void CloneBattleSvr::_SetAppCfg( )
{
    m_stAppCfg.m_pszBuff = (char*)&m_stConfig;
    m_stAppCfg.m_dwLen = sizeof(m_stConfig);
    m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB)g_szMetalib_CloneBattleSvrCfg;
    StrCpy( m_stAppCfg.m_szMetaName, "CloneBattleSvrCfg", sizeof(m_stAppCfg.m_szMetaName) );
    snprintf( m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/CloneBattleSvr.xml", CWorkDir::string());
}

bool CloneBattleSvr::_ReadCfg( )
{
    if( !CAppFrame::_ReadCfg() )
    {
        return false;
    }

    inet_aton(m_stConfig.m_szProcIDStr, (struct in_addr*)&m_stConfig.m_iProcID);
    inet_aton(m_stConfig.m_szZoneSvrIDStr, (struct in_addr*)&m_stConfig.m_iZoneSvrID);

    inet_aton(m_stConfig.m_szMiscSvrIDStr, (struct in_addr*)&m_stConfig.m_iMiscSvrID);
    return true;
}

