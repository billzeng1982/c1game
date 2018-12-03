#include "XiYouSDKSvr.h"
#include "BufferedLog.h"

int main( int argc, char** argv )
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

    if( !XiYouSDKSvr::Instance().SvrInit( argc, argv, "XiYouSDKSvr", dwLogTypes ) )
    {
        LOGERR("Init XiYouSDKSvr failed!");
        return -1;
    }

    LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);

    XiYouSDKSvr::Instance().SvrRun( );

    LOGRUN("==finish runing==");

    return 0;
}
