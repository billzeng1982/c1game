#include "LogQuerySvr.h"
#include "BufferedLog.h"

int main(int argc, char** argv)
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

    if( !LogQuerySvr::Instance().SvrInit( argc, argv, "LogQuerySvr", dwLogTypes ) )
    {
        LOGERR("Init LogQuerySvr failed!");
        return -1;
    }

    LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);

    LogQuerySvr::Instance().SvrRun( );

    LOGRUN("==finish runing==");

    return 0;
}
