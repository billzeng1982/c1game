#include "IdipAgentSvr.h"
#include "BufferedLog.h"

int main(int argc, char** argv)
{
	uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

	if( !IdipAgentSvr::Instance().SvrInit( argc, argv, "IdipAgentSvr", dwLogTypes ) )
	{
		LOGERR("Init IdipAgentSvr failed!");
		return -1;
	}

	LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);

	IdipAgentSvr::Instance().SvrRun();

	LOGRUN("==finish runing==");

	return 0;
}
