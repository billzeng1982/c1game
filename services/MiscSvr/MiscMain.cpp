#include "MiscSvr.h"
#include "BufferedLog.h"

int main( int argc, char** argv )
{
	uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

	if( !MiscSvr::Instance().SvrInit( argc, argv, "MiscSvr", dwLogTypes ) )
	{
		LOGERR("Init MiscSvr failed!");
		return -1;
	}

	LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);

	MiscSvr::Instance().SvrRun( );

	LOGRUN("==finish runing==");

	return 0;
}

