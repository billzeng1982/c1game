#include "ClusterGate.h"
#include "BufferedLog.h"

int main(int argc, char** argv)
{
 	uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;
	if (!ClusterGate::Instance().SvrInit(argc, argv, "ClusterGate", dwLogTypes))
    {
		LOGERR("Init ClusterGate failed!");
		return -1;
	}

	LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__, __TIME__);

	ClusterGate::Instance().SvrRun();

	return 0;
}

