#include "TDataClient.h"
#include "LogMacros.h"

size_t TDataClient::PostReceivedChild(char *ptr, size_t size, size_t nmemb)
{
    size_t n = size*nmemb;
    if( n >= (size_t)HTTP_RSP_DATA_BUF_SIZE )
    {
        assert( false );
        LOGERR_r( "receive too many bytes: <%ld>", n );
        m_szRspDataBuf[0] = '\0';
        return 0;
    }

    memcpy( m_szRspDataBuf, ptr, n );
    m_dwRspDataBufSize = n;
    return n;
}


