#include "HttpClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char** argv)
{
    int count = 0;
    if( 1 == argc ) count = 1;
    else count = atoi(argv[1]); 

    CHttpClient oHttpClient;
    oHttpClient.Init( "http://localhost:8081/ucloud/pending_charge" );

    for( int i = 0; i < count; i++ )
    {
        if( !oHttpClient.Post( "{\"channel\":\"test\", \"uid\":\"test_0\", \"appUid\": \"test_0\", \"serverId\": \"0\", \"currencyCount\": 0, \"realPayMoney\": 0, \"ratio\": 10}" ) )
        {
            fprintf( stderr, "Post failed!\n" );
            return -1;
        }

        printf("Recv: %s\n", oHttpClient.m_szRspDataBuf );
        
        sleep(2);
    }

    return 0;
}

