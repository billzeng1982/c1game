#include "RoleSvrMsgLayer_r.h"
#include "ss_proto.h"
#include "../logic/RoleTableMsg.h"
#include "../logic/RoleGmTableMsg.h"

using namespace PKGMETA;

bool RoleSvrMsgLayer_r::Init()
{
    this->_RegistServerMsg();

    return true;
}

void RoleSvrMsgLayer_r::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER_r( SS_MSG_MAJESTY_REGISTER_REQ,          CSsMajestyRegReq,           m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER_r( SS_MSG_ROLE_CREATE_REQ,               CSsRoleCreateReq,           m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER_r( SS_MSG_ROLE_LOGIN_REQ,                CSsRoleLoginReq,            m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER_r( SS_MSG_ROLE_UPDATE_REQ,               CSsRoleUptReq,              m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER_r( SS_MSG_ROLE_GM_UPDATE_INFO_REQ,       CSsRoleGmUpdateInfoReq,     m_oSsMsgHandlerMap );
	REGISTER_MSG_HANDLER_r( SS_MSG_NAME_CHANGE_QUERY_REQ,         CSsRoleChgNameQueryReq,     m_oSsMsgHandlerMap );	
	REGISTER_MSG_HANDLER_r( SS_MSG_DATABASE_OP,					  CSsGmDataOp, m_oSsMsgHandlerMap);
	//REGISTER_MSG_HANDLER_r( SS_MSG_NAME_CHANGE_REQ,		          CSsRoleChgNameReq,	      m_oSsMsgHandlerMap );	
}

