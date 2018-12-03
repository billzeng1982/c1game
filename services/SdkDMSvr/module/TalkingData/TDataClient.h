#pragma once

#include "HttpClient.h"
#include "curl/curl.h"
#include "zlib.h"

#define MAX_HTTP_REQUEST_NUM    2048
#define HEAD_CONTENT_TYPE "Content-Type:application/msgpack"
class TDataClient: public CHttpClient
{
public:
    virtual size_t PostReceivedChild(char *ptr, size_t size, size_t nmemb);
  
};

