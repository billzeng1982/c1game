#include "ReplaySvr.h"
#include "BufferedLog.h"

int main(int argc, char** argv)
{
 	uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;
	if (!ReplaySvr::Instance().SvrInit(argc, argv, "ReplaySvr", dwLogTypes))
    {
		LOGERR("Init ReplaySvr failed!");
		return -1;
	}

	LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__, __TIME__);

	ReplaySvr::Instance().SvrRun();

	return 0;
}

