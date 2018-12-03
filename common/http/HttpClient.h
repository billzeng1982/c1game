#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_

/*
    基于libcurl的http client
    同步方式
*/

#include <curl/curl.h>
#include <assert.h>
#include <string>

class CHttpClient
{
public:
    static const int HTTP_RSP_DATA_BUF_SIZE = 1024*4; // http 应答数据buf大小
    static const std::string DEFAULT_HEADER;
    static const int HTTP_REQ_TIME_OUT = 10; // http3?ê±, second

public:
    CHttpClient();
    virtual ~CHttpClient();

    void ChgUrl(const char* pszUrl);
    void HeaderAppend(const char* pszContent = NULL );
    bool Post(const char* pszRequest );
    //会真正回调虚函数PostReceivedChild,可以让子类实现自己的回调 lSize = -1 表示libcurl会自动去计算数据长度,但pszRequest必须是C字符串
    bool PostChild(const char* pszRequest , int64_t lSize = -1);
    char* GetLastErrInfo();
    virtual size_t PostReceivedChild(char* ptr, size_t size, size_t nmenb) { return 0; };
public:
    char* m_szRspDataBuf;
    size_t m_dwRspDataBufSize;

protected:
    // 获取post request 响应的数据
    static size_t _PostReceived(char *ptr, size_t size, size_t nmemb, void *userdata);
    static size_t _PostReceivedChild(char *ptr, size_t size, size_t nmenb, void* userdata);
public:
    //client -- http 全局初始化, 全局释放 方便多线程处理
    static void InitGloabal();
    static void ReleaseGlobal();
protected:
    char m_szUrl[1024];
    char m_szErrInfo[1024];
    struct curl_slist* m_header;
};

#endif

