#include "MailDBSvr.h"
#include "BufferedLog.h"

int main( int argc, char** argv )
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

    if( !MailDBSvr::Instance().SvrInit( argc, argv, "MailDBSvr", dwLogTypes ) )
    {
        LOGERR("Init MailDBSvr failed!");
        return -1;
    }
    
    LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);

    MailDBSvr::Instance().SvrRun( );

    LOGRUN("==finish runing==");

    return 0;
}
