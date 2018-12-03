#include "IdipAgentMsgLogic.h"
#include "LogMacros.h"
#include "IdipAgentSvrMsgLayer.h"
#include "IdipAgentSvr.h"
#include "HttpClientThread.h"
#include "md5Util.h"
#include "strutil.h"
#include "tsec/md5.h"
#include "HttpServerThread.h"
#include "idip_proto.h"
#include "HttpReqMgr.h"

using namespace PKGMETA;

int LogQueryRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_IDIP_RSP& rstIdipRsp = rstSsPkg.m_stBody.m_stIdipRsp;
    
    IdipPkg stIdipPkg;
    stIdipPkg.m_stHead.m_wMsgId = IDIP_MSG_GM_GET_PLAYER_LOG_RSP;

    int iRet = stIdipPkg.unpack((char*)rstIdipRsp.m_szData, sizeof(rstIdipRsp.m_szData));

    if( iRet != TdrError::TDR_NO_ERROR)
    {
       LOGERR("unpack DT_ROLE_MAJESTY_BLOB failed!");
       return -1;
    }

    struct evhttp_request* poRequest = HttpReqMgr::Instance().GetRequestByUin(rstSsPkg.m_stHead.m_ullUin);

    if (NULL == poRequest)
    {
       LOGERR("poRequest is null");
       return -1;
    }

    int iCnt = stIdipPkg.m_stBody.m_stGmGetPlayerLogRsp.m_wCount;
    if (0 == iCnt)
    {
       evhttp_send_reply(poRequest, HTTP_OK, "OK", NULL);
    }
    else
    {
       evbuffer* poBufData = evbuffer_new();
       for (int i = 0; i < iCnt; i++)
       {
         evbuffer_add(poBufData, stIdipPkg.m_stBody.m_stGmGetPlayerLogRsp.m_aszLogInfo[i], strlen(stIdipPkg.m_stBody.m_stGmGetPlayerLogRsp.m_aszLogInfo[i])); //TODO
         LOGRUN("i<%d>, <%s>", i, stIdipPkg.m_stBody.m_stGmGetPlayerLogRsp.m_aszLogInfo[i]);
       }

       //reply and send to client
       evhttp_send_reply(poRequest, HTTP_OK, "OK", poBufData);
       evbuffer_free(poBufData);
    }

    return 0;
}

int GmRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    IDIP_GM_RSP_BLOB stGmBlob;

    SS_PKG_GM_RSP& rstGmRsp = rstSsPkg.m_stBody.m_stGmRsp;

    size_t ulUseSize = 0;
    TdrError::ErrorType iRet = rstGmRsp.pack((char*)stGmBlob.m_szData, sizeof(stGmBlob.m_szData), &ulUseSize, 0);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("pack SS_PKG_GM_RSP failed!");
        return false;
    }
    stGmBlob.m_iLen = (int)ulUseSize;

    struct evhttp_request* poRequest = HttpReqMgr::Instance().GetRequestByUin(rstSsPkg.m_stHead.m_ullUin);
    evbuffer* poBufData = evbuffer_new();
    evbuffer_add(poBufData, stGmBlob.m_szData, stGmBlob.m_iLen);
    LOGRUN("insert");
    evhttp_send_reply(poRequest, HTTP_OK, "OK", poBufData);
    evbuffer_free(poBufData);

    return 0;
}
int GM_DATABASE_OP_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
	//IDIP_GM_RSP_BLOB stGmBlob;

	//SS_PKG_GM_DATABASE_OP_RSP& rstGmDbRsp = rstSsPkg.m_stBody.m_stGmDatabaseOpRsp;

	//size_t ulUseSize = 0;
	//TdrError::ErrorType iRet = rstGmRsp.pack((char*)stGmBlob.m_szData, sizeof(stGmBlob.m_szData), &ulUseSize, 0);
	//if (iRet != TdrError::TDR_NO_ERROR)
	//{
	//	LOGERR("pack SS_PKG_GM_RSP failed!");
	//	return false;
	//}
	//stGmBlob.m_iLen = (int)ulUseSize;

	/*struct evhttp_request* poRequest = HttpReqMgr::Instance().GetRequestByUin(rstSsPkg.m_stHead.m_ullUin);
	evbuffer* poBufData = evbuffer_new();
	evbuffer_add(poBufData, stGmBlob.m_szData, stGmBlob.m_iLen);
	LOGRUN("insert");
	evhttp_send_reply(poRequest, HTTP_OK, "OK", poBufData);
	evbuffer_free(poBufData);*/

	return 0;
}
