#include "ThreadMsgLogic.h"
#include "LogMacros.h"
#include "strutil.h"
#include "ClusterAccTable.h"
#include "../ClusterAccountSvr.h"
#include "../framework/ClusterAccSvrMsgLayer.h"
#include "../DBThread/DBWorkThread.h"

using namespace PKGMETA;

int SsClusterAccNewRoleReg::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;

    SS_PKG_CLUSTER_ACC_NEW_ROLE_REG& rstNewRoleReg = rstSsPkg.m_stBody.m_stClusterAccNewRoleReg;
    int iErrNo = poWorkThread->GetClusterAccTable().CreateNewAccount( rstNewRoleReg );
    if( iErrNo != ERR_NONE )
    {
        LOGCORE_r("Create new account failed! < %lu|%s|%s|%d|%s >", 
            rstNewRoleReg.m_ullUin, rstNewRoleReg.m_szSdkUserName, rstNewRoleReg.m_szRoleName, rstNewRoleReg.m_iServerID, rstNewRoleReg.m_szChannelID );
    }

    return 0;
}

int SsClusterAccChgRoleName::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;

    SS_PKG_CLUSTER_ACC_CHG_ROLE_NAME& rstChgRoleName = rstSsPkg.m_stBody.m_stClusterAccChgRoleName;
    int iErrNo = poWorkThread->GetClusterAccTable().ChangeRoleName( rstChgRoleName.m_ullUin, rstChgRoleName.m_szSdkUserName, rstChgRoleName.m_szNewRoleName);
    if( iErrNo != ERR_NONE )
    {
        LOGCORE_r("change role name failed! < %lu|%s >", 
            rstChgRoleName.m_ullUin, rstChgRoleName.m_szNewRoleName );
    }

    return 0;
}

int SsGetOneClusterAccInfoReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;

    SS_PKG_GET_ONE_CLUSTER_ACC_INFO_REQ& rstGetAccInfoReq = rstSsPkg.m_stBody.m_stGetOneClusterAccInfoReq;
    SSPKG& rstRspPkg = poWorkThread->GetRspSsPkg();
    
    int iErrNo= poWorkThread->GetClusterAccTable().GetAccInfo( rstGetAccInfoReq.m_szSdkUserName, rstGetAccInfoReq.m_iServerID, rstRspPkg.m_stBody.m_stGetOneClusterAccInfoRsp );
    if( iErrNo != ERR_NONE )
    {
        LOGCORE_r("Get one account info failed! < %s|%d >", 
           rstGetAccInfoReq.m_szSdkUserName, rstGetAccInfoReq.m_iServerID );
    }
    rstRspPkg.m_stBody.m_stGetOneClusterAccInfoRsp.m_nErrNo = iErrNo;

    rstRspPkg.m_stHead.m_wMsgId = SS_MSG_GET_ONE_CLUSTER_ACC_INFO_RSP;
    rstRspPkg.m_stHead.m_iDstProcId = rstSsPkg.m_stHead.m_iSrcProcId;
    rstRspPkg.m_stHead.m_ullReservId = rstSsPkg.m_stHead.m_ullReservId;

    poWorkThread->SendPkg( rstRspPkg );
    return 0;
}
