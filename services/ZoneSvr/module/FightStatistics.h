#pragma once

#include "define.h"
#include "singleton.h"
#include "tlog/tlog.h"
#include "cs_proto.h"
#include "dwlog_clt.h"

using namespace PKGMETA;
using namespace DWLOG;

class DwLog : public TSingleton<DwLog>
{
public:
	DwLog();
	virtual ~DwLog();
	void WriteDwLog(const CS_PKG_DW_LOG & rstCSPkgBody);
    void WriteDwLogPVE(const CltFightStatsPVE& stFightStatsPVE);
private:
	void WriteCltFightStats(const CS_PKG_DW_LOG & rstCSPkgBody);
    void WriteCltFightStatsPVE(const CS_PKG_DW_LOG& rstCSPkgBody);
private:
	LPTLOGCTX m_pstLogCtx;
	LPTLOGCATEGORYINST m_pstPVPCat;
    LPTLOGCATEGORYINST m_pstPVECat;
};

