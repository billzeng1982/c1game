#include "DirSvr.h"
#include "BufferedLog.h"

int main(int argc, char** argv)
{
 	uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;
	if (!DirSvr::Instance().SvrInit(argc, argv, "DirSvr", dwLogTypes))
    {
		LOGERR("Init DirSvr failed!");
		return -1;
	}

	LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__, __TIME__);

	DirSvr::Instance().SvrRun();

	return 0;
}

