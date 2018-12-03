#include "ZoneSvr.h"
#include "BufferedLog.h"

int main(int argc, char** argv) 
{
	uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;
	if (!ZoneSvr::Instance().SvrInit(argc, argv, "ZoneSvr", dwLogTypes)) {
		LOGERR("Init ZoneSvr failed!");
		return -1;
	}

	LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__, __TIME__);

	ZoneSvr::Instance().SvrRun();

	return 0;
}

