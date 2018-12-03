#include "XiYouSDKSvrMsgLayer_r.h"
#include "ss_proto.h"
#include "../logic/XiYouSDKMsgLogic.h"

using namespace PKGMETA;

bool XiYouSDKSvrMsgLayer_r::Init()
{
    this->_RegistServerMsg();

    return true;
}

void XiYouSDKSvrMsgLayer_r::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER_r(SS_MSG_SDK_ACCOUNT_LOGIN_REQ,    SDKAccountLoginReq_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_r(SS_MSG_SDK_GET_ORDERID_REQ,      SDKGetOrderIDReq_SS, m_oSsMsgHandlerMap);
}



