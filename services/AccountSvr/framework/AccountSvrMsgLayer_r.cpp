#include "AccountSvrMsgLayer_r.h"
#include "ss_proto.h"
#include "../logic/AccountTableMsg.h"

using namespace PKGMETA;

bool AccountSvrMsgLayer_r::Init()
{
    this->_RegistServerMsg();

    return true;
}

void AccountSvrMsgLayer_r::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER_r( SS_MSG_ACCOUNT_LOGIN_REQ,                     CSsAccLoginReq,        m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER_r( SS_MSG_ACCOUNT_GM_UPDATE_BANTIME_REQ,         CSsGmUpdateBanTimeReq, m_oSsMsgHandlerMap );
}

