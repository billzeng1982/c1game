#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "HttpClient.h"

#include "LogMacros.h"
#include "strutil.h"

const std::string CHttpClient::DEFAULT_HEADER = "Content-Type: application/x-www-form-urlencoded";

CHttpClient::CHttpClient()
{
    bzero( m_szUrl, sizeof(m_szUrl) );
    bzero( m_szErrInfo, sizeof(m_szErrInfo) );
    m_header = NULL;
    m_szRspDataBuf = new char[HTTP_RSP_DATA_BUF_SIZE];
}


CHttpClient::~CHttpClient()
{
    curl_slist_free_all(m_header);
    delete[] m_szRspDataBuf;
}

void CHttpClient::ChgUrl(const char* pszUrl)
{
    assert(pszUrl && pszUrl[0]);
    StrCpy(m_szUrl, pszUrl, sizeof(m_szUrl));
}

void CHttpClient::HeaderAppend(const char* pszContent )
{
    if( pszContent && pszContent[0] != '\0' )
    {
        m_header = curl_slist_append(m_header, pszContent);
    }else
    {
        m_header = curl_slist_append( m_header, CHttpClient::DEFAULT_HEADER.c_str() );
    }
}

bool CHttpClient::Post(const char* pszRequest )
{
    if( NULL == pszRequest || pszRequest[0] == '\0' )
    {
        fprintf(stderr, "Nil Request!\n");
        return false;
    }

    CURL* curl = NULL;
    CURLcode res;

    /* get a curl handle */
    curl = curl_easy_init();
    if( !curl )
    {
        LOGERR_r( "Can not get a curl handle!" );
        return false;
    }

    /* First set the URL that is about to receive our POST. This URL can
       just as well be a https:// URL if that is what should receive the
       data. */
    curl_easy_setopt(curl, CURLOPT_URL, m_szUrl);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, m_header);
    curl_easy_setopt(curl, CURLOPT_POST,1);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, HTTP_REQ_TIME_OUT);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);// seconds
    /* Now specify the POST data */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pszRequest);

    // 设置返回读取回调
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CHttpClient::_PostReceived);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, m_szRspDataBuf);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    /* Check for errors */
    if(res != CURLE_OK)
    {
        sprintf(m_szErrInfo, "%s", curl_easy_strerror(res));
        return false;
    }
    else
    {
        //curl_easy_cleanup(curl);
        return true;
    }
}

char* CHttpClient::GetLastErrInfo()
{
    return m_szErrInfo;
}


size_t CHttpClient::_PostReceived(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    char* pszRecvBuf = (char*)userdata;

    size_t n = size*nmemb;

    if( 0 == n )
    {
        LOGERR_r( "size*nmemb=0, size=%lu, nmemb=%lu, ptr=<%s>", size, nmemb, ptr );
        n = size;
    }
    
    if( n >= (size_t)HTTP_RSP_DATA_BUF_SIZE )
    {
        assert( false );
        LOGERR_r( "receive too many bytes: <%lu>, ptr=<%s>", n, ptr );
        pszRecvBuf[0] = '\0';
        return 0;
    }

    memcpy( pszRecvBuf, ptr, n );
    pszRecvBuf[n] = '\0'; // http包特别大的时候, _PostReceived可能被调用多次, 如这里问题突出，就不能这样写

    return n;
}

size_t CHttpClient::_PostReceivedChild(char *ptr, size_t size, size_t nmenb, void* userdata)
{
    CHttpClient* poSelf = (CHttpClient*) userdata;
    return poSelf->PostReceivedChild(ptr, size, nmenb);
}

bool CHttpClient::PostChild(const char* pszRequest, int64_t lSize)
{
    if( NULL == pszRequest || pszRequest[0] == '\0' )
    {
        LOGERR_r("Nil Request!");
        return false;
    }

    CURL* curl = NULL;
    CURLcode res;

    /* get a curl handle */
    curl = curl_easy_init();
    if( !curl )
    {
        LOGERR_r( "Can not get a curl handle!" );
        return false;
    }

    /* First set the URL that is about to receive our POST. This URL can
       just as well be a https:// URL if that is what should receive the
       data. */
    curl_easy_setopt(curl, CURLOPT_URL, m_szUrl);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, m_header);
    curl_easy_setopt(curl, CURLOPT_POST,1);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1); //测试
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, lSize );  //数据长度, lSize=-1,表示libcurl会自己计算
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, HTTP_REQ_TIME_OUT );  // seconds
    /* Now specify the POST data */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pszRequest );

    // 设置返回读取回调
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CHttpClient::_PostReceivedChild);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
    {
        LOGERR_r("curl_easy_perform() failed: %s", curl_easy_strerror(res));
    }

    /* always cleanup */
    curl_easy_cleanup(curl);

    return true;
}

void CHttpClient::InitGloabal()
{
    curl_global_init(CURL_GLOBAL_ALL);
}

void CHttpClient::ReleaseGlobal()
{
    curl_global_cleanup();
}



