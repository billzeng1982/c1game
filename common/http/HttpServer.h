#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

/*
	基于libevent
	单线程单进程
	同步模式

	After calling evhttp_send_reply() databuf will be empty, but the buffer is still
    owned by the caller and needs to be deallocated by the caller if necessary.
*/

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include "define.h"

class CHttpServer
{
public:
	typedef void (*HttpRequestCb)(struct evhttp_request *, void * /* CHttpServer* */);
	static const int HTTP_REQ_DATA_BUF_SIZE = 2048;

public:
	CHttpServer();
	~CHttpServer();

	bool Init( char* pszAddr, uint16_t wPort );
	// Init之后添加
	bool AddRequestCb( char* pszUri, HttpRequestCb fHttpReqCb, void*  arg=NULL);

	// 放至AppUpdate()里
	void RecvAndHandle();

public:
	struct evbuffer* m_pEvRspData; // 复用, 不要在每个回掉函数中去 new/delete
	char* m_szReqDataBuf;

private:
	struct event_base *m_pEvbase;
	struct evhttp *m_pHttp;
};


#endif

