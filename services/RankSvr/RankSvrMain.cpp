#include "RankSvr.h"
#include "BufferedLog.h"

int main( int argc, char** argv )
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

    if( !RankSvr::Instance().SvrInit( argc, argv, "RankSvr", dwLogTypes ) )
    {
        LOGERR("Init RankSvr failed!");
        return -1;
    }
    
    LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);

    RankSvr::Instance().SvrRun( );

    LOGRUN("==finish runing==");

    return 0;
}
