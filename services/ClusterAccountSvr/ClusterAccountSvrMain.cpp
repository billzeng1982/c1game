#include "ClusterAccountSvr.h"
#include "BufferedLog.h"

int main( int argc, char** argv )
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

    if( !ClusterAccountSvr::Instance().SvrInit( argc, argv, "ClusterAccountSvr", dwLogTypes ) )
    {
        LOGERR("Init ClusterAccountSvr failed!");
        return -1;
    }
    
    LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);

    ClusterAccountSvr::Instance().SvrRun( );

    LOGRUN("==finish runing==");

    return 0;
}


