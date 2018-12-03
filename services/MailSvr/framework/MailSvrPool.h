#pragma once
#include "ss_proto.h"
#include "DynMempool.h"
#include "MailTransaction.h"

using namespace PKGMETA;

class MailSvrPool : public TSingleton<MailSvrPool>
{
public:
	const static int PRI_MAILDATA_POOL_INIT_NUM  = 10000;
	const static int PRI_MAILDATA_POOL_DELTA_NUM = 1000;
    const static int PRI_MAILDATA_POOL_MAX_NUM = 300000;

    const static int SSPKG_POOL_INIT_NUM = 10;
    const static int SSPKG_POOL_DELTA_NUM = 1;
    const static int SSPKG_POOL_MAX_NUM =100;

    const static int ASYNC_ACTION_POOL_INIT_NUM = 100;
    const static int ASYNC_ACTION_POOL_DELTA_NUM = 10;

public:
	MailSvrPool() {}
	~MailSvrPool() {}

	bool Init();

	DynMempool<DT_MAIL_DATA>& PriMailDataPool() { return m_oPriMailDataPool; }
    DynMempool<SSPKG>& SsPkgPool() { return m_oSsPkgPool; }
    DynMempool<AsyncGetMailBoxAction>& AsyncActionPool() { return m_oAsyncActionPool; }

private:
	DynMempool<DT_MAIL_DATA> m_oPriMailDataPool;
    DynMempool<SSPKG> m_oSsPkgPool;
    DynMempool<AsyncGetMailBoxAction> m_oAsyncActionPool;
};