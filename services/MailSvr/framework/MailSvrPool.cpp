#include "MailSvrPool.h"
#include "LogMacros.h"

bool MailSvrPool::Init()
{
	if (!m_oPriMailDataPool.Init(PRI_MAILDATA_POOL_INIT_NUM, PRI_MAILDATA_POOL_DELTA_NUM, PRI_MAILDATA_POOL_MAX_NUM))
	{
		LOGERR("Init m_oMailDataPool failed");
		return false;
	}

	if (!m_oSsPkgPool.Init(SSPKG_POOL_INIT_NUM, SSPKG_POOL_DELTA_NUM, SSPKG_POOL_MAX_NUM))
	{
		LOGERR("Init m_oSsPkgPool failed");
		return false;
	}

    if (!m_oAsyncActionPool.Init(ASYNC_ACTION_POOL_INIT_NUM, ASYNC_ACTION_POOL_DELTA_NUM))
	{
		LOGERR("Init m_oSsPkgPool failed");
		return false;
	}

	return true;
}
