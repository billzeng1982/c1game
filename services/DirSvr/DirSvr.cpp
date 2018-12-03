#include "DirSvr.h"
#include "workdir.h"
#include "oi_misc.h"
#include "strutil.h"
#include "framework/DirSvrMsgLayer.h"
#include "module/DirHttpServer.h"
#include "gamedata/GameDataMgr.h"
#include "module/DirHttpServer.h"
#include <curl/curl.h>

extern unsigned char g_szMetalib_DirSvrCfg[];

bool DirSvr::AppInit()
{
    if(!CGameDataMgr::Instance().Init())
    {
        LOGERR("init CGameDataMgr failed.");
        return false;
    }
    
	if (!DirSvrMsgLayer::Instance().Init())
	{
		return false;
	}

    //初始化libcurl
    curl_global_init(CURL_GLOBAL_ALL);

    if (!DirHttpServer::Instance().Init())
    {
        return false;
    }

	return true;
}

void DirSvr::AppFini()
{
    //清理libcurl
    curl_global_cleanup();

	CAppFrame::AppFini();
}

void DirSvr::AppUpdate()
{
    DirSvrMsgLayer::Instance().DealPkg();

    DirHttpServer::Instance().Update();

    MsSleep(1);
}

void DirSvr::_SetAppCfg()
{
	m_stAppCfg.m_pszBuff = (char*) &m_stConfig;
	m_stAppCfg.m_dwLen = sizeof(m_stConfig);
	m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB) g_szMetalib_DirSvrCfg;
	StrCpy(m_stAppCfg.m_szMetaName, "DirSvrCfg", sizeof(m_stAppCfg.m_szMetaName));
	snprintf(m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/DirSvr.xml", CWorkDir::string());
}

bool DirSvr::_ReadCfg()
{
	if (!CAppFrame::_ReadCfg())
    {
		return false;
	}

	inet_aton(m_stConfig.m_szProcIDStr, (struct in_addr*) &m_stConfig.m_iProcID);

	return true;
}

bool DirSvr::ReloadGamedata()
{
    if(!CGameDataMgr::Instance().Reload())
    {
        LOGERR("reload gamedata failed.");
        return false;
    }

    if (!DirHttpServer::Instance().LoadData())
    {
        LOGERR_r("Load SvrList Data failed");
        return false;
    }

    return true;
}

