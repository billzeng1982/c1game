#include "FightSvr.h"
#include "BufferedLog.h"

int main( int argc, char** argv )
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;
    if( !CFightSvr::Instance().SvrInit( argc, argv, "", dwLogTypes ) )
    {
        LOGERR("Init FightSvr failed!");
        return -1;
    }
    
    LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);

    CFightSvr::Instance().SvrRun( );

    return 0;
}

