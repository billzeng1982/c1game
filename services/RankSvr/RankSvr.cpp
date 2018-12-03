#include "RankSvr.h"
#include "GameTime.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "LogMacros.h"
#include "framework/RankSvrMsgLayer.h"
#include "gamedata/GameDataMgr.h"
#include "module/RankMgr.h"
extern unsigned char g_szMetalib_RankSvrCfg[];

RankSvr::RankSvr()
{
}

bool RankSvr::AppInit()
{
    if (!CGameDataMgr::Instance().Init())
    {
        return false;
    }
    
    if (!RankSvrMsgLayer::Instance().Init())
    {
        return false;
    }

    if (!RankMgr::Instance().Init())
    {
        return false;
    }
   
    return true;
}

void RankSvr::AppUpdate()
{
    bool bIdle = true;

    if (RankSvrMsgLayer::Instance().DealPkg() > 0)
    {
        bIdle = false;
    }

    if (bIdle)
    {
        MsSleep(1);
    }
}


void RankSvr::AppFini()
{
    RankMgr::Instance().OnExit();
    CAppFrame::AppFini();
}

void RankSvr::_SetAppCfg()
{
    m_stAppCfg.m_pszBuff = (char*)&m_stConfig;
    m_stAppCfg.m_dwLen = sizeof(m_stConfig);
    m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB)g_szMetalib_RankSvrCfg;
    StrCpy( m_stAppCfg.m_szMetaName, "RankSvrCfg", sizeof(m_stAppCfg.m_szMetaName) );
    snprintf( m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/RankSvr.xml", CWorkDir::string());
}


bool RankSvr::_ReadCfg( )
{
    if( !CAppFrame::_ReadCfg() )
    {
        return false;
    }

    inet_aton( m_stConfig.m_szProcIDStr, (struct in_addr* )&m_stConfig.m_iProcID );
    inet_aton( m_stConfig.m_szZoneSvrIDStr, (struct in_addr* )&m_stConfig.m_iZoneSvrID );
    inet_aton( m_stConfig.m_szMailSvrIDStr, (struct in_addr* )&m_stConfig.m_iMailSvrID );
    inet_aton(m_stConfig.m_szGuildSvrIDStr, (struct in_addr*)&m_stConfig.m_iGuildSvrID);

    return true;
}


