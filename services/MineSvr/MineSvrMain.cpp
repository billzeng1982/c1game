#include "BufferedLog.h"
#include "MineSvr.h"


int main( int argc, char** argv )
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

    if( !MineSvr::Instance().SvrInit( argc, argv, "MineSvr", dwLogTypes ) )
    {
        LOGERR("Init MineSvr failed!");
        return -1;
    }
    
    LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__, __TIME__);

    MineSvr::Instance().SvrRun( );

    LOGRUN("==finish runing==");

    return 0;
}
