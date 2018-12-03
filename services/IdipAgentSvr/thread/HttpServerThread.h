#pragma once

#include "define.h"
#include "ThreadQueue.h"
#include "ThreadFrame.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"
#include "HttpServer.h"
#include "IdipAgentSvrCfgDesc.h"
#include "tdr/tdr.h"
#include "idip_proto.h"

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

	static void _HandleGmQuery(struct evhttp_request *req, void *arg);
	//gm操作
	static void _HandleGmOp(struct evhttp_request *req, void *arg);

	uint64_t _GenUniqeKey();

protected:
	CHttpServer m_oHttpServer;
	SSPKG m_stSsPkg;
    MyTdrBuf m_stSendBuf;
    IDIPAGENTSVRCFG* m_pstConfig;
    LPTDRMETA m_pstGmQueryDataMeta;

    uint64_t m_ullKey;
};

