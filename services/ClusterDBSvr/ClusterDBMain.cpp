#include "ClusterDBSvr.h"
#include "BufferedLog.h"

int main( int argc, char** argv )
{
	uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

	if( !ClusterDBSvr::Instance().SvrInit( argc, argv, "ClusterDBSvr", dwLogTypes ) )
	{
		LOGERR("Init ClusterDBSvr failed!");
		return -1;
	}

	LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);

	ClusterDBSvr::Instance().SvrRun( );

	LOGRUN("==finish runing==");

	return 0;
}

