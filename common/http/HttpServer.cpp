#include "HttpServer.h"
#include <string.h>
#include <assert.h>

CHttpServer::CHttpServer()
{
    m_pEvbase = NULL;
    m_pHttp = NULL;
    m_pEvRspData = NULL;
    m_szReqDataBuf = NULL;
}


CHttpServer::~CHttpServer()
{
    evhttp_free( m_pHttp );
    event_base_free( m_pEvbase );
    evbuffer_free( m_pEvRspData );
}


bool CHttpServer::Init( char* pszAddr, uint16_t wPort )
{
    if( !pszAddr || '\0' == pszAddr[0] )
    {
        assert( false );
        return false;
    }

    m_szReqDataBuf = new char[ HTTP_REQ_DATA_BUF_SIZE ];
    if( !m_szReqDataBuf )
    {
        fprintf(stderr, "new %d bytes buf failed!\n", HTTP_REQ_DATA_BUF_SIZE);
        return false;
    }

    m_pEvRspData = evbuffer_new();

    m_pEvbase = event_base_new();
	if (!m_pEvbase)
    {
		fprintf(stderr, "Couldn't create an event_base: exiting\n");
		return false;
	}

    /* Create a new evhttp object to handle requests. */
	m_pHttp = evhttp_new(m_pEvbase);
	if (!m_pHttp)
    {
		fprintf(stderr, "couldn't create evhttp. Exiting.\n");
		return false;
	}

    /* Now we tell the evhttp what port to listen on */
	struct evhttp_bound_socket* handle = evhttp_bind_socket_with_handle( m_pHttp, pszAddr, wPort );
	if (!handle)
    {
		fprintf(stderr, "couldn't bind to port %d. Exiting.\n", (int)wPort);
		return false;
	}

    return true;
}


// Init之后添加
bool CHttpServer::AddRequestCb(char* pszUri, HttpRequestCb fHttpReqCb, void* arg)
{
    if( NULL == fHttpReqCb )
    {
        assert( false );
        return false;
    }

    if (NULL == arg)
    {
        arg = this;
    }

    int iRet = evhttp_set_cb( m_pHttp, pszUri, fHttpReqCb, arg);
    if( iRet < 0 )
    {
        fprintf(stderr, "couldn't set http callback, errcode <%d>. Exiting.\n", iRet);
        return false;
    }

    return true;
}


// 放至AppUpdate()里
void CHttpServer::RecvAndHandle()
{
    event_base_loop( m_pEvbase, EVLOOP_NONBLOCK );
}


