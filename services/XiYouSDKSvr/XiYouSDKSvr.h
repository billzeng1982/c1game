#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/XiYouSDKSvrCfgDesc.h"
#include "thread/HttpClientThread.h"
#include "thread/HttpServerThread.h"

class XiYouSDKSvr : public CAppFrame, public TSingleton<XiYouSDKSvr>
{
public:
	XiYouSDKSvr();
	virtual ~XiYouSDKSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();
	virtual int GetProcID(){ return m_stConfig.m_iProcID; }

public:
	XIYOUSDKSVRCFG& GetConfig(){ return m_stConfig; }
    HttpClientThread* GetClientThread(int iPos){ return &m_astClientThreads[iPos]; }
    HttpServerThread* GetServerThread(int iPos){ return &m_astServerThreads[iPos]; }

private:
	virtual void _SetAppCfg( );
	virtual bool _ReadCfg( );

    bool _RegToClusterSdkCbSvr();

private:
	XIYOUSDKSVRCFG m_stConfig;
    HttpClientThread* m_astClientThreads;
    HttpServerThread* m_astServerThreads;

    CHttpClient m_oHttpClient;
    char m_szPostInfo[PKGMETA::MAX_LEN_HTTPPOST_INFO];
};

