#include "RoleGmTableMsg.h"
#include "RoleTableMsg.h"
#include "RoleTable.h"
#include "LogMacros.h"
#include "strutil.h"
#include "../framework/RoleSvrMsgLayer.h"
#include "../thread/DBWorkThread.h"



int CSsRoleGmUpdateInfoReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;

    PKGMETA::SS_PKG_ROLE_GM_UPDATE_INFO_REQ& rstRoleUptReq =  rstSsPkg.m_stBody.m_stRoleGmUpdateInfoReq;
    if ( '\0' == rstRoleUptReq.m_szCmd[0])
    {
        LOGERR_r("param is err");
        return -1;
    }
    m_oGmMgr.HandleMsgCmd(poWorkThread, rstSsPkg);

    return 0;
}