#include "oi_misc.h"
#include "LogMacros.h"
#include "HttpServerThread.h"
#include "../framework/XiYouSDKSvrMsgLayer.h"

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
    m_pstConfig = (XIYOUSDKSVRCFG*)pvAppData;

    if (!m_oHttpServer.Init("0.0.0.0", (uint16_t)m_pstConfig->m_iHttpSvrPort))
    {
        LOGERR_r("Init HttpServer failed");
        return false;
    }

    if (!m_oHttpServer.AddRequestCb(m_pstConfig->m_szPayCallBackUri, HttpServerThread::_HandlePayCb, this))
    {
        LOGERR_r("Add Pay CallBack failed");
        return false;
    }

    LPTDRMETALIB pstMetaLib;
    int iRet = tdr_load_metalib(&pstMetaLib, "../../protocol/common_proto.bin" );
    if (TDR_ERR_IS_ERROR(iRet))
	{
		LOGERR_r("load metalib common_proto.bin failed! (%s)", tdr_error_string(iRet));
		assert(false);
	}
    m_pstPayCbDataMeta = tdr_get_meta_by_name(pstMetaLib, "DT_SDK_PAY_CB");
    assert(m_pstPayCbDataMeta);

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


void HttpServerThread::_HandlePayCb(struct evhttp_request *req, void *arg)
{
    //只接受post请求
    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST)
    {
		LOGERR_r("http type error");
		return;
	}
    HttpServerThread* poHttpThread = (HttpServerThread*)arg;
    CHttpServer* poHttpServer = &poHttpThread->m_oHttpServer;
    struct evbuffer *evbuf;
    evbuf = evhttp_request_get_input_buffer(req);
    int iLen = evbuffer_copyout(evbuf, poHttpServer->m_szReqDataBuf, (CHttpServer::HTTP_REQ_DATA_BUF_SIZE-1));
    if (iLen < 0)
    {
        LOGERR_r("evbuffer_copyout failed, n=%d", iLen);
        return;
    }
    poHttpServer->m_szReqDataBuf[iLen]='\0';

    LOGRUN_r("handle SdkPayCb req, postinfo=(%s)", poHttpServer->m_szReqDataBuf);

    SSPKG& stSsPkg = poHttpThread->m_stSsPkg;
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_SDK_PAY_CB_NTF;
    SS_PKG_SDK_PAY_CB_NTF& rstPayCbNtf = stSsPkg.m_stBody.m_stSDKPayCbNtf;

    //解析SDK返回数据
    TDRDATA stHost, stJson;
    DT_SDK_PAY_CB stSdkPayCb;
    stJson.pszBuff = poHttpServer->m_szReqDataBuf;
    stJson.iBuff = CHttpServer::HTTP_REQ_DATA_BUF_SIZE;
    stHost.pszBuff = (char*)&stSdkPayCb;
    stHost.iBuff = sizeof(stSdkPayCb);

    const char* pszRsp = "SUCCESS";
    int iRet = tdr_input_json(poHttpThread->m_pstPayCbDataMeta, &stHost, &stJson, 0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        pszRsp = "FAIL";
    	    LOGERR_r("json(%s) to bin failed. (%s)", poHttpServer->m_szReqDataBuf, tdr_error_string(iRet));
    }
    else
    {
        LOGRUN_r("handle SdkPayCb sucess");
        rstPayCbNtf.m_stPayCbInfo = stSdkPayCb;
        poHttpThread->SendPkg(stSsPkg);
    }

    evbuffer_add(poHttpServer->m_pEvRspData, pszRsp, strlen(pszRsp));
    evhttp_send_reply(req, HTTP_OK, "OK", poHttpServer->m_pEvRspData);
    evbuffer_remove(poHttpServer->m_pEvRspData, poHttpServer->m_szReqDataBuf, CHttpServer::HTTP_REQ_DATA_BUF_SIZE);
    poHttpServer->m_szReqDataBuf[0]='\0';
}

