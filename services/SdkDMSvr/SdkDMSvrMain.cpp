#include "SdkDMSvr.h"
#include "BufferedLog.h"

int main( int argc, char** argv )
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

    if( !SdkDMSvr::Instance().SvrInit( argc, argv, "SdkDMSvr", dwLogTypes ) )
    {
        LOGERR_r("Init SdkDMSvr failed!");
        return -1;
    }
    
    LOGRUN_r("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);

    SdkDMSvr::Instance().SvrRun( );

    LOGRUN_r("==finish runing==");

    return 0;
}
