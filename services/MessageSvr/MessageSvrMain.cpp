#include "MessageSvr.h"
#include "BufferedLog.h"

int main(int argc, char** argv)
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;
    if (!MessageSvr::Instance().SvrInit(argc, argv, "MessageSvr", dwLogTypes))
    {
        LOGERR("Init MessageSvr failed!");
        return -1;
    }

    LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__, __TIME__);

    MessageSvr::Instance().SvrRun();

    return 0;
}

