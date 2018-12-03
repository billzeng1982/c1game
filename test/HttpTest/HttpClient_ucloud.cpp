#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

/*
    利用libcurl做http客户端test
    测试post方法
    同步方式
*/

static size_t PostReceived(char *ptr, size_t size, size_t nmemb, void *userdata)

{
    char* pszRecvBuf = (char*)userdata; 
    
    size_t n = size*nmemb;
    memcpy( pszRecvBuf, ptr, n  );
    pszRecvBuf[n] = '\0';
    //printf("size=%ld, nmemb=%ld \n", size, nmemb);
    
    return n;
}


int main(int argc, char** argv)
{
    int count = 0;
    if( 1 == argc ) count = 1;
    else count = atoi(argv[1]);

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    struct curl_slist* header=NULL;
    header = curl_slist_append( header, "Content-Type: application/json" );

    for( int i = 0; i < count; i++ )
    {
        printf("send count: %d\n", i+1);
        
        CURL* curl;
        CURLcode res;

        /* get a curl handle */
        curl = curl_easy_init();
        if(curl) {
            /* First set the URL that is about to receive our POST. This URL can
            just as well be a https:// URL if that is what should receive the
            data. */
            curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8081/ucloud/pending_charge");
            curl_easy_setopt(curl,CURLOPT_HTTPHEADER,header);
            curl_easy_setopt(curl,CURLOPT_POST,1);
            curl_easy_setopt(curl,CURLOPT_TIMEOUT, 5 );  // seconds
            /* Now specify the POST data */
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"channel\":\"test\", \"uid\":\"test_0\", \"appUid\": \"test_0\", \"serverId\": \"0\", \"currencyCount\": 0, \"realPayMoney\": 0, \"ratio\": 10}");

            // 设置返回读取回调
            char szRecvBuf[1024];
            curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, PostReceived);
            curl_easy_setopt(curl,CURLOPT_WRITEDATA,&szRecvBuf);
            
            /* Perform the request, res will get the return code */
            res = curl_easy_perform(curl);
            /* Check for errors */
            if(res != CURLE_OK)
            {
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                  curl_easy_strerror(res));
            }else
            {
                printf("Recv: %s\n", szRecvBuf );
            }

            /* always cleanup */
            curl_easy_cleanup(curl);
        }

        sleep(2);
    }

    curl_slist_free_all(header);
    curl_global_cleanup();
    return 0;
}

