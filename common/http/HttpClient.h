#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_

/*
    ����libcurl��http client
    ͬ����ʽ
*/

#include <curl/curl.h>
#include <assert.h>
#include <string>

class CHttpClient
{
public:
    static const int HTTP_RSP_DATA_BUF_SIZE = 1024*4; // http Ӧ������buf��С
    static const std::string DEFAULT_HEADER;
    static const int HTTP_REQ_TIME_OUT = 10; // http3?����, second

public:
    CHttpClient();
    virtual ~CHttpClient();

    void ChgUrl(const char* pszUrl);
    void HeaderAppend(const char* pszContent = NULL );
    bool Post(const char* pszRequest );
    //�������ص��麯��PostReceivedChild,����������ʵ���Լ��Ļص� lSize = -1 ��ʾlibcurl���Զ�ȥ�������ݳ���,��pszRequest������C�ַ���
    bool PostChild(const char* pszRequest , int64_t lSize = -1);
    char* GetLastErrInfo();
    virtual size_t PostReceivedChild(char* ptr, size_t size, size_t nmenb) { return 0; };
public:
    char* m_szRspDataBuf;
    size_t m_dwRspDataBufSize;

protected:
    // ��ȡpost request ��Ӧ������
    static size_t _PostReceived(char *ptr, size_t size, size_t nmemb, void *userdata);
    static size_t _PostReceivedChild(char *ptr, size_t size, size_t nmenb, void* userdata);
public:
    //client -- http ȫ�ֳ�ʼ��, ȫ���ͷ� ������̴߳���
    static void InitGloabal();
    static void ReleaseGlobal();
protected:
    char m_szUrl[1024];
    char m_szErrInfo[1024];
    struct curl_slist* m_header;
};

#endif

