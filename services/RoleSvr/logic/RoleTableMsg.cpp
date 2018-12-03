#include "RoleTableMsg.h"
#include "RoleTable.h"
#include "LogMacros.h"
#include "strutil.h"
#include "../framework/RoleSvrMsgLayer.h"
#include "../thread/DBWorkThread.h"

using namespace PKGMETA;

int CSsRoleCreateReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{

    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;

    PKGMETA::SS_PKG_ROLE_CREATE_REQ& rstRoleCreateReq = rstSsPkg.m_stBody.m_stRoleCreateReq;
    if (0 == rstRoleCreateReq.m_stRoleWholeData.m_stBaseInfo.m_ullUin) // || '\0' == rstRoleCreateReq.m_stRoleWholeData.m_stBaseInfo.m_szRoleName[0])
    {
        LOGERR_r("param is err, Uin is 0");
        return -1;
    }

    SSPKG stTempSspkg;

    stTempSspkg.m_stHead.m_wMsgId = SS_MSG_ROLE_CREATE_RSP;
    stTempSspkg.m_stHead.m_iDstProcId = rstSsPkg.m_stHead.m_iSrcProcId;

    PKGMETA::SS_PKG_ROLE_CREATE_RSP& rstRoleCreateRsp = stTempSspkg.m_stBody.m_stRoleCreateRsp;
    bzero( &rstRoleCreateRsp, sizeof(rstRoleCreateRsp) );

    rstRoleCreateRsp.m_ullUin = rstRoleCreateReq.m_stRoleWholeData.m_stBaseInfo.m_ullUin;
    StrCpy( rstRoleCreateRsp.m_szRoleName, rstRoleCreateReq.m_stRoleWholeData.m_stBaseInfo.m_szRoleName, sizeof(rstRoleCreateRsp) );

    rstRoleCreateRsp.m_nErrNo = poWorkThread->GetRoleTable().CreateRole( rstRoleCreateReq.m_stRoleWholeData );

    poWorkThread->SendPkg(stTempSspkg);

    return 0;
}


int CSsRoleLoginReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;

    PKGMETA::COMMON_PKG_ROLE_LOGIN_REQ& rstRoleLoginReq = rstSsPkg.m_stBody.m_stRoleLoginReq;
    if (0 == rstRoleLoginReq.m_ullUin)
    {
        LOGERR_r("param is err, Uin is 0 ");
        return -1;
    }

    SSPKG stTempSspkg;

    stTempSspkg.m_stHead.m_wMsgId = SS_MSG_ROLE_LOGIN_RSP;
    stTempSspkg.m_stHead.m_iDstProcId = rstSsPkg.m_stHead.m_iSrcProcId;

    PKGMETA::COMMON_PKG_ROLE_LOGIN_RSP& rstRoleLoginRsp = stTempSspkg.m_stBody.m_stRoleLoginRsp;
    rstRoleLoginRsp.m_ullUin = rstRoleLoginReq.m_ullUin;
    rstRoleLoginRsp.m_chIsReconn = rstRoleLoginReq.m_chIsReconn;

    bzero(&rstRoleLoginRsp.m_stRspData.m_stRoleWholeData, sizeof(rstRoleLoginRsp.m_stRspData.m_stRoleWholeData));
    int iErrNo = poWorkThread->GetRoleTable().GetRoleWholeData(rstRoleLoginReq.m_ullUin, rstRoleLoginRsp.m_stRspData.m_stRoleWholeData);
    if (iErrNo != ERR_NONE)
    {
        rstRoleLoginRsp.m_chResult = 0;
        rstRoleLoginRsp.m_nErrNo = iErrNo;
    }
    else
    {
        rstRoleLoginRsp.m_chResult = 1;
        rstRoleLoginRsp.m_nErrNo = ERR_NONE;
    }

    poWorkThread->SendPkg(stTempSspkg);

    return 0;
}


int CSsRoleUptReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
	
    PKGMETA::SS_PKG_ROLE_UPDATE_REQ& rstRoleUptReq =  rstSsPkg.m_stBody.m_stRoleUpdateReq;
    if (0 == rstRoleUptReq.m_ullUin /*|| '\0' == rstRoleUptReq.m_szRoleName[0]*/)
    {
        LOGERR_r("param is err");
        return -1;
    }

    SSPKG stTempSspkg;

    stTempSspkg.m_stHead.m_wMsgId = SS_MSG_ROLE_UPDATE_RSP;
    stTempSspkg.m_stHead.m_iDstProcId = rstSsPkg.m_stHead.m_iSrcProcId;

    PKGMETA::SS_PKG_ROLE_UPDATE_RSP& rstRoleUptRsp = stTempSspkg.m_stBody.m_stRoleUpdateRsp;
    rstRoleUptRsp.m_ullUin = rstRoleUptReq.m_ullUin;
    rstRoleUptRsp.m_nErrNo = poWorkThread->GetRoleTable().UptRoleData(rstRoleUptReq);

    poWorkThread->SendPkg(stTempSspkg);

    return 0;
}


int CSsMajestyRegReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;

    SSPKG stTempSspkg;

    SS_PKG_MAJESTY_REGISTER_REQ& rstMajestyRegReq = rstSsPkg.m_stBody.m_stMajestyRegReq;
    SS_PKG_MAJESTY_REGISTER_RSP& rstMajestyRegRsp = stTempSspkg.m_stBody.m_stMajestyRegRsp;

    stTempSspkg.m_stHead.m_wMsgId = SS_MSG_MAJESTY_REGISTER_RSP;
    stTempSspkg.m_stHead.m_iDstProcId = rstSsPkg.m_stHead.m_iSrcProcId;

    rstMajestyRegRsp.m_ullUin = rstMajestyRegReq.m_ullUin;
    StrCpy(rstMajestyRegRsp.m_szName, rstMajestyRegReq.m_szName, MAX_NAME_LENGTH);

    rstMajestyRegRsp.m_nErrNo = poWorkThread->GetRoleTable().UptRoleName(rstMajestyRegReq.m_ullUin, rstMajestyRegReq.m_szName);

    poWorkThread->SendPkg(stTempSspkg);

    return 0;
}


int CSsRoleChgNameQueryReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
	CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;

	SSPKG stTempSspkg;

	SS_PKG_NAME_CHANGE_QUERY_REQ& rstSsPkgBodyReq = rstSsPkg.m_stBody.m_stNameChangeQueryReq;
	SS_PKG_NAME_CHANGE_QUERY_RSP& rstSsPkgBodyRsp = stTempSspkg.m_stBody.m_stNameChangeQueryRsp;

	stTempSspkg.m_stHead.m_wMsgId = SS_MSG_NAME_CHANGE_QUERY_RSP;
	stTempSspkg.m_stHead.m_iDstProcId = rstSsPkg.m_stHead.m_iSrcProcId;
	stTempSspkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

	rstSsPkgBodyRsp.m_nErrNo = poWorkThread->GetRoleTable().UptRoleName(rstSsPkg.m_stHead.m_ullUin, rstSsPkgBodyReq.m_szName);
	//rstSsPkgBodyRsp.m_nErrNo = poWorkThread->GetRoleTable().CheckExist(rstSsPkgBodyReq.m_szName);
	StrCpy(rstSsPkgBodyRsp.m_szName, rstSsPkgBodyReq.m_szName, MAX_NAME_LENGTH);

	poWorkThread->SendPkg(stTempSspkg);

	return 0;
}

int CSsGmDataOp::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
	LOGRUN_r("CSsGmDataOp call");
	SSPKG stTempSspkg;
	CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
	SS_PKG_GM_DATABASE_OP& rstSsPkgBodyReq = rstSsPkg.m_stBody.m_stGmDatabaseOpreq;
	SS_PKG_GM_DATABASE_OP_RSP& rstSsPkgBodyRsp = stTempSspkg.m_stBody.m_stGmDatabaseOpRsp;
	switch (rstSsPkgBodyReq.m_bHandleType)
	{
	case GM_DT_DEL_PROPS:  //É¾³ýÎïÆ·
		rstSsPkgBodyRsp.m_nErrNo=deleteProps(rstSsPkgBodyReq.m_stHandleInfo.m_stGmdeleteprops, pvPara);
		stTempSspkg.m_stHead.m_wMsgId = SS_MSG_DATABASE_OP_RSP;
		break;

	default:
		break;
	}
	stTempSspkg.m_stHead.m_iDstProcId = rstSsPkg.m_stHead.m_iSrcProcId;
	stTempSspkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
	//rstSsPkgBodyRsp.m_nErrNo = poWorkThread->GetRoleTable().UptRoleName(rstSsPkg.m_stHead.m_ullUin, rstSsPkgBodyReq.m_szName);
	//rstSsPkgBodyRsp.m_nErrNo = poWorkThread->GetRoleTable().CheckExist(rstSsPkgBodyReq.m_szName);
	//StrCpy(rstSsPkgBodyRsp.m_szName, rstSsPkgBodyReq.m_szName, MAX_NAME_LENGTH);
	poWorkThread->SendPkg(stTempSspkg);
	return 0;
}
int CSsGmDataOp::deleteProps(DT_GM_DEL_PROPS m_stHandleInfo, void* pvPara)
{
	int iRet = 0;
	CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
	for (int i = 0; i < m_stHandleInfo.m_nUinCount; i++)
	{
		uint64_t ullUin = m_stHandleInfo.m_UinList[i];
		uint32_t ItemID = m_stHandleInfo.m_dwItemID;
		int32_t count = m_stHandleInfo.m_iOpCount;
		iRet = poWorkThread->GetRoleTable().DelRoleProps(ullUin, ItemID, count);
	}
	return iRet;
}
//int CSsRoleChgNameReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
//{
//	CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
//
//	SSPKG stTempSspkg;
//
//	SS_PKG_NAME_CHANGE_REQ& rstSsPkgBodyReq = rstSsPkg.m_stBody.m_stNameChangeReq;
//	SS_PKG_NAME_CHANGE_RSP& rstSsPkgBodyRsp = stTempSspkg.m_stBody.m_stNameChangeRsp;
//
//	stTempSspkg.m_stHead.m_wMsgId = SS_MSG_NAME_CHANGE_RSP;
//	stTempSspkg.m_stHead.m_iDstProcId = rstSsPkg.m_stHead.m_iSrcProcId;
//	stTempSspkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
//
//	rstSsPkgBodyRsp.m_nErrNo = poWorkThread->GetRoleTable().UptRoleName(rstSsPkg.m_stHead.m_ullUin, rstSsPkgBodyReq.m_szName);
//
//	poWorkThread->SendPkg(stTempSspkg);
//
//	return 0;
//}


	
