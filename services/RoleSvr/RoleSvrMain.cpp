#include "RoleSvr.h"
#include "BufferedLog.h"

int main( int argc, char** argv )
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

    if( !RoleSvr::Instance().SvrInit( argc, argv, "RoleSvr", dwLogTypes ) )
    {
        LOGERR("Init RoleSvr failed!");
        return -1;
    }
    
    LOGRUN("==begin to run==~~~~[LastCompile:%s %s]", __DATE__,__TIME__);

    RoleSvr::Instance().SvrRun( );

    LOGRUN("==finish runing==~~~~");

    return 0;
}
