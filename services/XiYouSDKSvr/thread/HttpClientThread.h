#pragma once

#include "define.h"
#include "ThreadQueue.h"
#include "ThreadFrame.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"
#include "HttpClient.h"
#include "XiYouSDKSvrCfgDesc.h"
#include "XiYouSDKSvrMsgLayer_r.h"

using namespace PKGMETA;

class HttpClientThread : public CThreadFrame
{
public:
	HttpClientThread();
	~HttpClientThread(){}

    	void SendPkg(SSPKG& rstSsPkg);
	
protected:
	virtual bool _ThreadAppInit(void* pvAppData);
	virtual void _ThreadAppFini();
	virtual int _ThreadAppProc();

protected:
	SSPKG m_stSsRecvPkg;
    	MyTdrBuf m_stSendBuf;
    	XIYOUSDKSVRCFG* m_pstConfig;
	XiYouSDKSvrMsgLayer_r m_oMsgLayer;	

public:
    CHttpClient m_oHttpClient;
};

