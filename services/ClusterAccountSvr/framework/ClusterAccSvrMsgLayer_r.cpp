#include "ClusterAccSvrMsgLayer_r.h"
#include "ss_proto.h"
#include "../logic/ThreadMsgLogic.h"

using namespace PKGMETA;

bool ClusterAccSvrMsgLayer_r::Init()
{
    this->_RegistServerMsg();

    return true;
}

void ClusterAccSvrMsgLayer_r::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER_r( SS_MSG_CLUSTER_ACC_NEW_ROLE_REG,    SsClusterAccNewRoleReg,         m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER_r( SS_MSG_CLUSTER_ACC_CHG_ROLE_NAME,   SsClusterAccChgRoleName,        m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER_r( SS_MSG_GET_ONE_CLUSTER_ACC_INFO_REQ, SsGetOneClusterAccInfoReq,     m_oSsMsgHandlerMap);
}


