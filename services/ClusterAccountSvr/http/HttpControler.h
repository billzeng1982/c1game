#pragma once

/*
	运行在主线程
	TODO: 加入 timer, timeout控制保护
*/

#include "HttpServer.h"
#include "oi_misc.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "tdr/tdr.h"
#include "ss_proto.h"
#include "cfg/ClusterAccountSvrCfgDesc.h"
#include "mempool.h"
#include <map>
#include "MyTdrBuf.h"

using namespace PKGMETA;
using namespace std;

class HttpControler
{
	typedef std::map<uint32_t /*id*/, struct evhttp_request*> HttpReqMap_t;

public:
	HttpControler();
	~HttpControler(){}
	
	CHttpServer* GetHttpServer() { return &m_oHttpServer; }
	
	bool Init( CLUSTERACCOUNTSVRCFG* pstConfig );
	void Update();

	void OnHttpGetOneAccInfo( struct evhttp_request *req, char* szReqJson );

	void SendHttpRsp(MyTdrBuf* pstTdrBuf);

private:
	uint32_t _GetNextReqID();
	void _Add2HttpReqMap( uint32_t dwReqID, struct evhttp_request *req);
	struct evhttp_request* _FindHttpReq( uint32_t dwReqID );
	void _DelFromHttpReq( uint32_t dwReqID );

	bool _Send2DbThread(SSPKG& rstSsPkg);
	void _SendGetOneAccInfoHttpRsp( uint32_t dwReqID, SS_PKG_GET_ONE_CLUSTER_ACC_INFO_RSP& rstGetOneAccInfoRsp );

	void _SendHttpRsp( struct evhttp_request *req, char* szRspStr );
	void _SendErrRsp( struct evhttp_request *req, int iErrNo );

private:
	CHttpServer m_oHttpServer;	
	CLUSTERACCOUNTSVRCFG* m_pstConfig;
	LPTDRMETALIB m_pstMetaLib;
	SSPKG m_stSsPkg;
	uint32_t m_dwSeqID;
	HttpReqMap_t m_oHttpReqMap;

	MyTdrBuf	m_stTdrBuf;
};

