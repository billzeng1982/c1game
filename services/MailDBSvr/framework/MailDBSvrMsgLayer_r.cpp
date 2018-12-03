#include "MailDBSvrMsgLayer_r.h"
#include "ss_proto.h"
#include "../logic/MailTableMsg.h"

using namespace PKGMETA;

bool MailDBSvrMsgLayer_r::Init()
{
    this->_RegistServerMsg();

    return true;
}

void MailDBSvrMsgLayer_r::_RegistServerMsg()
{

    //私人邮件箱
    //REGISTER_MSG_HANDLER_r( SS_MSG_MAIL_PRI_TABLE_CREATE_REQ,       CSsMailPriTableCreateReq,       m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER_r( SS_MSG_MAIL_PRI_TABLE_GET_DATA_REQ,       CSsMailPriTableGetDataReq,    m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER_r( SS_MSG_MAIL_PRI_TABLE_UPDATE_NTF,        CSsMailPriTableUptNtf,        m_oSsMsgHandlerMap );

    //公共邮件
    REGISTER_MSG_HANDLER_r( SS_MSG_MAIL_PUB_TABLE_GET_DATA_REQ,   CSsMailPubTableGetDataReq,    m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER_r( SS_MSG_MAIL_PUB_TABLE_DEL_NTF,        CSsMailPubTableDelNtf,        m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER_r( SS_MSG_MAIL_PUB_TABLE_ADD_REQ,        CSsMailPubTableAddReq,        m_oSsMsgHandlerMap );
}

