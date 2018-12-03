#pragma once
#include "CoDataFrame.h"
#include "../framework/MailSvrMsgLayer.h"

using namespace PKGMETA;

class AsyncGetMailBoxAction : public CoGetDataAction
{
public:
    AsyncGetMailBoxAction() { m_ullPlayerId = 0; }
    virtual ~AsyncGetMailBoxAction(){}

    virtual bool Execute( )
    {
        SSPKG& rstSsPkg = MailSvrMsgLayer::Instance().GetSsPkg();
        rstSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_PRI_TABLE_GET_DATA_REQ;

        SS_PKG_MAIL_PRI_TABLE_GET_DATA_REQ& rstGetMailReq = rstSsPkg.m_stBody.m_stMailPriTableGetDataReq;
        rstGetMailReq.m_ullUin = m_ullPlayerId;
        rstGetMailReq.m_ullTransTokenId = this->GetToken();

        MailSvrMsgLayer::Instance().SendToMailDBSvr(rstSsPkg);
        return true;
    }

    void SetPlayerId(uint64_t ullPlayerId) { m_ullPlayerId = ullPlayerId; }

private:
    uint64_t m_ullPlayerId;
};

