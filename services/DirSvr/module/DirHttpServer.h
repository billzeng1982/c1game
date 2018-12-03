#pragma once

#include "define.h"
#include "common_proto.h"
#include "HttpServer.h"
#include "DirSvrCfgDesc.h"

using namespace PKGMETA;

class DirHttpServer : public TSingleton<DirHttpServer>
{
public:
	DirHttpServer();
	~DirHttpServer(){}

    bool Init();
    bool Fini();

    bool LoadData();

    void Update();

    DT_SERVER_LIST & GetSvrList();

protected:
    static void _HandleGetSvrList(struct evhttp_request *req, void *arg);

public:
	CHttpServer m_oHttpServer;
    char m_szBuffer[CHttpServer::HTTP_REQ_DATA_BUF_SIZE];

private:
    DT_SERVER_LIST m_stSvrList;
};

