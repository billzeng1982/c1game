#include "AccountSvr.h"
#include "BufferedLog.h"

int main( int argc, char** argv )
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

    if( !AccountSvr::Instance().SvrInit( argc, argv, "AccountSvr", dwLogTypes ) )
    {
        LOGERR("Init AccountSvr failed!");
        return -1;
    }
    
    LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);

    AccountSvr::Instance().SvrRun( );

    LOGRUN("==finish runing==");

    return 0;
}
