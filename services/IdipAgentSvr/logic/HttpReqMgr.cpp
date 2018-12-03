#include "HttpReqMgr.h"
#include "HttpServer.h"

int HttpReqMgr::Init(IDIPAGENTSVRCFG* pstConfig)
{
    m_pstConfig = pstConfig;    

    //初始化HttpClient
    m_oHttpClient.ChgUrl(m_pstConfig->m_szIdipAgentRegUrl);
    m_oHttpClient.HeaderAppend( NULL );

    return true;
}

int HttpReqMgr::Fini()
{
    return true;
}

void HttpReqMgr::AddToHttpReqMap(uint64_t ullUin, struct evhttp_request* poReq)
{
    if (NULL == poReq)
    {
       return;
    }

    m_oHttpRequestMap.insert(HttpRequestMap_t::value_type(ullUin, poReq));
}

void HttpReqMgr::DelFromHttpReqMap(uint64_t ullUin)
{
    m_oHttpRequestIter = m_oHttpRequestMap.find(ullUin);

    if (m_oHttpRequestIter != m_oHttpRequestMap.end())
    {
       m_oHttpRequestMap.erase(m_oHttpRequestIter);
    }
}

struct evhttp_request* HttpReqMgr::GetRequestByUin(uint64_t ullUin)
{
    m_oHttpRequestIter = m_oHttpRequestMap.find(ullUin);

    if (m_oHttpRequestIter != m_oHttpRequestMap.end())
    {
       return m_oHttpRequestIter->second;
    }

    return NULL;
}

void HttpReqMgr::RegToIdipSvr()
{
    static uint64_t LastRegTime = 0;

    if ((CGameTime::Instance().GetCurrSecond() - LastRegTime) > m_pstConfig->m_dwRegInterval)
    {
       LastRegTime = CGameTime::Instance().GetCurrSecond();
       m_szPostInfo[0] = '\0';
       sprintf(m_szPostInfo, "{\"id\":\"%d\", \"name\":\"%s\", \"status\":\"%d\", \"url\":\"%s:%d%s\"}",
         m_pstConfig->m_iSvrId, m_pstConfig->m_szSvrName, 1, m_pstConfig->m_szHttpSvrPubIp, m_pstConfig->m_iHttpSvrPort, m_pstConfig->m_szGmOpUri);

       LOGRUN_r("Send post req, PostInfo(%s)", m_szPostInfo);
       if (!m_oHttpClient.Post(m_szPostInfo))
       {
         LOGERR_r("Reg to IdipSvr failed, errinfo=(%s) <%s>", m_oHttpClient.GetLastErrInfo(), m_pstConfig->m_szIdipAgentRegUrl);
       }

       LOGRUN_r("Recv http Rsp, data=%s", m_oHttpClient.m_szRspDataBuf);
    }

}

void HttpReqMgr::UnRegToIdipSvr()
{
    m_szPostInfo[0] = '\0';
    sprintf(m_szPostInfo, "{\"id\":\"%d\", \"name\":\"%s\", \"status\":\"%d\", \"url\":\"%s:%d%s\"}",
         m_pstConfig->m_iSvrId, m_pstConfig->m_szSvrName, 0, m_pstConfig->m_szHttpSvrPubIp, m_pstConfig->m_iHttpSvrPort, m_pstConfig->m_szGmOpUri);

    LOGRUN_r("Send post req, PostInfo(%s)", m_szPostInfo);
    if (!m_oHttpClient.Post(m_szPostInfo))
    {
       LOGERR_r("Reg to IdipSvr failed, errinfo=(%s)", m_oHttpClient.GetLastErrInfo());
    }

    LOGRUN_r("Recv http Rsp, data=%s", m_oHttpClient.m_szRspDataBuf);
}

bool HttpReqMgr::CheckReqUri(struct evhttp_request *req)
{
    // return !strcmp(req->remote_host, m_pstConfig->m_szPassRemoteHost);
    return true;
}
