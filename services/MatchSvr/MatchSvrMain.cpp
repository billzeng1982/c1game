#include "MatchSvr.h"
#include "BufferedLog.h"

int main(int argc, char** argv) 
{
 	uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;
	if (!MatchSvr::Instance().SvrInit(argc, argv, "MatchSvr", dwLogTypes)) 
    {
		LOGERR("Init MatchSvr failed!");
		return -1;
	}

	LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__, __TIME__);

	MatchSvr::Instance().SvrRun();

	return 0;
}

