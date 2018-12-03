#include "GuildSvr.h"
#include "BufferedLog.h"

int main(int argc, char** argv)
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;
    if (!GuildSvr::Instance().SvrInit(argc, argv, "GuildSvr", dwLogTypes))
    {
        LOGERR("Init GuildSvr failed!");
        return -1;
    }

    LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__, __TIME__);

    GuildSvr::Instance().SvrRun();

    return 0;
}

