#include "MailSvr.h"
#include "GameTime.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "LogMacros.h"
#include "MailSvrMsgLayer.h"
#include "GameDataMgr.h"
#include "MailBoxMgr.h"
#include "MailPubServData.h"
#include "MailSvrPool.h"


extern unsigned char g_szMetalib_MailSvrCfg[];

MailSvr::MailSvr()
{

}

bool MailSvr::AppInit()
{
    if (!MailSvrPool::Instance().Init())
    {
        return false;
    }

    if (!CGameDataMgr::Instance().Init())
    {
        return false;
    }

    if( !MailSvrMsgLayer::Instance().Init() )
    {
        return false;
    }

    if (!MailPubServData::Instance().Init())
    {
        return false;
    }

    if (!MailBoxMgr::Instance().Init(&m_stConfig))
    {
        return false;
    }

    return true;
}

void MailSvr::AppFini()
{
    MailBoxMgr::Instance().Fini();
    MailPubServData::Instance().Fini();
    CAppFrame::AppFini();
}

void MailSvr::AppUpdate()
{
    bool bIdle = true;

    if (MailSvrMsgLayer::Instance().DealPkg() > 0)
    {
        bIdle = false;
    }

    MailPubServData::Instance().Update();
    MailBoxMgr::Instance().Update();

    if (bIdle)
    {
        MsSleep(1);
    }
}

void MailSvr::_SetAppCfg( )
{
    m_stAppCfg.m_pszBuff = (char*)&m_stConfig;
    m_stAppCfg.m_dwLen = sizeof(m_stConfig);
    m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB)g_szMetalib_MailSvrCfg;
    StrCpy( m_stAppCfg.m_szMetaName, "MailSvrCfg", sizeof(m_stAppCfg.m_szMetaName) );
    snprintf( m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/MailSvr.xml", CWorkDir::string());
}

bool MailSvr::_ReadCfg( )
{
    if( !CAppFrame::_ReadCfg() )
    {
        return false;
    }

    inet_aton( m_stConfig.m_szProcIDStr, (struct in_addr* )&m_stConfig.m_iProcID );
    inet_aton( m_stConfig.m_szZoneSvrIDStr, (struct in_addr* )&m_stConfig.m_iZoneSvrID );
    inet_aton( m_stConfig.m_szMailDBSvrIDStr, (struct in_addr* )&m_stConfig.m_iMailDBSvrID );

    return true;
}

