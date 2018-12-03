#include "XiYouSDKMsgLogic.h"
#include "LogMacros.h"
#include "../framework/XiYouSDKSvrMsgLayer.h"
#include "../XiYouSDKSvr.h"
#include "md5Util.h"
#include "strutil.h"
#include "tsec/md5.h"

using namespace PKGMETA;

SDKAccountLoginReq_SS::SDKAccountLoginReq_SS()
{
    StrCpy(m_szXiyouUrl, XiYouSDKSvr::Instance().GetConfig().m_szXiyouAccountLoginUrl, MAX_LEN_URL);
    StrCpy(m_szCommonUrl, XiYouSDKSvr::Instance().GetConfig().m_szCommonAccountLoginUrl, MAX_LEN_URL);

    LPTDRMETALIB pstMetaLib;
    int iRet = tdr_load_metalib(&pstMetaLib, "../../protocol/common_proto.bin" );
    if (TDR_ERR_IS_ERROR(iRet))
	{
		LOGERR_r("load metalib common_proto.bin failed! (%s)", tdr_error_string(iRet));
		assert(false);
	}

    m_pstXiyouLoginRspMeta = tdr_get_meta_by_name(pstMetaLib, "DT_XY_SDK_LOGIN_RSP");
    assert(m_pstXiyouLoginRspMeta);

    m_pstCommonLoginReqMeta = tdr_get_meta_by_name(pstMetaLib, "DT_COMMON_SDK_LOGIN_REQ");
    assert(m_pstCommonLoginReqMeta);

    m_pstCommonLoginRspMeta = tdr_get_meta_by_name(pstMetaLib, "DT_COMMON_SDK_LOGIN_RSP");
    assert(m_pstCommonLoginRspMeta);
}

int SDKAccountLoginReq_SS::HandleServerMsg(SSPKG& rstSsPkg, void* pvPara)
{
    assert(pvPara);

    SS_PKG_SDK_ACCOUNT_LOGIN_REQ& rstLoginReq = rstSsPkg.m_stBody.m_stSDKAccountLoginReq;
    HttpClientThread* pThread = (HttpClientThread*)pvPara;

    LOGRUN_r("Handle Sdk AccountLogin Req, ChannelName is : %s", rstLoginReq.m_szChannelName);

    if (strcmp(rstLoginReq.m_szChannelName, "XY") == 0)
    {
        this->_HandleXiyouLogin(rstSsPkg, pThread);
    }
    else
    {
        this->_HandleCommonLogin(rstSsPkg, pThread);
    }

    return 0;
}

void SDKAccountLoginReq_SS::_HandleXiyouLogin(PKGMETA::SSPKG& rstSsPkg, HttpClientThread* pThread)
{
    SS_PKG_SDK_ACCOUNT_LOGIN_REQ& rstLoginReq = rstSsPkg.m_stBody.m_stSDKAccountLoginReq;

    SSPKG stTempSsPkg;
    stTempSsPkg.m_stHead.m_ullReservId = rstSsPkg.m_stHead.m_ullReservId;
    stTempSsPkg.m_stHead.m_wMsgId = SS_MSG_SDK_ACCOUNT_LOGIN_RSP;
    SS_PKG_SDK_ACCOUNT_LOGIN_RSP& rstLoginRsp = stTempSsPkg.m_stBody.m_stSDKAccountLoginRsp;

    CHttpClient* pHttpClient = &pThread->m_oHttpClient;
    pHttpClient->ChgUrl(m_szXiyouUrl);
    char szPostInfo[MAX_LEN_HTTPPOST_INFO];
    snprintf(szPostInfo, MAX_LEN_HTTPPOST_INFO, "token=%s", rstLoginReq.m_szToken);

    do
    {
        LOGRUN_r("Send post req, PostInfo(%s)", szPostInfo);
        //http post方式发送数据
        if (!pHttpClient->Post(szPostInfo))
        {
            rstLoginRsp.m_nErrNo = ERR_SYS;
            LOGERR_r("Account Login Post failed, errinfo=(%s), url=<%s>", pHttpClient->GetLastErrInfo(), m_szXiyouUrl);
            break;
        }
        LOGRUN_r("Session(%lu) Recv http Rsp, data=%s", rstSsPkg.m_stHead.m_ullReservId, pHttpClient->m_szRspDataBuf);

        //解析SDK返回数据
        TDRDATA stHost, stJson;
        DT_XY_SDK_LOGIN_RSP stSDKRsp;
        stJson.pszBuff = pHttpClient->m_szRspDataBuf;
        stJson.iBuff = CHttpClient::HTTP_RSP_DATA_BUF_SIZE;
        stHost.pszBuff = (char*)&stSDKRsp;
        stHost.iBuff = sizeof(stSDKRsp);

        int iRet = tdr_input_json(m_pstXiyouLoginRspMeta, &stHost, &stJson, 0);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            rstLoginRsp.m_nErrNo = ERR_SYS;
            LOGERR_r("json<%s> to bin failed, tdr error <%s>, url=<%s>, postInfo=<%s> ",
                pHttpClient->m_szRspDataBuf, tdr_error_string(iRet), m_szXiyouUrl, szPostInfo );
            break;
        }

        //SDK服务器返回的结果不是OK
        if (stSDKRsp.m_dwStatus != STATUS_OK)
        {
            rstLoginRsp.m_nErrNo = ERR_SYS;
            LOGERR_r("Xiyou SDK Login status(%u) is not OK, token(%s)", stSDKRsp.m_dwStatus, rstLoginReq.m_szToken);
            break;
        }

        //获取数据成功，返回
        rstLoginRsp.m_nErrNo = ERR_NONE;
        snprintf(rstLoginRsp.m_szUid, MAX_NAME_LENGTH, "%lu", stSDKRsp.m_stData.m_ullUserID);
    }while(false);
    pHttpClient->m_szRspDataBuf[0] = '\0';

    //由于XiYouSDKMsgLayer的Send函数不是线程安全的，因此不能直接在这里调用，将要发送的SsPkg缓存在
    //线程的队列中，由主线程统一调用Send函数
    pThread->SendPkg(stTempSsPkg);

    return;
}


void SDKAccountLoginReq_SS::_HandleCommonLogin(PKGMETA::SSPKG& rstSsPkg, HttpClientThread* pThread)
{
    SS_PKG_SDK_ACCOUNT_LOGIN_REQ& rstLoginReq = rstSsPkg.m_stBody.m_stSDKAccountLoginReq;

    SSPKG stTempSsPkg;
    stTempSsPkg.m_stHead.m_ullReservId = rstSsPkg.m_stHead.m_ullReservId;
    stTempSsPkg.m_stHead.m_wMsgId = SS_MSG_SDK_ACCOUNT_LOGIN_RSP;
    SS_PKG_SDK_ACCOUNT_LOGIN_RSP& rstLoginRsp = stTempSsPkg.m_stBody.m_stSDKAccountLoginRsp;

    CHttpClient* pHttpClient = &pThread->m_oHttpClient;
    pHttpClient->ChgUrl(m_szCommonUrl);
    char szPostInfo[MAX_LEN_HTTPPOST_INFO];

    DT_COMMON_SDK_LOGIN_REQ stCommonSDKLoginReq;
    StrCpy(stCommonSDKLoginReq.m_szToken, rstLoginReq.m_szToken, MAX_LEN_SDK_TOKEN_PARA);
    StrCpy(stCommonSDKLoginReq.m_szChannel, rstLoginReq.m_szChannelName, MAX_NAME_LENGTH);
    StrCpy(stCommonSDKLoginReq.m_szOpenid, rstLoginReq.m_szOpenID, MAX_NAME_LENGTH);
    stCommonSDKLoginReq.m_szExtension[0] = '\0';

    do
    {
        TDRDATA stHost, stJson;
        char szPostInfo[MAX_LEN_HTTPPOST_INFO];
        stJson.pszBuff = szPostInfo;
        stJson.iBuff = MAX_LEN_HTTPPOST_INFO;
        stHost.pszBuff = (char*)&stCommonSDKLoginReq;
        stHost.iBuff = sizeof(stCommonSDKLoginReq);
        int iRet = tdr_output_json(m_pstCommonLoginReqMeta, &stJson, &stHost, 0);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            rstLoginRsp.m_nErrNo = ERR_SYS;
            LOGERR_r("Login Req bin to json failed. (%s)", tdr_error_string(iRet));
            break;
        }

        LOGRUN_r("Send post req, PostInfo(%s)", szPostInfo);
        //http post方式发送数据
        if (!pHttpClient->Post(szPostInfo))
        {
            rstLoginRsp.m_nErrNo = ERR_SYS;
            LOGERR_r("Account Login Post failed, errinfo=(%s)", pHttpClient->GetLastErrInfo());
            break;
        }
        LOGRUN_r("Session(%lu) Recv http Rsp, data=%s", rstSsPkg.m_stHead.m_ullReservId, pHttpClient->m_szRspDataBuf);

        //解析SDK返回数据
        DT_COMMON_SDK_LOGIN_RSP stCommonSDKRsp;
        stJson.pszBuff = pHttpClient->m_szRspDataBuf;
        stJson.iBuff = CHttpClient::HTTP_RSP_DATA_BUF_SIZE;
        stHost.pszBuff = (char*)&stCommonSDKRsp;
        stHost.iBuff = sizeof(stCommonSDKRsp);

        iRet = tdr_input_json(m_pstCommonLoginRspMeta, &stHost, &stJson, 0);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            rstLoginRsp.m_nErrNo = ERR_SYS;
            LOGERR_r("json(%s) to bin failed. (%s)", pHttpClient->m_szRspDataBuf, tdr_error_string(iRet));
            break;
        }

        //SDK服务器返回的结果不是OK
        if (stCommonSDKRsp.m_iRet != 0)
        {
            rstLoginRsp.m_nErrNo = ERR_SYS;
            LOGERR_r("SDK Login status(%u) is not OK, token(%s)", stCommonSDKRsp.m_iRet, rstLoginReq.m_szToken);
            break;
        }

        //获取数据成功，返回
        rstLoginRsp.m_nErrNo = ERR_NONE;
        StrCpy(rstLoginRsp.m_szUid, stCommonSDKRsp.m_szUid, MAX_NAME_LENGTH);
    }while(false);
    pHttpClient->m_szRspDataBuf[0] = '\0';

    //由于XiYouSDKMsgLayer的Send函数不是线程安全的，因此不能直接在这里调用，将要发送的SsPkg缓存在
    //线程的队列中，由主线程统一调用Send函数
    pThread->SendPkg(stTempSsPkg);

    return;
}


SDKGetOrderIDReq_SS::SDKGetOrderIDReq_SS() : m_oExtensionStr("\"extension\":", 12), m_oMessageStr("},\"message\":", 12)
{
    StrCpy(m_szXiyouUrl, XiYouSDKSvr::Instance().GetConfig().m_szXiyouGetOrderIdUrl, MAX_LEN_URL);
    StrCpy(m_szCommonUrl, XiYouSDKSvr::Instance().GetConfig().m_szCommonGetOrderIdUrl, MAX_LEN_URL);
    StrCpy(m_szAppSecret, XiYouSDKSvr::Instance().GetConfig().m_szAppSecret, MAX_LEN_APPSECRET);

    LPTDRMETALIB pstMetaLib;
    int iRet = tdr_load_metalib(&pstMetaLib, "../../protocol/common_proto.bin" );
    if (TDR_ERR_IS_ERROR(iRet))
	{
		LOGERR_r("load metalib common_proto.bin failed! (%s)", tdr_error_string(iRet));
		assert(false);
	}
    m_pstGetOrderRspMeta = tdr_get_meta_by_name(pstMetaLib, "DT_XY_SDK_GET_ORDER_RSP");
    assert(m_pstGetOrderRspMeta);

    m_pstCommonGetOrderReqMeta = tdr_get_meta_by_name(pstMetaLib, "DT_COMMON_SDK_GET_ORDER_REQ");
    assert(m_pstCommonGetOrderReqMeta);

    m_pstCommonGetOrderRspMeta = tdr_get_meta_by_name(pstMetaLib, "DT_COMMON_SDK_GET_ORDER_RSP");
    assert(m_pstCommonGetOrderRspMeta);
}


int SDKGetOrderIDReq_SS::HandleServerMsg(SSPKG& rstSsPkg, void* pvPara)
{
    assert(pvPara);

    SS_PKG_SDK_GET_ORDERID_REQ& rstGetOrderIdReq = rstSsPkg.m_stBody.m_stSDKGetOrderIDReq;
    HttpClientThread* pThread = (HttpClientThread*)pvPara;

    LOGRUN_r("Handle Sdk GetOrder Req, ChannelName is : %s", rstGetOrderIdReq.m_szChannelName);

    if (strcmp(rstGetOrderIdReq.m_szChannelName, "XY") == 0)
    {
        this->_HandleXiyouPay(rstSsPkg, pThread);
    }
    else
    {
        this->_HandleCommonPay(rstSsPkg, pThread);
    }

    return 0;
}


void SDKGetOrderIDReq_SS::_HandleXiyouPay(PKGMETA::SSPKG& rstSsPkg, HttpClientThread* pThread)
{
    SS_PKG_SDK_GET_ORDERID_REQ& rstGetOrderIdReq = rstSsPkg.m_stBody.m_stSDKGetOrderIDReq;

    SSPKG stTempSsPkg;
    stTempSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    stTempSsPkg.m_stHead.m_wMsgId = SS_MSG_SDK_GET_ORDERID_RSP;
    SS_PKG_SDK_GET_ORDERID_RSP& rstGetOrderIdRsp = stTempSsPkg.m_stBody.m_stSDKGetOrderIDRsp;
    rstGetOrderIdRsp.m_dwProductID = rstGetOrderIdReq.m_dwProductID;

    CHttpClient* pHttpClient = &pThread->m_oHttpClient;
    pHttpClient->ChgUrl(m_szXiyouUrl);

    char szPostInfo[MAX_LEN_HTTPPOST_INFO];
    snprintf(szPostInfo, MAX_LEN_HTTPPOST_INFO, "buyNum=1&extension=%s&money=%u&pid=%u&productDesc=%s&productID=%u&productName=%s&roleID=%lu&roleLevel=%d&roleName=%s&serverID=%u&serverName=%s&uid=%s",
          rstGetOrderIdReq.m_szExtension, rstGetOrderIdReq.m_dwMoney, rstGetOrderIdReq.m_dwPid, rstGetOrderIdReq.m_szProductDesc,
          rstGetOrderIdReq.m_dwProductID, rstGetOrderIdReq.m_szProductName, rstGetOrderIdReq.m_ullRoleID, rstGetOrderIdReq.m_wRoleLevel,
          rstGetOrderIdReq.m_szRoleName, rstGetOrderIdReq.m_wServerID, rstGetOrderIdReq.m_szServerName, rstGetOrderIdReq.m_szUid);

    //计算签名
    char szOrginStr[MAX_LEN_HTTPPOST_INFO];
    snprintf(szOrginStr, MAX_LEN_HTTPPOST_INFO, "%s%s", szPostInfo, m_szAppSecret);
    unsigned char szmd5Sign[MD5_DIGEST_SIZE];
    CMD5Util::CalcMD5(szOrginStr, strlen(szOrginStr), szmd5Sign);
    char szmd5HexString[MD5_DIGEST_SIZE*2 +1];
    Md5HexString((char*)szmd5Sign, szmd5HexString);
    szmd5HexString[MD5_DIGEST_SIZE*2] = '\0';

    //将签名加入PostInfo中
    StrCat(szPostInfo, MAX_LEN_HTTPPOST_INFO, "&sign=%s", szmd5HexString);
    do
    {
        LOGRUN_r("Send post req, PostInfo(%s)", szPostInfo);
        if (!pHttpClient->Post(szPostInfo)) // 同步方式
        {
            rstGetOrderIdRsp.m_nErrNo = ERR_SYS;
            LOGERR_r("Get Order Post failed, errinfo=(%s)", pHttpClient->GetLastErrInfo());
            break;
        }
        LOGRUN_r("Recv http Rsp, data=%s", pHttpClient->m_szRspDataBuf);

        this->_HandleRspStr(pHttpClient->m_szRspDataBuf, strlen(pHttpClient->m_szRspDataBuf), rstGetOrderIdRsp.m_stData.m_szExtension);

        //解析SDK返回数据
        TDRDATA stHost, stJson;
        DT_XY_SDK_GET_ORDER_RSP stSDKRsp = {0};

        stJson.pszBuff = pHttpClient->m_szRspDataBuf;
        stJson.iBuff = CHttpClient::HTTP_RSP_DATA_BUF_SIZE;
        stHost.pszBuff = (char*)&stSDKRsp;
        stHost.iBuff = sizeof(stSDKRsp);

        int iRet = tdr_input_json(m_pstGetOrderRspMeta, &stHost, &stJson, 0);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            rstGetOrderIdRsp.m_nErrNo = ERR_SYS;
            LOGERR_r("json(%s) to bin failed. (%s)", pHttpClient->m_szRspDataBuf, tdr_error_string(iRet));
            break;
        }

        //SDK服务器返回的结果不是OK
        if (stSDKRsp.m_dwStatus != STATUS_OK)
        {
            rstGetOrderIdRsp.m_nErrNo = ERR_SYS;
            LOGERR_r("SDK Get Order status(%u) is not OK", stSDKRsp.m_dwStatus);
            break;
        }

        //获取数据成功，返回
        StrCpy(rstGetOrderIdRsp.m_stData.m_szOrderNo, stSDKRsp.m_stData.m_szOrderNo, MAX_LEN_SDK_PARA);
        rstGetOrderIdRsp.m_nErrNo = ERR_NONE;
    }while(false);

    //由于XiYouSDKMsgLayer的Send函数不是线程安全的，因此不能直接在这里调用，将要发送的SsPkg缓存在
    //线程的队列中，由主线程统一调用Send函数
    pThread->SendPkg(stTempSsPkg);

    return;
}


void SDKGetOrderIDReq_SS::_HandleRspStr(char* pszRspStr, int iStrLen, char* pszExtension)
{
    int iLeft = FindStr(pszRspStr, iStrLen, &m_oExtensionStr);
    if (iLeft == -1)
    {
        return;
    }
    iLeft += m_oExtensionStr.m_len;

    int iRight = FindStr(pszRspStr, iStrLen, &m_oMessageStr);
    if (iRight == -1)
    {
        return;
    }
    iRight -= 1;

    if (pszRspStr[iLeft]=='"' && pszRspStr[iRight]=='"')
    {
        strncpy(pszExtension, pszRspStr+iLeft+1, iRight-iLeft-1);
        pszExtension[iRight-iLeft-1] = '\0';
    }
    else
    {
        strncpy(pszExtension, pszRspStr+iLeft, iRight-iLeft+1);
        pszExtension[iRight-iLeft+1] = '\0';
    }

    pszRspStr[iLeft] = '"';
    pszRspStr[iRight] = '"';
    for (int i=iLeft+1; i<iRight; i++)
    {
        pszRspStr[i] = '.';
    }

    LOGRUN_r("Handle Str, Str=%s, Extension=%s", pszRspStr, pszExtension);
}

void SDKGetOrderIDReq_SS::_HandleCommonPay(PKGMETA::SSPKG& rstSsPkg, HttpClientThread* pThread)
{
    SS_PKG_SDK_GET_ORDERID_REQ& rstGetOrderIdReq = rstSsPkg.m_stBody.m_stSDKGetOrderIDReq;
    SSPKG stTempSsPkg;
    stTempSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    stTempSsPkg.m_stHead.m_wMsgId = SS_MSG_SDK_GET_ORDERID_RSP;
    SS_PKG_SDK_GET_ORDERID_RSP& rstGetOrderIdRsp = stTempSsPkg.m_stBody.m_stSDKGetOrderIDRsp;
    rstGetOrderIdRsp.m_dwProductID = rstGetOrderIdReq.m_dwProductID;

    CHttpClient* pHttpClient = &pThread->m_oHttpClient;
    pHttpClient->ChgUrl(m_szCommonUrl);

    DT_COMMON_SDK_GET_ORDER_REQ stCommonGetOrder;
    stCommonGetOrder.m_dwBuyNum = 1;
    stCommonGetOrder.m_dwMoney= rstGetOrderIdReq.m_dwMoney;
    stCommonGetOrder.m_dwProductId = rstGetOrderIdReq.m_dwProductID;
    stCommonGetOrder.m_dwServerId = rstGetOrderIdReq.m_wServerID;
    snprintf(stCommonGetOrder.m_szRoleId, MAX_NAME_LENGTH, "%lu", rstGetOrderIdReq.m_ullRoleID);
    StrCpy(stCommonGetOrder.m_szUid, rstGetOrderIdReq.m_szUid, MAX_NAME_LENGTH);
    StrCpy(stCommonGetOrder.m_szChannel, rstGetOrderIdReq.m_szChannelName, MAX_NAME_LENGTH);
    StrCpy(stCommonGetOrder.m_szToken, rstGetOrderIdReq.m_szToken, MAX_LEN_SDK_TOKEN_PARA);
    StrCpy(stCommonGetOrder.m_szExtension, rstGetOrderIdReq.m_szExtension, MAX_LEN_SDK_EXT_PARA);
    StrCpy(stCommonGetOrder.m_szProductDesc, rstGetOrderIdReq.m_szProductDesc, MAX_NAME_LENGTH);
    StrCpy(stCommonGetOrder.m_szProductName, rstGetOrderIdReq.m_szProductName, MAX_NAME_LENGTH);
    StrCpy(stCommonGetOrder.m_szRoleName, rstGetOrderIdReq.m_szRoleName, MAX_NAME_LENGTH);

    do
    {
        TDRDATA stHost, stJson;
        char szPostInfo[MAX_LEN_HTTPPOST_INFO];
        stJson.pszBuff = szPostInfo;
        stJson.iBuff = MAX_LEN_HTTPPOST_INFO;
        stHost.pszBuff = (char*)&stCommonGetOrder;
        stHost.iBuff = sizeof(stCommonGetOrder);
        int iRet = tdr_output_json(m_pstCommonGetOrderReqMeta, &stJson, &stHost, 0);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            rstGetOrderIdRsp.m_nErrNo = ERR_SYS;
            LOGERR_r("bin to json failed. (%s)", tdr_error_string(iRet));
            break;
        }

        LOGRUN_r("Send post req, PostInfo(%s)", szPostInfo);
        if (!pHttpClient->Post(szPostInfo))
        {
            rstGetOrderIdRsp.m_nErrNo = ERR_SYS;
            LOGERR_r("Get Order Post failed, errinfo=(%s)", pHttpClient->GetLastErrInfo());
            break;
        }
        LOGRUN_r("Recv http Rsp, data=%s", pHttpClient->m_szRspDataBuf);

        //解析SDK返回数据
        DT_COMMON_SDK_GET_ORDER_RSP stSDKRsp;
        stJson.pszBuff = pHttpClient->m_szRspDataBuf;
        stJson.iBuff = CHttpClient::HTTP_RSP_DATA_BUF_SIZE;
        stHost.pszBuff = (char*)&stSDKRsp;
        stHost.iBuff = sizeof(stSDKRsp);

        iRet = tdr_input_json(m_pstCommonGetOrderRspMeta, &stHost, &stJson, 0);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            rstGetOrderIdRsp.m_nErrNo = ERR_SYS;
            LOGERR_r("json(%s) to bin failed. (%s)", pHttpClient->m_szRspDataBuf, tdr_error_string(iRet));
            break;
        }

        //SDK服务器返回的结果不是OK
        if (stSDKRsp.m_iRet != 0)
        {
            rstGetOrderIdRsp.m_nErrNo = ERR_SYS;
            LOGERR_r("SDK  Get Order status(%d) is not OK", stSDKRsp.m_iRet);
            break;
        }

        //获取数据成功，返回
        StrCpy(rstGetOrderIdRsp.m_stData.m_szOrderNo, stSDKRsp.m_szOrderId, MAX_NAME_LENGTH);
        StrCpy(rstGetOrderIdRsp.m_stData.m_szExtension, stSDKRsp.m_szExtension, MAX_LEN_SDK_PARA);
        rstGetOrderIdRsp.m_nErrNo = ERR_NONE;
    }while(false);

    //由于XiYouSDKMsgLayer的Send函数不是线程安全的，因此不能直接在这里调用，将要发送的SsPkg缓存在
    //线程的队列中，由主线程统一调用Send函数
    pThread->SendPkg(stTempSsPkg);

    return;

}

