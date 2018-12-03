#include "AsyncPvpSvr.h"
#include "BufferedLog.h"

int main( int argc, char** argv )
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

    if( !AsyncPvpSvr::Instance().SvrInit( argc, argv, "AsyncPvpSvr", dwLogTypes ) )
    {
        LOGERR_r("Init AsyncPvpSvr failed!");
        return -1;
    }

    LOGRUN_r("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);

    AsyncPvpSvr::Instance().SvrRun( );

    LOGRUN_r("==finish runing==");

    return 0;
}
