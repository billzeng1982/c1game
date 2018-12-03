#include "UdpRelay.h"
#include "ss_proto.h"
#include "oi_misc.h"
#include "MyTdrBuf.h"
#include <stdlib.h>

using namespace PKGMETA;

// usage: ./UdpRelayCltTest ip port

int main( int argc, char** argv )
{
    assert( argc == 3 );
    char* pszIpAddr = argv[1];
    int iPort = atoi(argv[2]);

    PKGMETA::SSPKG ssPkg;

    UdpRelaySvr udpRelaySvr;
    
    if( !udpRelaySvr.Init( pszIpAddr, iPort, UdpRelaySvr::DEFAULT_THREAD_Q_SIZE) )
    {
        return -1;
    }

    while( true )
    {
        if( udpRelaySvr.Recv(CThreadFrame::MAIN_THREAD) > 0 )
        {
            MyTdrBuf* pTdrBuf = udpRelaySvr.GetRecvBuf(CThreadFrame::MAIN_THREAD);
            TdrError::ErrorType iRet = ssPkg.unpack( pTdrBuf->m_szTdrBuf, pTdrBuf->m_uPackLen );
            if ( iRet != TdrError::TDR_NO_ERROR )
            {
                printf("unpack pkg failed! errno : %d\n", iRet);
                return -1;
            } 

            printf( "Recv from client proc <%d>, name <%s>\n", \
                ssPkg.m_stHead.m_iSrcProcId, ssPkg.m_stBody.m_stAccountLoginReq.m_szAccountName );
        }
        else
        {
            MsSleep(1);
        }
    }

    udpRelaySvr.Close();

    return 0;
}

