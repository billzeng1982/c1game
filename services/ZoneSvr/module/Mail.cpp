#include "Mail.h"
#include "player/Player.h"
#include "player/PlayerMgr.h"
#include "LogMacros.h"
#include "../../gamedata/GameDataMgr.h"
#include "../../framework/ZoneSvrMsgLayer.h"
#include "../module/Pay.h"
#include "Item.h"
#include "ZoneLog.h"
#include "dwlog_svr.h"

using namespace std;
using namespace PKGMETA;
using namespace DWLOG;

Mail::Mail()
{
    m_dwPubMailSeq = 0;
}

Mail::~Mail()
{

}

void Mail::UpdatePlayerData(PlayerData* pstData)
{
    if (pstData->m_dwPubMailSeq < m_dwPubMailSeq)
    {
        this->SyncPlayerPubMail(pstData);
        pstData->m_dwPubMailSeq = m_dwPubMailSeq;
    }
}

void Mail::HandleSyncServerDataFromMailSvr(uint32_t dwMailId)
{
    m_dwPubMailSeq = dwMailId;
    return;
}


void Mail::HandleDrawMsgFromMailSvr(uint64_t ullUin, PKGMETA::DT_MAIL_DATA& rstMailData)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MAIL_DRAW_ATTACHMENT_RSP;
    SC_PKG_MAIL_DRAW_ATTACHMENT_RSP& rstScPkgBody = m_stScPkg.m_stBody.m_stMailDrawRsp;

    rstScPkgBody.m_stSyncItemInfo.m_bSyncItemCount = 0;

    for (int i=0; i<rstMailData.m_bAttachmentCount && i<MAX_MAIL_ATTACHMENT_NUM; i++)
    {
        //补偿订单
        if (rstMailData.m_astAttachmentList[i].m_bItemType == ITEM_TYPE_PAY_ORDER)
        {
            SS_PKG_SDK_PAY_CB_NTF stSdkPayCbNtf;
            stSdkPayCbNtf.m_stPayCbInfo.m_dwProductID = rstMailData.m_astAttachmentList[i].m_dwItemId;
            stSdkPayCbNtf.m_stPayCbInfo.m_ullRoleID = ullUin;
            StrCpy(stSdkPayCbNtf.m_stPayCbInfo.m_szUserID, poPlayer->GetAccountName(), PKGMETA::MAX_NAME_LENGTH);
            StrCpy(stSdkPayCbNtf.m_stPayCbInfo.m_szOrderId, "CompensationOder", MAX_LEN_SDK_PARA);
            Pay::Instance().DoPayOk(stSdkPayCbNtf);
        }
        else
        {
            Item::Instance().RewardItem(&poPlayer->GetPlayerData(),
                rstMailData.m_astAttachmentList[i].m_bItemType,
                rstMailData.m_astAttachmentList[i].m_dwItemId,
                rstMailData.m_astAttachmentList[i].m_iValueChg,
                rstScPkgBody.m_stSyncItemInfo, METHOD_MAIL_DRAW);
        }
        

         LOGRUN("Draw Mail Uin<%lu>, Id<%u>,ItemType<%u> ItemId<%u>,ValueChg<%d>",ullUin, rstMailData.m_dwId,
            rstMailData.m_astAttachmentList[i].m_bItemType,
            rstMailData.m_astAttachmentList[i].m_dwItemId,
            rstMailData.m_astAttachmentList[i].m_iValueChg
            );
    }

    LOGRUN("Draw Primail Uin<%lu>, Id<%u>,Type<%u>,State<%u>,From<%lu>,Time<%lu>,AttCnt<%u>",
        ullUin, rstMailData.m_dwId, rstMailData.m_bType, rstMailData.m_bState,
        rstMailData.m_ullFromUin, rstMailData.m_ullTimeStampMs, rstMailData.m_bAttachmentCount);
    rstScPkgBody.m_nErrNo = ERR_NONE;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return;
}

// 同步玩家数据到客户端
int Mail::SyncPlayerDataToClient(Player* poPlayer, SS_PKG_MAIL_SYNC_PLAYER_DATA_NTF& rstSsPkgBodyNtf)
{
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MAIL_SYNC_NTF;
    SC_PKG_MAIL_SYNC_NTF& rstScMailSyncNtf = m_stScPkg.m_stBody.m_stMailSyncNtf;

    PlayerData* pstData = &poPlayer->GetPlayerData();
    pstData->m_dwPubMailSeq = rstSsPkgBodyNtf.m_dwPubSeq;
    if (rstSsPkgBodyNtf.m_wCount == 0)
    {
        return 0;
    }

    int iSendTimes = (rstSsPkgBodyNtf.m_wCount - 1) / MAX_MAIL_PROTO_NUM + 1;
    int iRemainder = rstSsPkgBodyNtf.m_wCount - (iSendTimes -1) * MAX_MAIL_PROTO_NUM;
    for (int j=0; j<iSendTimes; j++)
    {
        rstScMailSyncNtf.m_wCount = (j==(iSendTimes-1)) ? iRemainder : MAX_MAIL_PROTO_NUM;

        for (int i=0; i<rstScMailSyncNtf.m_wCount; i++)
        {
            rstScMailSyncNtf.m_astMailList[i] = rstSsPkgBodyNtf.m_astMailList[i + j*MAX_MAIL_PROTO_NUM];
        }

        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }

    return 0;
}

int Mail::SendPlayerStatToMailSvr(uint64_t ullUin, uint8_t bState)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_PLAYER_STAT_NTF;
    m_stSsPkg.m_stBody.m_stMailPlayerStatNtf.m_ullUin = ullUin;
    m_stSsPkg.m_stBody.m_stMailPlayerStatNtf.m_bPlayerStat = bState;
    LOGRUN("Uin<%lu> State<%u> send mail player state to MailSvr", ullUin, bState);
    return ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
}

int Mail::SendMailStatToMailSvr(uint64_t ullUin, uint8_t bState, uint8_t bMailType, uint32_t dwMailId)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_STAT_NTF;
    m_stSsPkg.m_stBody.m_stMailStatNtf.m_ullUin = ullUin;                       // Uin
    m_stSsPkg.m_stBody.m_stMailStatNtf.m_bNewStat = bState;                     // 邮件状态 = ullUin;
    m_stSsPkg.m_stBody.m_stMailStatNtf.m_stMailData.m_bType = bMailType;        // 邮件类型
    m_stSsPkg.m_stBody.m_stMailStatNtf.m_stMailData.m_dwId = dwMailId;          // 邮件Id
    LOGRUN("Uin<%lu> chg mail state Id<%u>, Type<%u> ChgState<%u>", ullUin, dwMailId, bMailType, bState);
    return ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
}

void Mail::SyncPlayerPubMail(PlayerData* pstData)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_SYNC_REQ;
    m_stSsPkg.m_stBody.m_stMailStatNtf.m_ullUin = pstData->GetRoleBaseInfo().m_ullUin;
    m_stSsPkg.m_stBody.m_stMailSyncReq.m_ullUin = pstData->GetRoleBaseInfo().m_ullUin;
    ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
}

int Mail::TestSendPriMail(uint64_t ullUin1, uint64_t ullUin2)
{
    SSPKG stSsPkg;
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    stSsPkg.m_stBody.m_stMailAddReq.m_nUinCount = 2;
    stSsPkg.m_stBody.m_stMailAddReq.m_UinList[0] = ullUin1;
    stSsPkg.m_stBody.m_stMailAddReq.m_UinList[1] = ullUin2;

    DT_MAIL_DATA& rstMailData = stSsPkg.m_stBody.m_stMailAddReq.m_stMailData;
    rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
    rstMailData.m_bState = MAIL_STATE_UNOPENED;
    strcpy(rstMailData.m_szTitle, "Test");
    strcpy(rstMailData.m_szContent, "TestContent");
    rstMailData.m_bAttachmentCount = 0;
    rstMailData.m_ullFromUin = 0;
    stSsPkg.m_stBody.m_stMailAddReq.m_stMailData.m_bAttachmentCount = 1;
    stSsPkg.m_stBody.m_stMailAddReq.m_stMailData.m_astAttachmentList[0].m_bItemType = 3;
    stSsPkg.m_stBody.m_stMailAddReq.m_stMailData.m_astAttachmentList[0].m_dwItemId = 1;
    stSsPkg.m_stBody.m_stMailAddReq.m_stMailData.m_astAttachmentList[0].m_iValueChg = 4;
    return ZoneSvrMsgLayer::Instance().SendToMailSvr(stSsPkg);
}

int Mail::TestSendPubMail()
{
    SSPKG stSsPkg = {0};
    static int iTest = 3;
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    snprintf(stSsPkg.m_stBody.m_stMailAddReq.m_stMailData.m_szTitle, MAX_MAIL_TITLE_LEN, "TestTitleee%d", iTest );
    strcpy(stSsPkg.m_stBody.m_stMailAddReq.m_stMailData.m_szContent, "TestContext" );
    stSsPkg.m_stBody.m_stMailAddReq.m_stMailData.m_bType = MAIL_TYPE_PUBLIC_SYSTEM;
    stSsPkg.m_stBody.m_stMailAddReq.m_nUinCount = 0;
    stSsPkg.m_stBody.m_stMailAddReq.m_ullStartTimeSec = CGameTime::Instance().GetCurrSecond();
    stSsPkg.m_stBody.m_stMailAddReq.m_ullEndTimeSec = CGameTime::Instance().GetCurrSecond() + SECONDS_OF_DAY;
    stSsPkg.m_stBody.m_stMailAddReq.m_stMailData.m_bAttachmentCount = 1;
    stSsPkg.m_stBody.m_stMailAddReq.m_stMailData.m_astAttachmentList[0].m_bItemType = 3;
    stSsPkg.m_stBody.m_stMailAddReq.m_stMailData.m_astAttachmentList[0].m_dwItemId = 1;
    stSsPkg.m_stBody.m_stMailAddReq.m_stMailData.m_astAttachmentList[0].m_iValueChg = 4;
    ZoneSvrMsgLayer::Instance().SendToMailSvr(stSsPkg);
    iTest++;
    return 0;
}
