#include "HttpServer.h"
#include "oi_misc.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <vector>

using namespace std;

static vector<struct evhttp_request*> s_reqVec;

static void handle_request_cb( struct evhttp_request *req, void *arg )
{
    CHttpServer* poHttpServer = (CHttpServer*)arg;

	/* Yes, we are expecting a post request */
	if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
		fprintf(stdout, "FAILED (post type)\n");
		return;
	}

    struct evbuffer *evbuf;
    evbuf = evhttp_request_get_input_buffer(req);
    int n = evbuffer_copyout(evbuf, poHttpServer->m_szReqDataBuf, (CHttpServer::HTTP_REQ_DATA_BUF_SIZE-1) );
    if( n < 0 )
    {
        printf("shit! n=%d\n",n);
        assert( false );
        return;
    }
    poHttpServer->m_szReqDataBuf[n]='\0';

    printf("post req:  %s\n", poHttpServer->m_szReqDataBuf );

    /*
        TODO: handle json pkg: json->tdr, send to internal game svr
    */
    s_reqVec.push_back(req);
}


void SendReply( struct evhttp_request *req, CHttpServer* poHttpServer )
{
    // response
    char* rspstr = "{code:0}";
    evbuffer_add(poHttpServer->m_pEvRspData, rspstr, strlen(rspstr) );

    // ´òÓ¡ÏÂreply
    char buf[1024];
    int n = evbuffer_copyout( poHttpServer->m_pEvRspData, buf, sizeof(buf)-1);
    buf[n]='\0';
    printf("post reply: %s\n", buf);

    /*
        After calling evhttp_send_reply() databuf will be empty, but the buffer is still
        owned by the caller and needs to be deallocated by the caller if necessary.
    */
	evhttp_send_reply( req, HTTP_OK, "pay ok", poHttpServer->m_pEvRspData ); 

    printf("post relpy end!!");
}

int main( int argc, char**argv )
{
    char* uri = NULL;
    if( 2 == argc )
    {
        uri = argv[1];
    }else
    {
        uri = "/";
    }

    CHttpServer oHttpServer;
    bool bRet = oHttpServer.Init( "127.0.0.1", 8080 );
    if( !bRet )
    {
        printf( "init error!\n" );
        return -1;
    }

    oHttpServer.AddRequestCb( uri, handle_request_cb );

    while( 1 )
    {
        oHttpServer.RecvAndHandle();
        MsSleep( 1 );

        // other logic
        while( !s_reqVec.empty() )
        {
            struct evhttp_request *req = s_reqVec.back();
            s_reqVec.pop_back();

            MsSleep(2000);
            SendReply( req, &oHttpServer );
            //evhttp_request_free( req );
        }
    }

    return 0;
}

