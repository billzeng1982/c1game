#include "CombineSvr.h"
#include "BufferedLog.h"

int main(int argc, char** argv)
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;
    if (!CombineSvr::Instance().SvrInit(argc, argv, "CombineSvr", dwLogTypes))
    {
        LOGERR("Init CombineSvr failed!");
        return -1;
    }

    LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__, __TIME__);

	CombineSvr::Instance().SvrRun();

    return 0;
}

