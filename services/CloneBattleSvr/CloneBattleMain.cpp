#include "BufferedLog.h"
#include "CloneBattleSvr.h"


int main( int argc, char** argv )
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

    if( !CloneBattleSvr::Instance().SvrInit( argc, argv, "CloneBattleSvr", dwLogTypes ) )
    {
        LOGERR("Init MailSvr failed!");
        return -1;
    }
    
    LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__, __TIME__);

    CloneBattleSvr::Instance().SvrRun( );

    LOGRUN("==finish runing==");

    return 0;
}
