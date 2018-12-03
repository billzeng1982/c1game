#include "define.h"
#include "LogMacros.h"
#include "LogQuerySvr.h"
#include "LogQuerySvrMsgLayer.h"
#include "LogQueryMsgLogic.h"

using namespace PKGMETA;

LogQuerySvrMsgLayer::LogQuerySvrMsgLayer() : CMsgLayerBase( sizeof(PKGMETA::SCPKG)*2 , sizeof(PKGMETA::SSPKG)*2 )
{

}


void LogQuerySvrMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER(SS_MSG_IDIP_REQ, IdipReq_SS, m_oSsMsgHandlerMap);
}


bool LogQuerySvrMsgLayer::Init()
{
    m_pstConfig = &(LogQuerySvr::Instance().GetConfig());
    if (!this->_Init(m_pstConfig->m_iBusGCIMKey, m_pstConfig->m_iProcID))
    {
        return false;
    }

    return true;
}

int LogQuerySvrMsgLayer::DealPkg()
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

int LogQuerySvrMsgLayer::SendToIdipAgent(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(m_pstConfig->m_iIdipAgentID, rstSsPkg);
}

