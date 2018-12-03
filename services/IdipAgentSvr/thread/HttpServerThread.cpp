#include "oi_misc.h"
#include "LogMacros.h"
#include "HttpServerThread.h"
#include "IdipAgentSvrMsgLayer.h"
#include "idip_proto.h"
#include "IdipAgentSvr.h"
#include "HttpReqMgr.h"
#include "strutil.h"

using namespace PKGMETA;

HttpServerThread::HttpServerThread() : m_stSendBuf(sizeof(SSPKG) * 2 + 1)
{

}

void HttpServerThread::SendPkg(SSPKG& rstSsPkg)
{
    rstSsPkg.m_stHead.m_iSrcProcId = m_pstConfig->m_iProcID;

    TdrError::ErrorType iRet = rstSsPkg.pack(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uSize, &m_stSendBuf.m_uPackLen);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack ss pkg error! MsgId(%u)", rstSsPkg.m_stHead.m_wMsgId);
        return;
    }

    m_oRspQueue.WriteMsg(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uPackLen);
}

bool HttpServerThread::_ThreadAppInit(void* pvAppData)
{
    m_pstConfig = (IDIPAGENTSVRCFG*)pvAppData;

    if (!m_oHttpServer.Init(m_pstConfig->m_szHttpSvrPriIp, (uint16_t)m_pstConfig->m_iHttpSvrPort))
    {
        LOGERR_r("Init HttpServer failed ip<%s>, port<%d>", m_pstConfig->m_szHttpSvrPriIp, m_pstConfig->m_iHttpSvrPort);
        return false;
    }

    if (!m_oHttpServer.AddRequestCb(m_pstConfig->m_szGmQueryUri, HttpServerThread::_HandleGmQuery, this))
    {
        LOGERR_r("Add GmQuery CallBack failed");
        return false;
    }

    if (!m_oHttpServer.AddRequestCb(m_pstConfig->m_szGmOpUri, HttpServerThread::_HandleGmOp, this))
    {
        LOGERR_r("Add GmOp CallBack failed");
        return false;
    }

    LPTDRMETALIB pstMetaLib;
    int iRet = tdr_load_metalib(&pstMetaLib, "../../protocol/idip_proto.bin" );

    if (TDR_ERR_IS_ERROR(iRet))
    {
       LOGERR_r("load metalib idip_proto.bin failed! (%s)", tdr_error_string(iRet));
       assert(false);
    }

    m_pstGmQueryDataMeta = tdr_get_meta_by_name(pstMetaLib, "IDIP_PKG_GM_GET_PLAYER_LOG_REQ");

    assert(m_pstGmQueryDataMeta);

    m_ullKey = 0;

    return true;
}

void HttpServerThread::_ThreadAppFini()
{

}

int HttpServerThread::_ThreadAppProc()
{
    m_oHttpServer.RecvAndHandle();

    return 0;
}

void HttpServerThread::_HandleGmQuery(struct evhttp_request *req, void *arg)
{
    //只接受post请求
    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST)
    {
       LOGERR_r("http type error");
       return;
    }

    if (!HttpReqMgr::Instance().CheckReqUri(req))
    {
        LOGERR_r("the request source uri is ilegal.");
        return;
    }

    HttpServerThread* poHttpThread = (HttpServerThread*)arg;
    CHttpServer* poHttpServer = &poHttpThread->m_oHttpServer;
    struct evbuffer *evbuf;
    evbuf = evhttp_request_get_input_buffer(req);
    int iLen = evbuffer_copyout(evbuf, poHttpServer->m_szReqDataBuf, (CHttpServer::HTTP_REQ_DATA_BUF_SIZE - 1));

    if (iLen < 0)
    {
       LOGERR_r("evbuffer_copyout failed, n=%d", iLen);
       return;
    }

    poHttpServer->m_szReqDataBuf[iLen]='\0';

    //解析SDK返回数据
    IdipPkg stPkgIdipReq;
    stPkgIdipReq.m_stHead.m_wMsgId = IDIP_MSG_GM_GET_PLAYER_LOG_REQ;
    IDIP_PKG_GM_GET_PLAYER_LOG_REQ& rstIdipPkgBody = stPkgIdipReq.m_stBody.m_stGmGetPlayerLogReq;
    TDRDATA stHost, stJson;
    stJson.pszBuff = poHttpServer->m_szReqDataBuf;
    stJson.iBuff = CHttpServer::HTTP_REQ_DATA_BUF_SIZE;
    stHost.pszBuff = (char*)&stPkgIdipReq.m_stBody.m_stGmGetPlayerLogReq;
    stHost.iBuff = sizeof(stPkgIdipReq.m_stBody.m_stGmGetPlayerLogReq);
    SSPKG stSsPkg;
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_IDIP_REQ;
    stSsPkg.m_stHead.m_ullUin = poHttpThread->_GenUniqeKey();
    SS_PKG_IDIP_REQ& rstIdipReqBody = stSsPkg.m_stBody.m_stIdipReq;

    int iRet = tdr_input_json(poHttpThread->m_pstGmQueryDataMeta, &stHost, &stJson, 0);
    LOGERR_r("uin<%lu>, stime<%s>, type<%d>, gmuin<%lu>", rstIdipPkgBody.m_ullUin, rstIdipPkgBody.m_szStartTime, rstIdipPkgBody.m_bType, stSsPkg.m_stHead.m_ullUin);
    if (TDR_ERR_IS_ERROR(iRet))
    {
       LOGERR_r("json(%s) to bin failed. (%s)", poHttpServer->m_szReqDataBuf, tdr_error_string(iRet));
       return;
    }
    else
    {
       LOGERR_r("handle client request (%s) sucess", poHttpServer->m_szReqDataBuf);
       size_t uUsedSize = 0;
       int iRet = stPkgIdipReq.pack((char*)rstIdipReqBody.m_szData, sizeof(IdipPkg), &uUsedSize);
       
       if (iRet != TdrError::TDR_NO_ERROR)
       {
         LOGERR_r("stPkgIdipReq pack failed");
         return;
       }
       rstIdipReqBody.m_dwLen = (uint32_t)uUsedSize;
       LOGERR_r("len1<%d>, len2<%d>", sizeof(rstIdipReqBody.m_szData), rstIdipReqBody.m_dwLen);
       poHttpThread->SendPkg(stSsPkg);
    }
    
    HttpReqMgr::Instance().AddToHttpReqMap(stSsPkg.m_stHead.m_ullUin, req);
}

// void HttpServerThread::_HandleGmOp(struct evhttp_request *req, void *arg)
// {
//     //只接受post请求
//     if (evhttp_request_get_command(req) != EVHTTP_REQ_POST)
//     {
//        LOGERR_r("http type error");
//        return;
//     }

//     if (!HttpReqMgr::Instance().CheckReqUri(req))
//     {
//         LOGERR_r("the request source uri is ilegal.");
//         return;
//     }

//     HttpServerThread* poHttpThread = (HttpServerThread*)arg;
//     CHttpServer* poHttpServer = &poHttpThread->m_oHttpServer;
//     struct evbuffer *evbuf;
//     evbuf = evhttp_request_get_input_buffer(req);
//     int iLen = evbuffer_copyout(evbuf, poHttpServer->m_szReqDataBuf, (CHttpServer::HTTP_REQ_DATA_BUF_SIZE - 1));

//     if (iLen < 0)
//     {
//         LOGERR_r("evbuffer_copyout failed, n=%d", iLen);
//         return;
//     }

//     poHttpServer->m_szReqDataBuf[iLen]='\0';

//     //解析SDK返回数据
//     SSPkg stPkgGmReq;
//     stPkgGmReq.m_stHead.m_wMsgId = SS_MSG_GM_REQ;
//     stPkgGmReq.m_stHead.m_ullUin = poHttpThread->_GenUniqeKey();
//     SS_PKG_GM_REQ& rstPkgGmBody = stPkgGmReq.m_stBody.m_stGmReq;
//     if (rstPkgGmBody.unpack(poHttpServer->m_szReqDataBuf, iLen) != TdrError::TDR_NO_ERROR)
//     {
//         LOGERR_r("unpack rstPkgGmBody failed. iLen<%d>", iLen);
//         return;
//     }
//     else
//     {
//         LOGRUN_r("GmName<%s>, Type<%d>", rstPkgGmBody.m_szName, rstPkgGmBody.m_bType);
//         poHttpThread->SendPkg(stPkgGmReq);
//     }

//     HttpReqMgr::Instance().AddToHttpReqMap(stPkgGmReq.m_stHead.m_ullUin, req);
// }

uint64_t HttpServerThread::_GenUniqeKey()
{
    //前32位用作进程编号，后32为为id
    uint64_t ullRetKey = m_iThreadIdx << 32;
    ullRetKey += (m_ullKey++) % 0xFFFFFFFF;

    return ullRetKey;
}

void HttpServerThread::_HandleGmOp(struct evhttp_request *req, void *arg)
{
	LOGRUN_r("handle gm op--");
    //只接受post请求
    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST)
    {
        LOGERR_r("http type error");
        return;
    }

    if (!HttpReqMgr::Instance().CheckReqUri(req))
    {
        LOGERR_r("the request source uri is ilegal.");
        return;
    }

    HttpServerThread* poHttpThread = (HttpServerThread*)arg;
    CHttpServer* poHttpServer = &poHttpThread->m_oHttpServer;
    struct evbuffer *evbuf;
    evbuf = evhttp_request_get_input_buffer(req);
    int iLen = evbuffer_copyout(evbuf, poHttpServer->m_szReqDataBuf, (CHttpServer::HTTP_REQ_DATA_BUF_SIZE - 1));

    if (iLen < 0)
    {
        LOGERR_r("evbuffer_copyout failed, n=%d", iLen);
        return;
    }

    poHttpServer->m_szReqDataBuf[iLen]='\0';

    //解析SDK返回数据
    SSPkg stSsPkg;
    if(stSsPkg.unpack(poHttpServer->m_szReqDataBuf, iLen) != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("unpack stSsPkg failed. iLen<%d>", iLen);
        return;
    }
    stSsPkg.m_stHead.m_ullUin = poHttpThread->_GenUniqeKey();

    switch (stSsPkg.m_stHead.m_wMsgId)
    {
    case SS_MSG_GM_REQ:
        LOGRUN_r("GmName<%s>, Type<%d>", stSsPkg.m_stBody.m_stGmReq.m_szName, stSsPkg.m_stBody.m_stGmReq.m_bType);
        poHttpThread->SendPkg(stSsPkg);
        break;
    case SS_MSG_IDIP_REQ:
        LOGRUN_r("");
        poHttpThread->SendPkg(stSsPkg);
        break;
	case SS_MSG_DATABASE_OP:
		//LOGRUN_r("GMNAME<%lld>,type<%s%d>  ", stSsPkg.m_stBody.m_stGmDelprops.m_ullGmname, "删除物品", stSsPkg.m_stBody.m_stGmDelprops.m_dwItemID);
		poHttpThread->SendPkg(stSsPkg);
		break;
    default:
        LOGERR_r("msgid<%d>", stSsPkg.m_stHead.m_wMsgId);
    }

    HttpReqMgr::Instance().AddToHttpReqMap(stSsPkg.m_stHead.m_ullUin, req);
}
