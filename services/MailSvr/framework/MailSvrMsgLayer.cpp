#include "MailSvrMsgLayer.h"
#include "LogMacros.h"
#include "MailSvr.h"
#include "MsgLogicMail.h"

using namespace PKGMETA;

MailSvrMsgLayer::MailSvrMsgLayer() : CMsgLayerBase_c(sizeof(PKGMETA::SSPKG) + 256)
{
    m_pstConfig = NULL;
}

bool MailSvrMsgLayer::Init()
{
    MAILSVRCFG& rstDBSvrCfg = MailSvr::Instance().GetConfig();
    if (_Init(rstDBSvrCfg.m_iBusGCIMKey, rstDBSvrCfg.m_iProcID) < 0)
    {
        return false;
    }

    m_pstConfig = &rstDBSvrCfg;

    return true;
}

void MailSvrMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER_c( SS_MSG_MAIL_PLAYER_STAT_NTF,         CSsMailPlayerStatNtf,       m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER_c( SS_MSG_MAIL_STAT_NTF,                CSsMailStatNtf,             m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER_c( SS_MSG_MAIL_SYNC_REQ,                CSsMailSyncReq,             m_oSsMsgHandlerMap );

    REGISTER_MSG_HANDLER_c( SS_MSG_MAIL_ADD_REQ,                 CSsMailAddReq,              m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER_c( SS_MSG_MAIL_ADD_BY_ID_REQ,           CSsMailAddByIdReq,          m_oSsMsgHandlerMap );

    REGISTER_MSG_HANDLER_c( SS_MSG_MAIL_PUB_TABLE_GET_DATA_RSP,          CSsMailPubTableGetDataRsp,  m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER_c( SS_MSG_MAIL_PRI_TABLE_GET_DATA_RSP,          CSsMailPriTableGetDataRsp,  m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER_c( SS_MSG_MAIL_PRI_TABLE_GET_DATA_RSP,          CSsMailPriTableGetDataRsp,  m_oSsMsgHandlerMap );
}

int MailSvrMsgLayer::DealPkg()
{
    int iRecvBytes = 0;
    int iDealPkgNum = 0;
    int i = 0;

    for (; i<DEAL_PKG_PER_LOOP; i++ )
    {
        iRecvBytes = m_oCommBusLayer.Recv();
        if( iRecvBytes < 0 )
        {
            LOGERR_r("bus recv error!");
            return -1;
        }
        if( 0 == iRecvBytes )
        {
            break;
        }

        this->_DealSvrPkg();
        iDealPkgNum++;
    }

    return iDealPkgNum;
}

int MailSvrMsgLayer::SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(MailSvr::Instance().GetConfig().m_iZoneSvrID, rstSsPkg);
}

int MailSvrMsgLayer::SendToMailDBSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(MailSvr::Instance().GetConfig().m_iMailDBSvrID, rstSsPkg);
}

