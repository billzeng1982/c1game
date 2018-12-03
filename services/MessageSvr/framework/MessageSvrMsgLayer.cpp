#include "MessageSvrMsgLayer.h"
#include "LogMacros.h"
#include "ObjectPool.h"
#include "hash_func.cpp"
#include "../MessageSvr.h"
#include "../msglogic/MsgLogicMessage.h"

using namespace PKGMETA;

MessageSvrMsgLayer::MessageSvrMsgLayer() : CMsgLayerBase( sizeof(PKGMETA::SCPKG)*2 , sizeof(PKGMETA::SSPKG)*2 )
{
    m_pstConfig = NULL;
}

bool MessageSvrMsgLayer::Init()
{
    MESSAGESVRCFG& rstDBSvrCfg =  MessageSvr::Instance().GetConfig();
    if( _Init(rstDBSvrCfg.m_iBusGCIMKey, rstDBSvrCfg.m_iProcID) < 0 )
    {
        return false;
    }

    m_pstConfig = &rstDBSvrCfg;

    return true;
}


void MessageSvrMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER(SS_MSG_MESSAGE_SEND_REQ,          MessageSendReq_SS,        m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MESSAGE_GET_BOX_REQ,       MessageGetBoxReq_SS,      m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_MESSAGE_DEL_BOX_REQ,       MessageDelBoxReq_SS,      m_oSsMsgHandlerMap);
}

int MessageSvrMsgLayer::DealPkg()
{
    int iRecvBytes = 0;
    int i = 0;

    for (; i < DEAL_PKG_PER_LOOP; i++)
    {
        iRecvBytes = m_oCommBusLayer.Recv();
        if (iRecvBytes < 0)
        {
            LOGERR("bus recv error!");
            return -1;
        }

        if (0 == iRecvBytes)
        {
            break;
        }

        this->_DealSvrPkg();
    }
    return i;
}

int MessageSvrMsgLayer::SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(m_pstConfig->m_iZoneSvrID, rstSsPkg);
}

