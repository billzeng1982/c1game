
#include "GuildExpeditionSvr.h"
#include "BufferedLog.h"

int main( int argc, char** argv )
{
	uint32_t dwLogTypes = CBufferedLog::LOGRUN | CBufferedLog::LOGERR | CBufferedLog::LOGWARN;

	if( !GuildExpeditionSvr::Instance().SvrInit( argc, argv, "GuildExpeditionSvr", dwLogTypes ) )
	{
		LOGERR("Init GuildExpeditionSvr failed!");
		return -1;
	}

	LOGRUN("==begin to run==[LastCompile:%s %s]", __DATE__,__TIME__);

	GuildExpeditionSvr::Instance().SvrRun( );

	LOGRUN("==finish runing==");

	return 0;
}

