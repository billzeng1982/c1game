/*
 * 
 */

#pragma once

#include "define.h"
#include "ThreadQueue.h"
#include "ThreadFrame.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"
#include "HttpServer.h"
#include "XiYouSDKSvrCfgDesc.h"
#include "tdr/tdr.h"

using namespace PKGMETA;

class HttpServerThread : public CThreadFrame
{
public:
	HttpServerThread();
	~HttpServerThread(){}

    void SendPkg(SSPKG& rstSsPkg);

protected:
	virtual bool _ThreadAppInit(void* pvAppData);
	virtual void _ThreadAppFini();
	virtual int _ThreadAppProc();
	int _HandleRoleSvrMsg();

    static void _HandlePayCb(struct evhttp_request *req, void *arg);

protected:
	CHttpServer m_oHttpServer;
	SSPKG m_stSsPkg;
    MyTdrBuf m_stSendBuf;
    XIYOUSDKSVRCFG* m_pstConfig;
    LPTDRMETA m_pstPayCbDataMeta;
};

