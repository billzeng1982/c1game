#include "strutil.h"
#include "LogMacros.h"
#include "MsgLogicDir.h"
#include "../framework/DirSvrMsgLayer.h"
#include "../module/ZoneSvrMgr.h"

using namespace PKGMETA;

int ZoneSvrStatNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_CLUSTER_SERV_STAT_NTF& rstSsPkgBody = rstSsPkg.m_stBody.m_stClusterServStatNtf;
    ZoneSvrMgr::Instance().HandleSvrStatNtf(rstSsPkgBody.m_stServInfo);
    return 0;
}
