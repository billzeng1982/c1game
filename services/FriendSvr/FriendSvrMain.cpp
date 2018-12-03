#include "FriendSvr.h"
#include "BufferedLog.h"

int main( int argc, char** argv )
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

    if( !FriendSvr::Instance().SvrInit( argc, argv, "FriendSvr", dwLogTypes ) )
    {
        LOGERR_r("Init FriendSvr failed!");
        return -1;
    }
    
    LOGRUN_r("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);

    FriendSvr::Instance().SvrRun( );

    LOGRUN_r("==finish runing==");

    return 0;
}
