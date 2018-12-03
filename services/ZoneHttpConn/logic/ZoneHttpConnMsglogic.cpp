#include "ZoneHttpConnMsglogic.h"
#include "framework/ZoneHttpConnMsgLayer.h"

int GetPlayerWebTaskInfoRsp::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    uint32_t dwReqId = rstSsPkg.m_stHead.m_ullReservId;
    SS_PKG_ROLE_WEB_TASK_INFO_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stRoleWebTaskInfoRsp;
    ZoneHttpConnMsgLayer::Instance().GetHttpControler().SendGetRoleWebTaskInfoHttpRsp(dwReqId, rstSsRsp);
    return 0;
}

int GetWebTaskRwdRsp::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    uint32_t dwReqId = rstSsPkg.m_stHead.m_ullReservId;
    SS_PKG_WEB_TASK_GET_RWD_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stRoleGetWebTaskRwdRsp;
    ZoneHttpConnMsgLayer::Instance().GetHttpControler().SendGetRoleWebTaskRwdHttpRsp(dwReqId, rstSsRsp);
    return 0;
}
