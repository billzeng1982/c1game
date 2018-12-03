#include "stdio.h"
#include "strutil.h"
#include "TDataWorkThread.h"
#include "LogMacros.h"
#include "ThreadFrame.h"
#include "../../framework/SdkDMSvrMsgLayer.h"

#define TEST_JSON                           \
        "[{"                                \
        "\"msgID\":\"1001\","               \
        "\"gameVersion\": \"2.0\","         \
        "\"OS\": \"android\","              \
        "\"accountID\": \"xxx\","           \
        "\"level\": 15,"                    \
        "\"gameServer\": \"金戈铁马\","      \
        "\"orderID\": \"xxx\","             \
        "\"iapID\": \"充值包类型\","         \
        "\"currencyAmount\": 99,"           \
        "\"currencyType\": \"CNY\","        \
        "\"virtualCurrencyAmount\": 990,"   \
        "\"paymentType\": \"支付宝\","       \
        "\"status\": \"request\","          \
        "\"chargeTime\": 1374732001321,"    \
        "\"mission\": \"新手任务1\""         \
        "}]"

static int CompressGzip(Bytef* pchData, uLong dwData, Bytef* pchZData, uLong* pdwZData);     //gzip方式压缩
static int UnCompressGzip(Bytef* pchZData, uLong dwZData, Bytef* pchData, uLong* pdwData);   //gzip方式解压缩

TDataWorkThread::TDataWorkThread() : m_stSendBuf(sizeof(DT_FRIEND_DB_REQ)*2+1)
{
    m_pstConfig = NULL;
}

bool TDataWorkThread::_ThreadAppInit(void* pvAppData)
{
    m_pstConfig = (SDKDMSVRCFG*)pvAppData;
    m_stClient.ChgUrl(m_pstConfig->m_szTDataUrl);
    m_stClient.HeaderAppend(HEAD_CONTENT_TYPE);
    return true;
}

void TDataWorkThread::_ThreadAppFini()
{

}

int TDataWorkThread::_ThreadAppProc()
{

    int iRet = _HandleTDataMgrMsg();
    if (iRet <= 0)
    {
        return -1;
    }
    return 0;
}

//处理主线程发来的消息
int TDataWorkThread::_HandleTDataMgrMsg()
{
    int iRecvBytes = this->Recv( WORK_THREAD );
    if (iRecvBytes < 0)
    {
        return -1;
    }
    else if (0 == iRecvBytes)
    {
        return 0;
    }
    MyTdrBuf* pstRecvBuf = GetRecvBuf(WORK_THREAD);
    int iRet = (int) m_stReq.unpack(pstRecvBuf->m_szTdrBuf, pstRecvBuf->m_uPackLen);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("unpack pkg failed! errno : %d", iRet);
        return 1;
    }
    SendPost();
    return 1;
}

void TDataWorkThread::SendReq(DT_TDATA_ODER_INFO& rstReq)
{
    TdrError::ErrorType iRet = rstReq.pack(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uSize, &m_stSendBuf.m_uPackLen );
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack rstReq pkg error!" );
        return;
    }
    m_oReqQueue.WriteMsg(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uPackLen);
    return;
}

int TDataWorkThread::SendPost()
{
//  bzero(m_szRawStr, MAX_HTTP_REQUEST_NUM);
//  bzero(m_szGzData, MAX_HTTP_REQUEST_NUM);
    m_szRawStr[0] = '\0';
    m_stClient.m_dwRspDataBufSize = 0;
    snprintf(m_szRawStr, MAX_HTTP_REQUEST_NUM, JOSN_STR_ARGU,
        m_stReq.m_szMsgID,
        m_stReq.m_szStatus,
        m_stReq.m_szOS,
        m_stReq.m_szAccountID,
        m_stReq.m_szOrderID,
        m_stReq.m_fCurrencyAmount,
        m_stReq.m_szCurrencyType,
        m_stReq.m_fVirtualCurrencyAmount,
        m_stReq.m_ullChargeTime,
        m_stReq.m_szIapID,
        m_stReq.m_szGameServer,
        m_stReq.m_iLevel,
        m_stReq.m_szPartner);
    m_ulGzDataSize = MAX_HTTP_REQUEST_NUM;
    int iRet =  CompressGzip( (Bytef*)m_szRawStr, strlen(m_szRawStr), (Bytef*) m_szGzData , &m_ulGzDataSize);
    if( 0 != iRet )
    {
        LOGERR_r("Comprees failed. AccountId<%s>, OderId<%s>", m_stReq.m_szAccountID, m_stReq.m_szOrderID);
        return ERR_SYS;
    }
    m_stClient.PostChild(m_szGzData, m_ulGzDataSize);
    if ( 0 == m_stClient.m_dwRspDataBufSize )
    {
        LOGERR_r("Http response error!  Account<%s>, Order<%s>", m_stReq.m_szAccountID,  m_stReq.m_szOrderID);
        return 0;
    }
    m_szRawStr[0] = '\0';
    m_ulGzDataSize = MAX_HTTP_REQUEST_NUM;
    if (UnCompressGzip((Bytef*)m_stClient.m_szRspDataBuf, m_stClient.m_dwRspDataBufSize, (Bytef*)m_szRawStr, &m_ulGzDataSize) != 0)
    {
        LOGERR_r("Uncompress the http-response failed! Account<%s>, Order<%s>", m_stReq.m_szAccountID,  m_stReq.m_szOrderID);
        return 0;
    }
    LOGRUN_r("Send OK: Account<%s>, Order<%s> Rsp<%s>", m_stReq.m_szAccountID,  m_stReq.m_szOrderID, m_szRawStr);
    return 0;
}

int TDataWorkThread::TestSendPost()
{
    bzero(&m_stReq, sizeof(m_stReq));
    StrCpy(m_stReq.m_szMsgID, "8002", 10);
    StrCpy(m_stReq.m_szStatus, "success", 10);
    StrCpy(m_stReq.m_szOS, "android", 10);
    StrCpy(m_stReq.m_szAccountID, "play_001", 10);
    StrCpy(m_stReq.m_szOrderID, "order_002", 10);
    m_stReq.m_fCurrencyAmount = 888;
    StrCpy(m_stReq.m_szCurrencyType, "CNY", 10);
    m_stReq.m_fVirtualCurrencyAmount = 8880;
    m_stReq.m_ullChargeTime = CGameTime::Instance().GetCurrSec();
    StrCpy(m_stReq.m_szIapID, "Iap_001", 10);
    StrCpy(m_stReq.m_szPaymentType, "zhifubao", 10);
    StrCpy(m_stReq.m_szGameServer, "Sid_001", 10);
    StrCpy(m_stReq.m_szGameVersion, "1.0.0", 10);
    m_stReq.m_iLevel = 18;
    StrCpy(m_stReq.m_szMission, "Task_001", 10);
    StrCpy(m_stReq.m_szPartner, "Xiyou", 10);
    SendPost();

    char ResponseBuf[2048] = {0};
    uLong dwSize = 2048;
    int iRet = UnCompressGzip((Bytef*)m_stClient.m_szRspDataBuf, m_stClient.m_dwRspDataBufSize, (Bytef*)ResponseBuf, &dwSize);
    if (iRet != 0)
    {
        LOGERR_r("error");
    }
    ResponseBuf[2047] = '\0';
    LOGRUN_r("HTTP Response:%s", ResponseBuf);
    sleep(222);
    return 0;
}


/**
 *pchData待压缩数据, dwData待压缩数据大小
 *pchZData压缩后数据存放地址, pdwZData 传入时表示 pchZData 的长度,返回时 表示压缩后数据大小
 **/
int CompressGzip(IN Bytef* pchData, IN uLong dwData, OUT Bytef* pchZData,  INOUT uLong* pdwZData)
{
    z_stream c_stream;
    int err = 0;
    if(pchData && dwData > 0)
    {
        c_stream.zalloc = Z_NULL;
        c_stream.zfree = Z_NULL;
        c_stream.opaque = Z_NULL;
        //MAX_WBITS+16 表示压缩后会带gzip的头和尾
        if(deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        {
            return -1;
        }
        c_stream.next_in = pchData;
        c_stream.avail_in = dwData;
        c_stream.next_out = pchZData;
        c_stream.avail_out = *pdwZData;
        while (c_stream.avail_in != 0 && c_stream.total_out < *pdwZData)
        {
            if(deflate(&c_stream, Z_NO_FLUSH) != Z_OK)
                return -1;
        }
        if(c_stream.avail_in != 0)
            return c_stream.avail_in;
        for (;;)
        {
            if((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END)
                break;
            if(err != Z_OK)
                return -1;
        }
        if(deflateEnd(&c_stream) != Z_OK)
            return -1;
        *pdwZData = c_stream.total_out;
        return 0;
    }
    return -1;
}
/**
 *pchData待解压缩数据, dwData待解压缩数据大小
 *pchZData压缩后数据存放地址, pdwZData 传入时表示 pchZData 的长度,返回时 表示解压缩后数据大小
 **/
int UnCompressGzip(IN Bytef* pchZData,IN uLong dwZData, OUT Bytef* pchData, INOUT uLong* pdwData)
{
    int err = 0;
    z_stream d_stream = {0}; /* decompression stream */
    static char dummy_head[2] =
    {
        0x8 + 0x7 * 0x10,
        (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
    };
    d_stream.zalloc = Z_NULL;
    d_stream.zfree = Z_NULL;
    d_stream.opaque = Z_NULL;
    d_stream.next_in = pchZData;
    d_stream.avail_in = 0;
    d_stream.next_out = pchData;
    //MAX_WBITS+16 表示解压带gzip的头和尾的gzip格式数据
    if(inflateInit2(&d_stream, MAX_WBITS + 16) != Z_OK)
        return -1;

    while (d_stream.total_out < *pdwData && d_stream.total_in < dwZData)
    {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
        if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END)
            break;
        if(err != Z_OK )
        {
            if(err == Z_DATA_ERROR)
            {
                d_stream.next_in = (Bytef*) dummy_head;
                d_stream.avail_in = sizeof(dummy_head);
                if((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK)
                {
                    return -1;
                }
            }
            else
                return -1;
        }
    }
    if(inflateEnd(&d_stream) != Z_OK)
        return -1;
    *pdwData = d_stream.total_out;
    return 0;
}

