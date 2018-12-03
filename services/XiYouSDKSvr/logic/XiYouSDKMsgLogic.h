#pragma once
#include "MsgBase.h"
#include "tdr/tdr.h"
#include "../thread/HttpClientThread.h"
#include "StrSearch.h"

#define STATUS_OK 200
#define CHAR_MAX_ 256

class SDKAccountLoginReq_SS: public IMsgBase_r
{
public:
    SDKAccountLoginReq_SS();
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

    void _HandleXiyouLogin(PKGMETA::SSPKG& rstSsPkg, HttpClientThread* pThread);
    void _HandleCommonLogin(PKGMETA::SSPKG& rstSsPkg, HttpClientThread* pThread);

private:
    char m_szXiyouUrl[PKGMETA::MAX_LEN_URL];
    char m_szCommonUrl[PKGMETA::MAX_LEN_URL];

    LPTDRMETA m_pstXiyouLoginRspMeta;
    LPTDRMETA m_pstCommonLoginReqMeta;
    LPTDRMETA m_pstCommonLoginRspMeta;
};


class SDKGetOrderIDReq_SS: public IMsgBase_r
{
public:
    SDKGetOrderIDReq_SS();
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
    void _HandleXiyouPay(PKGMETA::SSPKG& rstSsPkg, HttpClientThread* pThread);
    void _HandleCommonPay(PKGMETA::SSPKG& rstSsPkg, HttpClientThread* pThread);

private:
	//处理返回的字符串，预先将extension解析出来
	void _HandleRspStr(char* pszRspStr, int iStrLen, char* pszExtension);

private:
    char m_szXiyouUrl[PKGMETA::MAX_LEN_URL];
    char m_szCommonUrl[PKGMETA::MAX_LEN_URL];
    char m_szAppSecret[PKGMETA::MAX_LEN_APPSECRET];

	PatternStr m_oExtensionStr;
	PatternStr m_oMessageStr;

    LPTDRMETA m_pstGetOrderRspMeta;
    LPTDRMETA m_pstCommonGetOrderReqMeta;
    LPTDRMETA m_pstCommonGetOrderRspMeta;

};




