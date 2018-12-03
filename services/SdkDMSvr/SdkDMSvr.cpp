#include "SdkDMSvr.h"
#include "GameTime.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "./framework/SdkDMSvrMsgLayer.h"
#include "LogMacros.h"
#include "TableDefine.h"
#include "./module/TalkingData/TDataWorkThreadMgr.h"

extern unsigned char g_szMetalib_SdkDMSvrCfg[];

SdkDMSvr::SdkDMSvr() {
    //m_astWorkThreads = NULL;
}

bool SdkDMSvr::AppInit() {
    if (!SdkDMSvrMsgLayer::Instance().Init()) 
    {
        return false;
    }


    if (!TDataWorkThreadMgr::Instance().Init(&m_stConfig))
    {
        assert(false);
        LOGERR_r("TDataWorkThreadMgr init failed!");
        return false;
    }
    

    return true;
}

void SdkDMSvr::AppFini() 
{
    TDataWorkThreadMgr::Instance().Fini();
    CAppFrame::AppFini();
}

void SdkDMSvr::AppUpdate() 
{
    bool bIdle = true;

    if (SdkDMSvrMsgLayer::Instance().DealPkg() > 0) {
        bIdle = false;
    }

    if (bIdle) {
        MsSleep(1);
    }
}

void SdkDMSvr::_SetAppCfg() {
    m_stAppCfg.m_pszBuff = (char*) &m_stConfig;
    m_stAppCfg.m_dwLen = sizeof(m_stConfig);
    m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB) g_szMetalib_SdkDMSvrCfg;
    StrCpy(m_stAppCfg.m_szMetaName, "SdkDMSvrCfg",
            sizeof(m_stAppCfg.m_szMetaName));
    snprintf(m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile),
            "%s/config/SdkDMSvr.xml", CWorkDir::string());
}

bool SdkDMSvr::_ReadCfg() {
    if (!CAppFrame::_ReadCfg()) {
        return false;
    }

    inet_aton(m_stConfig.m_szProcIDStr,
            (struct in_addr*) &m_stConfig.m_iProcID);
// 	inet_aton(m_stConfig.m_szProxySvrIDStr,
// 			(struct in_addr*) &m_stConfig.m_iProxySvrID);

    return true;
}

