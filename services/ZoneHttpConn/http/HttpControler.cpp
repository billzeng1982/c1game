#include "HttpControler.h"
#include "utils/strutil.h"
#include "framework/GameObjectPool.h"
#include "framework/ZoneHttpConnMsgLayer.h"
#include "event2/keyvalq_struct.h"
#include "GameTime.h"
#include "LogMacros.h"

using namespace PKGMETA;

const int HttpControler::DEFAULT_HTTP_REQ_TIMER_NUM = 100;
const int HttpControler::HTTP_REQ_TIMER_TIMEOUT_TIME = 5;

struct evkeyvalq params;

static void HttpGetRoleWebTaskInfo(struct evhttp_request *req, void *arg)
{
    HttpControler* poHttpControler = static_cast<HttpControler*>(arg);
    CHttpServer* poHttpServer = poHttpControler->GetHttpServer();

    if (evhttp_request_get_command(req) == EVHTTP_REQ_POST)
    {
        struct evbuffer *evbuf;
        evbuf = evhttp_request_get_input_buffer(req);
        int n = evbuffer_copyout(evbuf, poHttpServer->m_szReqDataBuf, (CHttpServer::HTTP_REQ_DATA_BUF_SIZE - 1));
        if (n <= 0)
        {
            LOGERR("copy out req data error! n=%d\n", n);
            evhttp_request_free(req);
            return;
        }
        poHttpServer->m_szReqDataBuf[n] = '\0';

        poHttpControler->OnHttpGetRoleWebTaskInfo(req, poHttpServer->m_szReqDataBuf);
        return;
    }

    return;
}

static void HttpGetWebTaskRwd(struct evhttp_request *req, void *arg)
{
    HttpControler* poHttpControler = static_cast<HttpControler*>(arg);
    CHttpServer* poHttpServer = poHttpControler->GetHttpServer();

    if (evhttp_request_get_command(req) == EVHTTP_REQ_POST)
    {
        struct evbuffer *evbuf;
        evbuf = evhttp_request_get_input_buffer(req);
        int n = evbuffer_copyout(evbuf, poHttpServer->m_szReqDataBuf, (CHttpServer::HTTP_REQ_DATA_BUF_SIZE - 1));
        if (n <= 0)
        {
            LOGERR("copy out req data error! n=%d\n", n);
            evhttp_request_free(req);
            return;
        }
        poHttpServer->m_szReqDataBuf[n] = '\0';

        poHttpControler->OnHttpGetWebTaskRwd(req, poHttpServer->m_szReqDataBuf);
        return;
    }
    else if(evhttp_request_get_command(req) == EVHTTP_REQ_GET)
    {
        char *decoded_uri;
        decoded_uri = evhttp_decode_uri(evhttp_request_get_uri(req));
        int iRet = evhttp_parse_query(decoded_uri, &params);
        if (iRet != 0)
        {
            LOGERR("uri not well formatted");
            evhttp_request_free(req);
            return;
        }
        SS_PKG_WEB_TASK_GET_RWD_REQ stSsReq;
        if (evhttp_find_header(&params, "Uin") == NULL || 
            evhttp_find_header(&params, "TaskId") == NULL ||
            evhttp_find_header(&params, "SvrId") == NULL)
        {
            LOGERR("uri not well formatted");
            evhttp_request_free(req);
            return;
        }
        stSsReq.m_ullUin = atoll(evhttp_find_header(&params, "Uin"));
        stSsReq.m_dwTaskId = atoi(evhttp_find_header(&params, "TaskId"));
        stSsReq.m_bSvrId = atoi(evhttp_find_header(&params, "SvrId"));

        poHttpControler->OnHttpGetWebTaskRwd(req, stSsReq);
        return;
    }
    return;
}

HttpControler::HttpControler() : m_stTdrBuf(2048)
{
    m_dwReqId = 0;
}

void HttpControler::Update()
{
    m_oTimerMgr.Update();
    m_oHttpServer.RecvAndHandle();
}

bool HttpControler::Init(ZONEHTTPCONNCFG& rstConfig)
{
    m_stConfig = rstConfig;

    bool bRet = m_oHttpServer.Init(m_stConfig.m_szHttpSvrAddr, m_stConfig.m_wHttpSvrPort);
    if (!bRet)
    {
        LOGERR("init error! http addr: %s, port: %d", m_stConfig.m_szHttpSvrAddr, m_stConfig.m_wHttpSvrPort);
        return false;
    }

    m_oHttpServer.AddRequestCb(m_stConfig.m_szGetRoleWebTaskInfo, HttpGetRoleWebTaskInfo, this);
    m_oHttpServer.AddRequestCb(m_stConfig.m_szGetWebTaskRwd, HttpGetWebTaskRwd, this);

    int iRet = tdr_load_metalib(&m_pstMetaLib, "../../protocol/ss_proto.bin");
    if (TDR_ERR_IS_ERROR(iRet))
    {
        LOGERR("load metalib idip_proto.bin failed! (%s)", tdr_error_string(iRet));
        return false;
    }

    OnReleaseTimerCb_t fReleaseTimer = HttpControler::ReleaseTimer;
    if (!m_oTimerMgr.Init(DEFAULT_HTTP_REQ_TIMER_NUM, fReleaseTimer))
    {
        LOGERR("init HttpReqTimer failed");
        return false;
    }

    return true;
}

void HttpControler::OnHttpGetRoleWebTaskInfo(evhttp_request * req, char * szReqJson)
{
    assert(req != NULL && !StrUtil::IsStringNull(szReqJson));

    // json 2 tdr
    LPTDRMETA pstMeta = tdr_get_meta_by_name(m_pstMetaLib, "SS_PKG_ROLE_WEB_TASK_INFO_REQ");
    assert(pstMeta);

    TDRDATA stHost, stJson;
    stJson.pszBuff = szReqJson;
    stJson.iBuff = CHttpServer::HTTP_REQ_DATA_BUF_SIZE;
    stHost.pszBuff = (char*)&m_stSsPkg.m_stBody.m_stRoleWebTaskInfoReq;
    stHost.iBuff = sizeof(m_stSsPkg.m_stBody.m_stRoleWebTaskInfoReq);

    int iRet = tdr_input_json_ex(pstMeta, &stHost, &stJson, 0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        LOGERR("json(%s) to bin failed. (%s)", szReqJson, tdr_error_string(iRet));
        // send errmsg!
        this->_SendErrRsp(req, ERR_WRONG_PARA);
        return;
    }

    HttpReqTimer* poTimer = GET_GAMEOBJECT(HttpReqTimer, GAMEOBJ_HTTP_REQ_TIMER);
    poTimer->AttatchParam(this->_GetNextReqId(), SS_MSG_ROLE_WEB_TASK_INFO_REQ);
    struct timeval tFirstFireTime = *(CGameTime::Instance().GetCurrTime());
    tFirstFireTime.tv_sec += HTTP_REQ_TIMER_TIMEOUT_TIME;
    poTimer->SetTimerAttr(tFirstFireTime);
    m_oTimerMgr.AddTimer(poTimer);
    this->_AddToHttpTimerMap(m_dwReqId, poTimer->GetObjID());
    this->_AddToHttpReqMap(m_dwReqId, req);

    LOGRUN("Recieve player<%lu> get web task info req, send to ZoneSvr", m_stSsPkg.m_stBody.m_stRoleWebTaskInfoReq.m_ullUin);

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ROLE_WEB_TASK_INFO_REQ;
    m_stSsPkg.m_stHead.m_ullUin = m_stSsPkg.m_stBody.m_stRoleWebTaskInfoReq.m_ullUin;
    m_stSsPkg.m_stHead.m_ullReservId = m_dwReqId;
    ZoneHttpConnMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
}

void HttpControler::OnHttpGetWebTaskRwd(evhttp_request * req, char * szReqJson)
{
    assert(req != NULL && !StrUtil::IsStringNull(szReqJson));

    // json 2 tdr
    LPTDRMETA pstMeta = tdr_get_meta_by_name(m_pstMetaLib, "SS_PKG_WEB_TASK_GET_RWD_REQ");
    assert(pstMeta);

    TDRDATA stHost, stJson;
    stJson.pszBuff = szReqJson;
    stJson.iBuff = CHttpServer::HTTP_REQ_DATA_BUF_SIZE;
    stHost.pszBuff = (char*)&m_stSsPkg.m_stBody.m_stRoleGetWebTaskRwdReq;
    stHost.iBuff = sizeof(m_stSsPkg.m_stBody.m_stRoleGetWebTaskRwdReq);

    int iRet = tdr_input_json_ex(pstMeta, &stHost, &stJson, 0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        LOGERR("json(%s) to bin failed. (%s)", szReqJson, tdr_error_string(iRet));
        // send errmsg!
        this->_SendErrRsp(req, ERR_WRONG_PARA);
        return;
    }

    HttpReqTimer* poTimer = GET_GAMEOBJECT(HttpReqTimer, GAMEOBJ_HTTP_REQ_TIMER);
    poTimer->AttatchParam(this->_GetNextReqId(), SS_MSG_WEB_TASK_GET_RWD_REQ);
    struct timeval tFirstFireTime = *(CGameTime::Instance().GetCurrTime());
    tFirstFireTime.tv_sec += HTTP_REQ_TIMER_TIMEOUT_TIME;
    poTimer->SetTimerAttr(tFirstFireTime);
    m_oTimerMgr.AddTimer(poTimer);
    this->_AddToHttpTimerMap(m_dwReqId, poTimer->GetObjID());
    this->_AddToHttpReqMap(m_dwReqId, req);

    LOGRUN("Recieve player<%lu> get web reward req<%d>, method<GET>, send to ZoneSvr", m_stSsPkg.m_stBody.m_stRoleGetWebTaskRwdReq.m_ullUin, m_dwReqId);

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_WEB_TASK_GET_RWD_REQ;
    m_stSsPkg.m_stHead.m_ullUin = m_stSsPkg.m_stBody.m_stRoleGetWebTaskRwdReq.m_ullUin;
    m_stSsPkg.m_stHead.m_ullReservId = m_dwReqId;
    ZoneHttpConnMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
}

void HttpControler::OnHttpGetWebTaskRwd(evhttp_request * req, PKGMETA::SS_PKG_WEB_TASK_GET_RWD_REQ & rstSsReq)
{
    HttpReqTimer* poTimer = GET_GAMEOBJECT(HttpReqTimer, GAMEOBJ_HTTP_REQ_TIMER);
    poTimer->AttatchParam(this->_GetNextReqId(), SS_MSG_WEB_TASK_GET_RWD_REQ);
    struct timeval tFirstFireTime = *(CGameTime::Instance().GetCurrTime());
    tFirstFireTime.tv_sec += HTTP_REQ_TIMER_TIMEOUT_TIME;
    poTimer->SetTimerAttr(tFirstFireTime);
    m_oTimerMgr.AddTimer(poTimer);
    this->_AddToHttpTimerMap(m_dwReqId, poTimer->GetObjID());
    this->_AddToHttpReqMap(m_dwReqId, req);

    LOGRUN("Recieve player<%lu> get web reward req<%d>, method<GET>, send to ZoneSvr", rstSsReq.m_ullUin, m_dwReqId);

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_WEB_TASK_GET_RWD_REQ;
    m_stSsPkg.m_stHead.m_ullUin = rstSsReq.m_ullUin;
    m_stSsPkg.m_stHead.m_ullReservId = m_dwReqId;
    m_stSsPkg.m_stBody.m_stRoleGetWebTaskRwdReq = rstSsReq;
    ZoneHttpConnMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
}

void HttpControler::ReleaseTimer(GameTimer * pTimer)
{
    if (!pTimer)
    {
        LOGERR("pTimer is NULL");
        return;
    }

    LOGRUN("release HttpReq timer<%d>", pTimer->GetObjID());
    RELEASE_GAMEOBJECT(pTimer);
}

bool HttpControler::DeleteReq(uint32_t dwReqId)
{
    if (0 == dwReqId)
    {
        return false;
    }
    HttpReqMap_t::iterator it = m_oHttpReqMap.find(dwReqId);
    if (it != m_oHttpReqMap.end())
    {
        LOGRUN("Delete req<%d> from map", dwReqId);
        m_oHttpReqMap.erase(it);
    }
    return true;
}

bool HttpControler::DeleteTimer(uint32_t dwReqId)
{
    HttpReqTimerIdMap_t::iterator iterator = m_oHttpTimerMap.find(dwReqId);
    if (iterator != m_oHttpTimerMap.end())
    {
        m_oTimerMgr.DelTimer(iterator->second);
        m_oHttpTimerMap.erase(iterator);
        return true;
    }
    return false;
}

evhttp_request * HttpControler::_FindReq(uint32_t dwReqId)
{
    HttpReqMap_t::iterator it = m_oHttpReqMap.find(dwReqId);
    if (it != m_oHttpReqMap.end())
    {
        return it->second;
    }
    return NULL;
}

void HttpControler::_AddToHttpReqMap(uint32_t dwReqId, evhttp_request * req)
{
    if (NULL == req)
    {
        assert(false);
        return;
    }

    m_oHttpReqMap.insert(HttpReqMap_t::value_type(dwReqId, req));
    return;
}

void HttpControler::_AddToHttpTimerMap(uint32_t dwReqId, uint32_t dwTimerId)
{
    m_oHttpTimerMap.insert(HttpReqTimerIdMap_t::value_type(dwReqId, dwTimerId));
    return;
}

void HttpControler::_SendGetRoleWebTaskInfoHttpTimeoutRsp(uint32_t dwReqId, PKGMETA::SS_PKG_ROLE_WEB_TASK_INFO_RSP & rstRoleWebTaskInfoRsp)
{
    struct evhttp_request* req = this->_FindReq(dwReqId);
    if (req == NULL)
    {
        return;
    }
    rstRoleWebTaskInfoRsp.m_nErrno = ERR_TIME_OUT;

    TDRDATA stHost, stJson;
    char szJsonRsp[512];
    stJson.pszBuff = szJsonRsp;
    stJson.iBuff = sizeof(szJsonRsp);
    stHost.pszBuff = (char*)&rstRoleWebTaskInfoRsp;
    stHost.iBuff = sizeof(rstRoleWebTaskInfoRsp);
    LPTDRMETA pstMeta = tdr_get_meta_by_name(m_pstMetaLib, "SS_PKG_ROLE_WEB_TASK_INFO_RSP");
    int iRet = tdr_output_json(pstMeta, &stJson, &stHost, 0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        assert(false);
        LOGERR("bin to json failed. (%s)", tdr_error_string(iRet));
        snprintf(szJsonRsp, sizeof(szJsonRsp), "{\"ErrNo\":%d}", ERR_SYS);
    }

    this->_SendHttpRsp(req, szJsonRsp);

    DeleteReq(dwReqId);
    DeleteTimer(dwReqId);
}

void HttpControler::_SendGetRoleWebTaskRwdHttpTimeoutRsp(uint32_t dwReqId, PKGMETA::SS_PKG_WEB_TASK_GET_RWD_RSP & rstWebTaskGetRwdRsp)
{
    struct evhttp_request* req = this->_FindReq(dwReqId);
    if (req == NULL)
    {
        return;
    }

    rstWebTaskGetRwdRsp.m_nErrno = ERR_TIME_OUT;

    TDRDATA stHost, stJson;
    char szJsonRsp[512];
    stJson.pszBuff = szJsonRsp;
    stJson.iBuff = sizeof(szJsonRsp);
    stHost.pszBuff = (char*)&rstWebTaskGetRwdRsp;
    stHost.iBuff = sizeof(rstWebTaskGetRwdRsp);
    LPTDRMETA pstMeta = tdr_get_meta_by_name(m_pstMetaLib, "SS_PKG_WEB_TASK_GET_RWD_RSP");
    int iRet = tdr_output_json(pstMeta, &stJson, &stHost, 0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        assert(false);
        LOGERR("bin to json failed. (%s)", tdr_error_string(iRet));
        snprintf(szJsonRsp, sizeof(szJsonRsp), "{\"ErrNo\":%d}", ERR_SYS);
    }

    this->_SendHttpRsp(req, szJsonRsp);

    DeleteReq(dwReqId);
    DeleteTimer(dwReqId);
}

void HttpControler::SendGetRoleWebTaskInfoHttpRsp(uint32_t dwReqId, SS_PKG_ROLE_WEB_TASK_INFO_RSP & rstRoleWebTaskInfoRsp)
{
    struct evhttp_request* req = this->_FindReq(dwReqId);
    if (req == NULL)
    {
        return;
    }

    TDRDATA stHost, stJson;
    memset(m_szJsonRsp, 0, sizeof(m_szJsonRsp));
    stJson.pszBuff = m_szJsonRsp;
    stJson.iBuff = sizeof(m_szJsonRsp);
    stHost.pszBuff = (char*)&rstRoleWebTaskInfoRsp;
    stHost.iBuff = sizeof(rstRoleWebTaskInfoRsp);
    LPTDRMETA pstMeta = tdr_get_meta_by_name(m_pstMetaLib, "SS_PKG_ROLE_WEB_TASK_INFO_RSP");
    int iRet = tdr_output_json(pstMeta, &stJson, &stHost, 0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        assert(false);
        LOGERR("bin to json failed. (%s)", tdr_error_string(iRet));
        snprintf(m_szJsonRsp, sizeof(m_szJsonRsp), "{\"ErrNo\":%d}", ERR_SYS);
    }

    iRet = evhttp_add_header(evhttp_request_get_output_headers(req), "Access-Control-Allow-Origin", "*");
    if (iRet != 0)
    {
        LOGERR("http add header failed");
        evhttp_request_free(req);
        DeleteReq(dwReqId);
        DeleteTimer(dwReqId);
        return;
    }

    this->_SendHttpRsp(req, m_szJsonRsp);

    DeleteReq(dwReqId);
    DeleteTimer(dwReqId);
}

void HttpControler::SendGetRoleWebTaskRwdHttpRsp(uint32_t dwReqId, PKGMETA::SS_PKG_WEB_TASK_GET_RWD_RSP & rstWebTaskGetRwdRsp)
{
    struct evhttp_request* req = this->_FindReq(dwReqId);
    if (req == NULL)
    {
        return;
    }

    int iRet = evhttp_add_header(evhttp_request_get_output_headers(req), "Access-Control-Allow-Origin", "*");
    if (iRet != 0)
    {
        LOGERR("http add header failed");
        evhttp_request_free(req);
        DeleteReq(dwReqId);
        DeleteTimer(dwReqId);
        return;
    }

    TDRDATA stHost, stJson;
    memset(m_szJsonRsp, 0, sizeof(m_szJsonRsp));
    stJson.pszBuff = m_szJsonRsp;
    stJson.iBuff = sizeof(m_szJsonRsp);
    stHost.pszBuff = (char*)&rstWebTaskGetRwdRsp;
    stHost.iBuff = sizeof(rstWebTaskGetRwdRsp);
    LPTDRMETA pstMeta = tdr_get_meta_by_name(m_pstMetaLib, "SS_PKG_WEB_TASK_GET_RWD_RSP");
    iRet = tdr_output_json(pstMeta, &stJson, &stHost, 0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        assert(false);
        LOGERR("bin to json failed. (%s)", tdr_error_string(iRet));
        snprintf(m_szJsonRsp, sizeof(m_szJsonRsp), "{\"ErrNo\":%d}", ERR_SYS);
    }

    this->_SendHttpRsp(req, m_szJsonRsp);

    DeleteReq(dwReqId);
    DeleteTimer(dwReqId);
}

void HttpControler::SendHttpRsp(SSPKG& rstSsPkg)
{
    switch (rstSsPkg.m_stHead.m_wMsgId)
    {
    case SS_MSG_ROLE_WEB_TASK_INFO_RSP:
        this->SendGetRoleWebTaskInfoHttpRsp(static_cast<uint32_t>(rstSsPkg.m_stHead.m_ullReservId), rstSsPkg.m_stBody.m_stRoleWebTaskInfoRsp);
        break;
    case SS_MSG_WEB_TASK_GET_RWD_RSP:
        this->SendGetRoleWebTaskRwdHttpRsp(static_cast<uint32_t>(rstSsPkg.m_stHead.m_ullReservId), rstSsPkg.m_stBody.m_stRoleGetWebTaskRwdRsp);
        break;
    default:
        break;
    }
}

void HttpControler::SendTimeoutRsp(PKGMETA::SSPKG & rstSsPkg)
{
    switch (rstSsPkg.m_stHead.m_wMsgId)
    {
    case SS_MSG_ROLE_WEB_TASK_INFO_RSP:
        this->_SendGetRoleWebTaskInfoHttpTimeoutRsp(static_cast<uint32_t>(rstSsPkg.m_stHead.m_ullReservId), rstSsPkg.m_stBody.m_stRoleWebTaskInfoRsp);
        break;
    case SS_MSG_WEB_TASK_GET_RWD_RSP:
        this->_SendGetRoleWebTaskRwdHttpTimeoutRsp(static_cast<uint32_t>(rstSsPkg.m_stHead.m_ullReservId), rstSsPkg.m_stBody.m_stRoleGetWebTaskRwdRsp);
        break;
    default:
        break;
    }
}

void HttpControler::_SendErrRsp(struct evhttp_request *req, int iErrNo)
{
    char szJsonRsp[512];
    snprintf(szJsonRsp, sizeof(szJsonRsp), "{\"ErrNo\":%d}", iErrNo);
    this->_SendHttpRsp(req, szJsonRsp);
}

void HttpControler::_SendHttpRsp(struct evhttp_request *req, char* szRspStr)
{
    assert(req);
    
    evbuffer_add(m_oHttpServer.m_pEvRspData, szRspStr, strlen(szRspStr));
    /*
    After calling evhttp_send_reply() databuf will be empty, but the buffer is still
    owned by the caller and needs to be deallocated by the caller if necessary.
    */
    evhttp_send_reply(req, HTTP_OK, "ok", m_oHttpServer.m_pEvRspData);
}
