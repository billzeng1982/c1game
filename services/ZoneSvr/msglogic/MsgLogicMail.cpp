#include "LogMacros.h"
#include "MsgLogicMail.h"
#include "common_proto.h"
#include "../gamedata/GameDataMgr.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../module/player/Player.h"
#include "../module/player/PlayerMgr.h"
#include "../module/Mail.h"

using namespace PKGMETA;

int MailSendReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    return 0;
}

int MailDeleteNtf_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("MailDelete poPlayer is null");
        return -1;
    }

    CS_PKG_MAIL_DELETE_NTF& rstCsPkgBodyNtf = rstCsPkg.m_stBody.m_stMailDeleteNtf;
    Mail::Instance().SendMailStatToMailSvr(poPlayer->GetUin(), MAIL_STATE_DELETED, rstCsPkgBodyNtf.m_bMailType, rstCsPkgBodyNtf.m_dwMailId);


    return 0;
}

int MailOpenNtf_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("MailOpen poPlayer is null");
        return -1;
    }

    CS_PKG_MAIL_OPEN_NTF& rstCsPkgBodyNtf = rstCsPkg.m_stBody.m_stMailOpenNtf;
    Mail::Instance().SendMailStatToMailSvr(poPlayer->GetUin(), MAIL_STATE_OPENED, rstCsPkgBodyNtf.m_bMailType, rstCsPkgBodyNtf.m_dwMailId);


    return 0;
}

int MailDrawAttachmentReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("MailDraw poPlayer is null");
        return -1;
    }

    CS_PKG_MAIL_DRAW_ATTACHMENT_REQ& rstCsPkgBodyNtf = rstCsPkg.m_stBody.m_stMailDrawAttachment;
    Mail::Instance().SendMailStatToMailSvr(poPlayer->GetUin(), MAIL_STATE_DRAWED, rstCsPkgBodyNtf.m_bMailType, rstCsPkgBodyNtf.m_dwMailId);


    return 0;
}

int MailSyncServerDataNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    // 保存最新的全局邮件数据
    SS_PKG_MAIL_SYNC_SERVER_DATA_NTF& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stMailSyncServerDataNtf;
    Mail::Instance().HandleSyncServerDataFromMailSvr(rstSsPkgBodyRsp.m_dwMailId);
    LOGRUN("MailSyncServerNtf_SS, MailId<%u>", rstSsPkgBodyRsp.m_dwMailId);
    return 0;
}

int MailSyncPlayerDataNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_MAIL_SYNC_PLAYER_DATA_NTF& rstSsPkgBodyNtf = rstSsPkg.m_stBody.m_stMailSyncPlayerDataNtf;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkgBodyNtf.m_ullUin);
    if (!poPlayer)
    {//下线了
        if (0 == rstSsPkgBodyNtf.m_ullUin)
        {
            LOGERR("Uin<%lu> MailSycPlayerData poPlayer is null", rstSsPkgBodyNtf.m_ullUin);
        }
        return -1;
    }

    Mail::Instance().SyncPlayerDataToClient(poPlayer, rstSsPkgBodyNtf);

    LOGRUN("Uin<%lu> MailSyncPlayerNtf_SS, Cnt<%u>", rstSsPkgBodyNtf.m_ullUin, rstSsPkgBodyNtf.m_wCount);

    return 0;
}

int MailStatNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_MAIL_STAT_NTF& rstSsPkgBodyNtf = rstSsPkg.m_stBody.m_stMailStatNtf;
    SC_PKG_MAIL_DRAW_ATTACHMENT_RSP& rstScPkgBody = m_stScPkg.m_stBody.m_stMailDrawRsp;
    if (rstSsPkgBodyNtf.m_bNewStat == MAIL_STATE_DRAWED)
    {
        if (ERR_NONE == rstSsPkgBodyNtf.m_nErrNo)
        {
            Mail::Instance().HandleDrawMsgFromMailSvr(rstSsPkgBodyNtf.m_ullUin, rstSsPkgBodyNtf.m_stMailData);
            return 0;
        }
        else
        {
            Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkgBodyNtf.m_ullUin);
            if (!poPlayer)
            {
                LOGERR("Uin<%lu> poPlayer is null, MaildId<%u>", rstSsPkgBodyNtf.m_ullUin, rstSsPkgBodyNtf.m_stMailData.m_dwId);
                return 0;
            }
             m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MAIL_DRAW_ATTACHMENT_RSP;
             rstScPkgBody.m_nErrNo = rstSsPkgBodyNtf.m_nErrNo;
             rstScPkgBody.m_bMailType = rstSsPkgBodyNtf.m_stMailData.m_bType;
             rstScPkgBody.m_dwMailId = rstSsPkgBodyNtf.m_stMailData.m_dwId;

             rstScPkgBody.m_stSyncItemInfo.m_bSyncItemCount = 0;
             ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        }
    }
    else
    {// 分支加个打印
        LOGERR("Mail stat error, Uin<%lu>, MailId<%u>,Type<%u>,State<%u>,From<%lu>,Time<%lu>,AttCnt<%u>",
            rstSsPkgBodyNtf.m_ullUin, rstSsPkgBodyNtf.m_stMailData.m_dwId, rstSsPkgBodyNtf.m_stMailData.m_bType, rstSsPkgBodyNtf.m_stMailData.m_bState,
            rstSsPkgBodyNtf.m_stMailData.m_ullFromUin, rstSsPkgBodyNtf.m_stMailData.m_ullTimeStampMs, rstSsPkgBodyNtf.m_stMailData.m_bAttachmentCount);
    }


    return 0;
}

