#include "ZoneHttpConn.h"
#include "log/BufferedLog.h"

int main(int argc, char** argv)
{
    uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

    if (!ZoneHttpConn::Instance().SvrInit(argc, argv, "ZoneHttpConn", dwLogTypes))
    {
        LOGERR("Init ZoneHttpConn failed!");
        return -1;
    }

    LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__, __TIME__);

    ZoneHttpConn::Instance().SvrRun();

    LOGRUN("==finish runing==");

    return 0;
}