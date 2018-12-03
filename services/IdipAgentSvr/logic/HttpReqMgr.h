#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/IdipAgentSvrCfgDesc.h"
#include "thread/HttpClientThread.h"
#include "thread/HttpServerThread.h"
#include <map>
#include <set>

class HttpReqMgr : public TSingleton<HttpReqMgr>
{
private:
    typedef std::map<uint64_t, struct evhttp_request*> HttpRequestMap_t;

public:
    int Init(IDIPAGENTSVRCFG* pstConfig);
    int Fini();

    void RegToIdipSvr();
    void UnRegToIdipSvr();
    void AddToHttpReqMap(uint64_t ullUin, struct evhttp_request* poReq);
    void DelFromHttpReqMap(uint64_t ullUin);
    struct evhttp_request* GetRequestByUin(uint64_t ullUin);
    bool CheckReqUri(struct evhttp_request *req);

private:
    CHttpClient m_oHttpClient;
    char m_szPostInfo[PKGMETA::MAX_LEN_HTTPPOST_INFO];

private:
    HttpRequestMap_t m_oHttpRequestMap;
    HttpRequestMap_t::iterator m_oHttpRequestIter;

private:
    IDIPAGENTSVRCFG* m_pstConfig;
};