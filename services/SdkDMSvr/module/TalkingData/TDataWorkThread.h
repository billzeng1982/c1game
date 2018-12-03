#pragma once
#include "define.h"
#include "singleton.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "LogMacros.h"
#include "ThreadFrame.h"
#include "../../cfg/SdkDMSvrCfgDesc.h"
#include "TDataClient.h"

#define JOSN_STR_ARGU                           \
    "[{"                                        \
    "\"msgID\":\"%s\","                         \
    "\"status\": \"%s\","                       \
    "\"OS\": \"%s\","                           \
    "\"accountID\": \"%s\","                    \
    "\"orderID\": \"%s\","                      \
    "\"currencyAmount\": %f,"                   \
    "\"currencyType\": \"%s\","                 \
    "\"virtualCurrencyAmount\": %f,"            \
    "\"chargeTime\": %lu,"                      \
    "\"iapID\": \"%s\","                        \
    "\"gameServer\": \"%s\","                   \
    "\"level\": %d,"                            \
    "\"partner\": \"%s\""                       \
    "}]"

using namespace PKGMETA;


class TDataWorkThread : public CThreadFrame
{
public:
    TDataWorkThread();
    ~TDataWorkThread(){}
public:
    virtual bool _ThreadAppInit(void* pvAppData);
    virtual void _ThreadAppFini();
    virtual int _ThreadAppProc();
    int _HandleTDataMgrMsg();
    int SendPost();
    int TestSendPost();
    void SendReq(DT_TDATA_ODER_INFO& rstReq);        //向工作线程发送请求
protected:
    SDKDMSVRCFG*	m_pstConfig;
    MyTdrBuf		m_stSendBuf;
    DT_TDATA_ODER_INFO           m_stReq;
    TDataClient     m_stClient;

    char m_szRawStr[MAX_HTTP_REQUEST_NUM];
    char m_szGzData[MAX_HTTP_REQUEST_NUM];
    size_t m_ulGzDataSize;
};

