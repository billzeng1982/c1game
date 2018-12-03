#include "NetConn.h"
#include "BufferedLog.h"

int main( int argc, char** argv )
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;
    CNetConn oNetConn;
    if( !oNetConn.SvrInit( argc, argv, "", dwLogTypes ) )
    {
        LOGERR("Init NetConn failed!");
        return -1;
    }
    
    LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);
    oNetConn.SvrRun( );

    LOGRUN("==finish runing==");

    return 0;
}


