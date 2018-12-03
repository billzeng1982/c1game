#include "./MineSvr.h"
#include "GameTime.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "LogMacros.h"
#include "./framework/MineSvrMsgLayer.h"
#include "./gamedata/GameDataMgr.h"
#include "./module/MineLogicMgr.h"
#include "./module/MineDataMgr.h"


extern unsigned char g_szMetalib_MineSvrCfg[];


bool MineSvr::AppInit()
{


    if (!CGameDataMgr::Instance().Init())
    {
        return false;
    }
    if (!MineSvrMsgLayer::Instance().Init())
    {
        return false;
    }
    if (!MineDataMgr::Instance().Init(&m_stConfig))
    {
        return false;
    }
    if (!MineLogicMgr::Instance().Init(&m_stConfig))
    {
        return false;
    }


    return true;
}

void MineSvr::AppFini()
{
    MineLogicMgr::Instance().Fini();
    MineDataMgr::Instance().Fini();		//·Å×îºó
    CAppFrame::AppFini();
}

void MineSvr::AppUpdate()
{


    bool bIdle = true;
    if (MineSvrMsgLayer::Instance().DealPkg() > 0)
    {
        bIdle = false;
    }
	MineDataMgr::Instance().Update(bIdle);
	MineLogicMgr::Instance().Update(bIdle);

    if (bIdle)
    {
        MsSleep(1);
    }
}

void MineSvr::_SetAppCfg( )
{
    m_stAppCfg.m_pszBuff = (char*)&m_stConfig;
    m_stAppCfg.m_dwLen = sizeof(m_stConfig);
    m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB)g_szMetalib_MineSvrCfg;
    StrCpy( m_stAppCfg.m_szMetaName, "MineSvrCfg", sizeof(m_stAppCfg.m_szMetaName) );
    snprintf( m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/MineSvr.xml", CWorkDir::string());
}

bool MineSvr::_ReadCfg( )
{
    if( !CAppFrame::_ReadCfg() )
    {
        return false;
    }
    inet_aton(m_stConfig.m_szProcIDStr, (struct in_addr*)&m_stConfig.m_iProcID);
    inet_aton(m_stConfig.m_szClusterGateIDStr, (struct in_addr*)&m_stConfig.m_iClusterGateID);
    inet_aton(m_stConfig.m_szMineDBSvrIDStr, (struct in_addr*)&m_stConfig.m_iMineDBSvrID);
    return true;
}

