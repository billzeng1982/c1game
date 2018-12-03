#include "CommBusLayer.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "tbus/tbus.h"
#include <string.h>

/*
    usage: ./send GCIMShmKey srcAddr dstAddr
*/

int main( int argc, char** argv )
{
    assert( argc == 4 );

    CCommBusLayer oBusLayer;

    int iGCIMKey = atoi( argv[1] );
    char* sSrcAddr = argv[2]; 
    char* sDstAddr = argv[3];
    int iSrcAddr = 0;
    inet_aton( sSrcAddr, (struct in_addr*)&iSrcAddr); 

    int iDstAddr = 0;
    inet_aton( sDstAddr, (struct in_addr*)&iDstAddr); 

    if( oBusLayer.Init( iGCIMKey, iSrcAddr ) != 0 )
    {
        printf("init failed!\n");
        return -2;
    }

    char buf[64];
    snprintf( buf, sizeof(buf), "%s: %s", sSrcAddr, "Hello tbus!" );
    oBusLayer.Send( iDstAddr, buf, strlen(buf) );

    return 0;
}

