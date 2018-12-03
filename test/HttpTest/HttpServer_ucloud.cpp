/*
    基于libevent, 接收json格式的http post包，处理(转成tbus包发往游戏内部)，发送http应答
    单线程单进程
    测试uri，命令行参数为: "/callback/charge"
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <assert.h>

#include "oi_misc.h"

bool g_bIdle = true;


static void print_header( struct evhttp_request *req )
{
    struct evkeyvalq *headers;
	struct evkeyval *header;

    headers = evhttp_request_get_input_headers(req);
	for (header = headers->tqh_first; header;
	    header = header->next.tqe_next) {
		printf("  %s: %s\n", header->key, header->value);
	}
}

static void handle_request_cb( struct evhttp_request *req, void *arg )
{
    g_bIdle = false;

	struct evbuffer *evbuf;

	/* Yes, we are expecting a post request */
	if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
		fprintf(stdout, "FAILED (post type)\n");
		exit(1);
	}

    print_header( req );

    evbuf = evhttp_request_get_input_buffer(req);
    char buf[1024];
    int n = evbuffer_copyout(evbuf, buf, sizeof(buf)-1);
    if( n < 0 )
    {
        printf("shit! n=%d\n",n);
        assert( false );
    }
    buf[n]='\0';

    printf("post req buf:  %s\n", buf );

    struct evbuffer *evb = evbuffer_new();
    char* rspstr = "{code:0}";
	evbuffer_add(evb, rspstr, strlen(rspstr) );

    /*
        handle json pkg!
    */
    n = evbuffer_copyout( evb, buf, sizeof(buf)-1);
    buf[n]='\0';
    printf("response n = %d\n",n);
    printf("post reply buf: %s\n", buf);

    /*
        After calling evhttp_send_reply() databuf will be empty, but the buffer is still
        owned by the caller and needs to be deallocated by the caller if necessary.
    */
	evhttp_send_reply(req, HTTP_OK, "pay ok", evb); 

    evbuffer_free(evb);
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

    struct event_base *base;
	struct evhttp *http;
	struct evhttp_bound_socket *handle;
    unsigned short port = 8080; // http listen port

    base = event_base_new();
	if (!base) {
		fprintf(stderr, "Couldn't create an event_base: exiting\n");
		return 1;
	}

    /* Create a new evhttp object to handle requests. */
	http = evhttp_new(base);
	if (!http) {
		fprintf(stderr, "couldn't create evhttp. Exiting.\n");
		return 1;
	}

    evhttp_set_cb(http, uri, handle_request_cb, NULL);
    //int iRet = evhttp_set_cb( http, "/bill", handle_request_cb, NULL); // for test
    //iRet = evhttp_set_cb( http, "/billz", handle_request_cb, NULL); // for test
    //printf( "iRet = %d\n", iRet );

	/* Now we tell the evhttp what port to listen on */
	//handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", port);
	handle = evhttp_bind_socket_with_handle( http, "127.0.0.1", port );
	if (!handle) {
		fprintf(stderr, "couldn't bind to port %d. Exiting.\n",
		    (int)port);
		return 1;
	}

    while(1)
    {
       event_base_loop( base, EVLOOP_NONBLOCK );

        if( g_bIdle )
        {
            MsSleep(2000);
            printf("idle ...\n");
        }else
        {
            printf("busy ...\n");
        }

        g_bIdle = true;
    }

    evhttp_free( http );
    event_base_free( base );
}

