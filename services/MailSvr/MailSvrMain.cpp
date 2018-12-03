#include "MailSvr.h"
#include "BufferedLog.h"

int main( int argc, char** argv )
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

    if( !MailSvr::Instance().SvrInit( argc, argv, "MailSvr", dwLogTypes ) )
    {
        LOGERR("Init MailSvr failed!");
        return -1;
    }
    
    LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);

    MailSvr::Instance().SvrRun( );

    LOGRUN("==finish runing==");

    return 0;
}
