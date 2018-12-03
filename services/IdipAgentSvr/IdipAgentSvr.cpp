#include "IdipAgentSvr.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "LogMacros.h"
#include "framework/IdipAgentSvrMsgLayer.h"
#include <curl/curl.h>

extern unsigned char g_szMetalib_IdipAgentSvrCfg[];

IdipAgentSvr::IdipAgentSvr()
{
	HttpReqMgr::Instance().Fini();
}

IdipAgentSvr::~IdipAgentSvr()
{

}

bool IdipAgentSvr::AppInit()
{
    //初始化libcurl
    curl_global_init(CURL_GLOBAL_ALL);

	if (!IdipAgentSvrMsgLayer::Instance().Init())
	{
		return false;
	}
    if (!HttpReqMgr::Instance().Init(&m_stConfig))
    {
        LOGERR("Init HttpReqMgr falied.");
        return false;
    }

	//初始化HttpClient的工作线程
	m_astClientThreads = new HttpClientThread[m_stConfig.m_iClientThreadNum];

	if (!m_astClientThreads)
	{
		return false;
	}

	key_t iShmKey = m_stConfig.m_iThreadQBaseShmKey;

	for( int i = 0; i < m_stConfig.m_iClientThreadNum; i++ )
	{
		if (!m_astClientThreads[i].InitThread(i, m_stConfig.m_dwThreadQSize, THREAD_QUEUE_DUPLEX, (void*)&m_stConfig, &iShmKey))
		{
			LOGERR_r("Init client thread <%d> failed", i );
			return false;
		}
	}

	//初始化HttpServer的工作线程
	m_astServerThreads = new HttpServerThread[m_stConfig.m_iServerThreadNum];

	if (!m_astServerThreads)
	{
		return false;
	}

	iShmKey = m_stConfig.m_iThreadQBaseShmKey;
	for( int i = 0; i < m_stConfig.m_iServerThreadNum; i++ )
	{
		if (!m_astServerThreads[i].InitThread(i, m_stConfig.m_dwThreadQSize, THREAD_QUEUE_DUPLEX, (void*)&m_stConfig, &iShmKey))
		{
			LOGERR_r("Init server thread <%d> failed", i );
			return false;
		}
	}

	return true;
}

void IdipAgentSvr::AppUpdate()
{
	bool bIdle = true;

	if (IdipAgentSvrMsgLayer::Instance().DealPkg() > 0)
	{
		bIdle = false;
	}

	if (bIdle)
	{
		MsSleep(1);
	}

	HttpReqMgr::Instance().RegToIdipSvr();
}

void IdipAgentSvr::AppFini()
{
	//清理libcurl
	curl_global_cleanup();

	for (int i = 0; i < m_stConfig.m_iClientThreadNum; i++)
	{
		m_astClientThreads[i].FiniThread();
	}

	for (int i = 0; i < m_stConfig.m_iServerThreadNum; i++)
	{
		m_astServerThreads[i].FiniThread();
	}

	HttpReqMgr::Instance().UnRegToIdipSvr();

	CAppFrame::AppFini();
}

void IdipAgentSvr::_SetAppCfg()
{
	m_stAppCfg.m_pszBuff = (char*)&m_stConfig;
	m_stAppCfg.m_dwLen = sizeof(m_stConfig);
	m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB)g_szMetalib_IdipAgentSvrCfg;
	StrCpy( m_stAppCfg.m_szMetaName, "IdipAgentSvrCfg", sizeof(m_stAppCfg.m_szMetaName) );
	snprintf( m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/IdipAgentSvr.xml", CWorkDir::string());
}

bool IdipAgentSvr::_ReadCfg()
{
	if( !CAppFrame::_ReadCfg() )
	{
		return false;
	}

	inet_aton( m_stConfig.m_szProcIDStr, (struct in_addr* )&m_stConfig.m_iProcID );
	inet_aton( m_stConfig.m_szLogQuerySvrIDStr, (struct in_addr* )&m_stConfig.m_iLogQuerySvrID );
	inet_aton( m_stConfig.m_szZoneSvrIDStr, (struct in_addr* )&m_stConfig.m_iZoneSvrID );
	inet_aton(m_stConfig.m_szRoleSvrIDStr, (struct in_addr*)&m_stConfig.m_iRoleSvrID );

	return true;
}



