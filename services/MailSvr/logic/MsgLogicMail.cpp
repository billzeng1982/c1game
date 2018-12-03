#include "MsgLogicMail.h"
#include "LogMacros.h"
#include "strutil.h"
#include "MailSvrMsgLayer.h"
#include "MailBoxMgr.h"
#include "MailBox.h"
#include "MailPubServData.h"

using namespace PKGMETA;

int CSsMailPlayerStatNtf::HandleServerMsg(SSPKG* pstSsPkg)
{
    SS_PKG_MAIL_PLAYER_STAT_NTF& rstPlayerStateNtf = pstSsPkg->m_stBody.m_stMailPlayerStatNtf;
    if(rstPlayerStateNtf.m_ullUin == 0)
    {
        return ERR_NONE;
    }

    switch(rstPlayerStateNtf.m_bPlayerStat)
    {
        case (uint8_t)PLAYER_STATE_CREATE:
        case (uint8_t)PLAYER_STATE_LOGIN:
        {
            MailBoxInfo* poMailBox = MailBoxMgr::Instance().GetMailBoxInfo(rstPlayerStateNtf.m_ullUin);
            if (!poMailBox)
            {
                LOGERR("Uin(%lu) Player State(%d) Ntf, but Mailbox not found", rstPlayerStateNtf.m_ullUin, rstPlayerStateNtf.m_bPlayerStat);
                break;
            }

            if (rstPlayerStateNtf.m_bPlayerStat == (uint8_t)PLAYER_STATE_CREATE)
            {
                poMailBox->m_stBaseInfo.m_dwPubSeq = CGameTime::Instance().GetCurrSecond();
            }

            //上线时更新一次公共邮件信息
            poMailBox->UpdatePubMail(false);

            //上线时全同步一次邮件数据，其他时候都是增量同步
            poMailBox->SendMailBoxInfoToClient();

            LOGRUN("Uin(%lu) Player State(%d) Ntf", rstPlayerStateNtf.m_ullUin, rstPlayerStateNtf.m_bPlayerStat);
            break;
        }
        case (uint8_t)PLAYER_STATE_LOGOUT:
        {
            break;
        }
        default:
            LOGERR("Uin(%lu) Player State(%d) is err", rstPlayerStateNtf.m_ullUin, rstPlayerStateNtf.m_bPlayerStat);
            return ERR_SYS;
    }

    return ERR_NONE;
}

int CSsMailStatNtf::HandleServerMsg(SSPKG* pstSsPkg)
{
    SS_PKG_MAIL_STAT_NTF& rstMailStateNtf = pstSsPkg->m_stBody.m_stMailStatNtf;
    assert(rstMailStateNtf.m_ullUin != 0);

    MailBoxInfo* poMailBox = MailBoxMgr::Instance().GetMailBoxInfo(rstMailStateNtf.m_ullUin);
    if (!poMailBox)
    {
        LOGERR("Uin(%lu) Player Mail(%u) State(%d) Ntf, but Mailbox not found",
                rstMailStateNtf.m_ullUin, rstMailStateNtf.m_stMailData.m_dwId, rstMailStateNtf.m_bNewStat);
        return ERR_NOT_FOUND;
    }

    int iRet = poMailBox->ChgMailState(rstMailStateNtf.m_stMailData.m_bType, rstMailStateNtf.m_stMailData.m_dwId, rstMailStateNtf.m_bNewStat);
    if (iRet!=ERR_NONE)
    {
        LOGERR("Uin(%lu) Player Mail(%u) Chg to NewState(%d) failed, Ret=%d",
                rstMailStateNtf.m_ullUin, rstMailStateNtf.m_stMailData.m_dwId, rstMailStateNtf.m_bNewStat, iRet);
        return ERR_SYS;
    }

    return ERR_NONE;
}

int CSsMailSyncReq::HandleServerMsg(SSPKG* pstSsPkg)
{
    SS_PKG_MAIL_SYNC_REQ& rstMailSyncReq = pstSsPkg->m_stBody.m_stMailSyncReq;
    MailBoxInfo* poMailBox = MailBoxMgr::Instance().GetMailBoxInfo(rstMailSyncReq.m_ullUin);
    if (!poMailBox)
    {
        LOGERR("Uin(%lu) Player Mail Sync Req, but Mailbox not found",  rstMailSyncReq.m_ullUin);
        return ERR_NOT_FOUND;
    }

    int iRet = poMailBox->UpdatePubMail(true);
    if (!poMailBox)
    {
        LOGERR("Uin(%lu) Player Mail Sync Req failed, Ret=(%d)",  rstMailSyncReq.m_ullUin, iRet);
        return ERR_SYS;
    }

    return ERR_NONE;
}

int CSsMailAddReq::HandleServerMsg(SSPKG* pstSsPkg)
{
    SS_PKG_MAIL_ADD_REQ& rstMailAddReq = pstSsPkg->m_stBody.m_stMailAddReq;

    switch(rstMailAddReq.m_stMailData.m_bType)
    {
    case MAIL_TYPE_PUBLIC_SYSTEM:
        {
            //增加系统公共邮件
            DT_MAIL_PUB_SER_DATA stPubSerMail;
            DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
            stPubSerMail.m_dwId = rstMailAddReq.m_ullStartTimeSec;
            stPubSerMail.m_ullStartTimeSec = rstMailAddReq.m_ullStartTimeSec;
            stPubSerMail.m_ullEndTimeSec = rstMailAddReq.m_ullEndTimeSec;
            StrCpy(stPubSerMail.m_szTitle, rstMailData.m_szTitle, MAX_MAIL_TITLE_LEN);
            StrCpy(stPubSerMail.m_szContent, rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN);
            stPubSerMail.m_bAttachmentCount = rstMailData.m_bAttachmentCount;
            for (int i=0; i<rstMailData.m_bAttachmentCount && i<MAX_MAIL_REWARD_NUM; i++)
            {
                stPubSerMail.m_astAttachmentList[i].m_dwId = rstMailData.m_astAttachmentList[i].m_dwItemId;
                stPubSerMail.m_astAttachmentList[i].m_bType = rstMailData.m_astAttachmentList[i].m_bItemType;
                stPubSerMail.m_astAttachmentList[i].m_dwNum = rstMailData.m_astAttachmentList[i].m_iValueChg;
            }
            SSPKG& rstSsPkg = MailSvrMsgLayer::Instance().GetSsPkg();
            rstSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_PUB_TABLE_ADD_REQ;
            rstSsPkg.m_stBody.m_stMailPubTableAddReq.m_stPubMail = stPubSerMail;
            MailSvrMsgLayer::Instance().SendToMailDBSvr(rstSsPkg);
        }
        break;
    case MAIL_TYPE_PRIVATE_SYSTEM:
    case MAIL_TYPE_PRIVATE_PLAYER:
        {
            for (int i=0; i<(int)rstMailAddReq.m_nUinCount && i<MAX_MAIL_MULTI_USER_NUM; i++)
            {
                // 将组发信息拆解为单条
                MailBoxInfo* pstMailBoxInfo = MailBoxMgr::Instance().GetMailBoxInfo(rstMailAddReq.m_UinList[i]);
                if (!pstMailBoxInfo)
                {
                    continue;
                }
                pstMailBoxInfo->AddPriMail(rstMailAddReq.m_stMailData);
            }
            break;
        }
        default:
            break;
    }

    return ERR_NONE;
}

int CSsMailAddByIdReq::HandleServerMsg(SSPKG* pstSsPkg)
{
    SS_PKG_MAIL_ADD_BY_ID_REQ& rstMailAddReq = pstSsPkg->m_stBody.m_stMailAddByIdReq;
    for (int i=0; i<(int)rstMailAddReq.m_nUinCount && i<MAX_MAIL_MULTI_USER_NUM; i++)
    {
        // 将组发信息拆解为单条
        MailBoxInfo* pstMailBoxInfo = MailBoxMgr::Instance().GetMailBoxInfo(rstMailAddReq.m_UinList[i]);
        if (!pstMailBoxInfo)
        {
            continue;
        }
        pstMailBoxInfo->AddPriMail(rstMailAddReq.m_dwMailResId);
    }

    return ERR_NONE;
}

int CSsMailPubTableGetDataRsp::HandleServerMsg(SSPKG* pstSsPkg)
{
    SS_PKG_MAIL_PUB_TABLE_GET_DATA_RSP& rstRsp = pstSsPkg->m_stBody.m_stMailPubTableGetDataRsp;
    if (ERR_NONE != rstRsp.m_nErrNo)
    {
        LOGERR("Get pub mail error <%d>", rstRsp.m_nErrNo);
        return ERR_NONE;
    }

    for (int i = 0; i < rstRsp.m_wNum; i++)
    {
        MailPubServData::Instance().AddPubMailToWaitQueue(rstRsp.m_astPubMails[i]);
    }

    return 0;
}

int CSsMailPriTableGetDataRsp::HandleServerMsg(SSPKG* pstSsPkg)
{
    SS_PKG_MAIL_PRI_TABLE_GET_DATA_RSP& rstGetMailTableRsp = pstSsPkg->m_stBody.m_stMailPriTableGetDataRsp;
    //MailBoxMgr::Instance().DebugCheck();
    MailBoxMgr::Instance().AsyncGetDataDone(rstGetMailTableRsp.m_ullTransTokenId, rstGetMailTableRsp.m_nErrNo, &rstGetMailTableRsp.m_stData);
    return ERR_NONE;
}

