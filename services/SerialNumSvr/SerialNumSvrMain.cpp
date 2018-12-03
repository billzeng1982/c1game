#include "SerialNumSvr.h"
#include "BufferedLog.h"

int main(int argc, char** argv)
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;
    if (!SerialNumSvr::Instance().SvrInit(argc, argv, "ServialNumSvr", dwLogTypes))
    {
        LOGERR("Init ServialNumSvr failed!");
        return -1;
    }

    LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__, __TIME__);

    SerialNumSvr::Instance().SvrRun();

    return 0;
}

