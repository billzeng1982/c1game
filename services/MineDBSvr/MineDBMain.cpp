#include "MineDBSvr.h"
#include "BufferedLog.h"

int main( int argc, char** argv )
{
	uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

	if( !MineDBSvr::Instance().SvrInit( argc, argv, "MineDBSvr", dwLogTypes ) )
	{
		LOGERR("Init MineDBSvr failed!");
		return -1;
	}

	LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);

	MineDBSvr::Instance().SvrRun( );

	LOGRUN("==finish runing==");

	return 0;
}

