#include <string.h>
#include "strutil.h"
#include "LogMacros.h"
#include "MsgLogicClusterGate.h"
#include "common_proto.h"
#include "../framework/ClusterGateMsgLayer.h"
#include "../module/ServOnlineMgr.h"

using namespace PKGMETA;

int ClusterServStatNtf_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_CLUSTER_SERV_STAT_NTF& rstSsPkgBody = rstSsPkg.m_stBody.m_stClusterServStatNtf;
    ServOnlineMgr::Instance().HandleServStatNtf(rstSsPkgBody.m_stServInfo);
    return 0;
}

