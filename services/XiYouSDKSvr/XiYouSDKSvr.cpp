#include "XiYouSDKSvr.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "LogMacros.h"
#include "framework/XiYouSDKSvrMsgLayer.h"
#include <curl/curl.h>
#include "GameTime.h"

extern unsigned char g_szMetalib_XiYouSDKSvrCfg[];

XiYouSDKSvr::XiYouSDKSvr()
{

}

bool XiYouSDKSvr::AppInit()
{
    if (!XiYouSDKSvrMsgLayer::Instance().Init())
    {
        return false;
    }

    //初始化libcurl
    curl_global_init(CURL_GLOBAL_ALL);

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

    //初始化HttpClient
    m_oHttpClient.ChgUrl(m_stConfig.m_szClusterCbSvrUrl);
    m_oHttpClient.HeaderAppend( NULL );

    return true;
}

bool XiYouSDKSvr::_RegToClusterSdkCbSvr()
{
    static uint64_t LastRegTime = 0;

    if ((CGameTime::Instance().GetCurrSecond() - LastRegTime) > m_stConfig.m_dwRegInterval)
    {
        LastRegTime = CGameTime::Instance().GetCurrSecond();
        m_szPostInfo[0] = '\0';
        snprintf(m_szPostInfo, MAX_LEN_HTTPPOST_INFO, "{\"SvrID\":\"%d\",\"CbUrl\":\"%s:%d%s\"}",
                m_stConfig.m_iSvrId,m_stConfig.m_szSvrIpStr, m_stConfig.m_iHttpSvrPort, m_stConfig.m_szPayCallBackUri);

        LOGRUN_r("Send post req, PostInfo(%s)", m_szPostInfo);
        if (!m_oHttpClient.Post(m_szPostInfo))
        {
            LOGERR_r("Reg to ClusterCbSvr failed, errinfo=(%s)", m_oHttpClient.GetLastErrInfo());
            return false;
        }
        LOGRUN_r("Recv http Rsp, data=%s", m_oHttpClient.m_szRspDataBuf);
    }

    return true;
}

void XiYouSDKSvr::AppUpdate()
{
    bool bIdle = true;

    if (XiYouSDKSvrMsgLayer::Instance().DealPkg() > 0)
    {
        bIdle = false;
    }

    if (bIdle)
    {
        MsSleep(1);
    }

    _RegToClusterSdkCbSvr();
}

void XiYouSDKSvr::AppFini()
{
    //清理libcurl
    curl_global_cleanup();

	for( int i = 0; i < m_stConfig.m_iClientThreadNum; i++ )
	{
		m_astClientThreads[i].FiniThread();
	}

	for ( int i = 0; i < m_stConfig.m_iServerThreadNum; i++ )
	{
		m_astServerThreads[i].FiniThread();
	}

    CAppFrame::AppFini();
}

void XiYouSDKSvr::_SetAppCfg()
{
    m_stAppCfg.m_pszBuff = (char*)&m_stConfig;
    m_stAppCfg.m_dwLen = sizeof(m_stConfig);
    m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB)g_szMetalib_XiYouSDKSvrCfg;
    StrCpy( m_stAppCfg.m_szMetaName, "XiYouSDKSvrCfg", sizeof(m_stAppCfg.m_szMetaName) );
    snprintf( m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/XiYouSDKSvr.xml", CWorkDir::string());
}

bool XiYouSDKSvr::_ReadCfg()
{
    if( !CAppFrame::_ReadCfg() )
    {
        return false;
    }

    inet_aton( m_stConfig.m_szProcIDStr, (struct in_addr* )&m_stConfig.m_iProcID );
    inet_aton( m_stConfig.m_szZoneSvrIDStr, (struct in_addr* )&m_stConfig.m_iZoneSvrID );

    return true;
}


