#include "CommBusLayer.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "tbus/tbus.h"
#include <string.h>
#include "MyTdrBuf.h"
#include "oi_misc.h"

/*
    usage: ./send GCIMShmKey bindAddr
*/


int main( int argc, char** argv )
{
    assert( argc == 3 );

    CCommBusLayer oBusLayer;

    int iGCIMKey = atoi( argv[1] );

    char* sBindAddr = argv[2]; 
    int iBindAddr = 0;

    inet_aton( sBindAddr, (struct in_addr*)&iBindAddr); 

    if( oBusLayer.Init( iGCIMKey, iBindAddr ) != 0 )
    {
        printf("init failed!\n");
        return -2;
    }

    int iCount = 0;
    while(1)
    {
        int iRet = oBusLayer.Recv();
        if( iRet > 0 )
        {
            MyTdrBuf* pBuf = oBusLayer.GetRecvTdrBuf();

            printf( "Recv ---- %s\n", pBuf->m_szTdrBuf );
        }

        MsSleep(1);
        iCount++;
        if( iCount == 1000 )
        {
            oBusLayer.RefreshHandle();
            iCount = 0;
        }
    }

    return 0;
}

