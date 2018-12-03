#include "MailPubServData.h"
#include "GameTime.h"
#include "TableDefine.h"
#include "MailSvrMsgLayer.h"

using namespace PKGMETA;

int PubMailServerDataCmp(const void *pstFirst, const void *pstSecond)
{
    const DT_MAIL_PUB_SER_DATA* pstItemFirst = (DT_MAIL_PUB_SER_DATA*)pstFirst;
    const DT_MAIL_PUB_SER_DATA* pstItemSecond = (DT_MAIL_PUB_SER_DATA*)pstSecond;

    int iResult = (int)pstItemFirst->m_dwId - (int)pstItemSecond->m_dwId;
    return iResult;
}

MailPubServData::MailPubServData()
{
    m_iPubMailsCount = 0;
}

MailPubServData::~MailPubServData()
{

}

bool MailPubServData::Init()
{
    if (!_InitBase())
    {
        LOGERR("InitBase failed");
        return false;
    }

    if (!_InitRes())
    {
        LOGERR("InitRes failed");
        return false;
    }

    return true;
}

bool MailPubServData::_InitBase()
{
    SSPKG& rstSsPkg = MailSvrMsgLayer::Instance().GetSsPkg();
    rstSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_PUB_TABLE_GET_DATA_REQ;
    MailSvrMsgLayer::Instance().SendToMailDBSvr(rstSsPkg);
    return true;
}

bool MailPubServData::_InitRes()
{
    DT_MAIL_PUB_SER_DATA stPubSerMail;
    ResPubMailMgr_t& rstResMailMgr = CGameDataMgr::Instance().GetResPubMailMgr();
    for (int i=0; i<rstResMailMgr.GetResNum(); i++)
    {
        RESPUBMAIL* pResMail = rstResMailMgr.GetResByPos(i);
        assert(pResMail);

        if (!this->_ConvResToData(pResMail, &stPubSerMail))
        {
            return false;
        }

        this->AddPubMailToWaitQueue(stPubSerMail);
    }
    return true;
}

bool MailPubServData::_ConvResToData(RESPUBMAIL* pResMail, DT_MAIL_PUB_SER_DATA* pstPubMailData)
{
    time_t tTime;
    if (CGameTime::Instance().ConvStrToTime(pResMail->m_szStartTimeSec, tTime) < 0)
    {
        LOGERR("Res start time convert failed, pos(%u)", pResMail->m_dwId);
        return false;
    }
    pstPubMailData->m_ullStartTimeSec = (uint64_t)tTime;

    if (CGameTime::Instance().ConvStrToTime(pResMail->m_szEndTimeSec, tTime) < 0)
    {
        LOGERR("Res end time convert failed, pos(%u)", pResMail->m_dwId);
        return false;
    }
    pstPubMailData->m_ullEndTimeSec = (uint64_t)tTime;

    pstPubMailData->m_dwId = (uint32_t)pstPubMailData->m_ullStartTimeSec;
    pstPubMailData->m_bState = 1;
    StrCpy(pstPubMailData->m_szTitle, pResMail->m_szTitle, MAX_MAIL_TITLE_LEN);
    StrCpy(pstPubMailData->m_szContent, pResMail->m_szContent, MAX_MAIL_CONTENT_LEN);
    pstPubMailData->m_bAttachmentCount = pResMail->m_bAttachmentNum;
    for (int i=0; i<pResMail->m_bAttachmentNum; i++)
    {
        pstPubMailData->m_astAttachmentList[i].m_dwId = pResMail->m_astAttachmentList[i].m_dwId;
        pstPubMailData->m_astAttachmentList[i].m_bType = pResMail->m_astAttachmentList[i].m_bType;
        pstPubMailData->m_astAttachmentList[i].m_dwNum = pResMail->m_astAttachmentList[i].m_dwNum;
    }

    return true;
}

void MailPubServData::_AddPubMail(DT_MAIL_PUB_SER_DATA* pstPubMail)
{
    if ((m_iPubMailsCount != 0) && (pstPubMail->m_dwId <= m_astPubMails[m_iPubMailsCount -1].m_dwId))
    {
        LOGERR("AddPubMail failed, last mail id(%u), add mail id(%u)",
                m_astPubMails[m_iPubMailsCount -1].m_dwId, pstPubMail->m_dwId);
    }
    else
    {
        m_astPubMails[m_iPubMailsCount++] = *pstPubMail;
        LOGRUN("AddPubMail, mail id(%u)", pstPubMail->m_dwId);
        this->_SyncToZoneSvr();
    }
}

void MailPubServData::Update()
{
    uint64_t ullNowTimeSec = (uint64_t)CGameTime::Instance().GetCurrSecond();

    // 新增邮件
    while(!m_stPubMailWaitQueue.Empty() && m_stPubMailWaitQueue.Top().m_ullStartTimeSec <= ullNowTimeSec)
    {
        this->_AddPubMail(&m_stPubMailWaitQueue.Top());
        m_stPubMailWaitQueue.Pop();
    }
}

void MailPubServData::Fini()
{

}

void MailPubServData::_SyncToZoneSvr()
{
    SSPKG& rstSsPkg = MailSvrMsgLayer::Instance().GetSsPkg();
    rstSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_SYNC_SERVER_DATA_NTF;
    SS_PKG_MAIL_SYNC_SERVER_DATA_NTF& rstSsPkgBody = rstSsPkg.m_stBody.m_stMailSyncServerDataNtf;
    rstSsPkgBody.m_dwMailId = m_astPubMails[m_iPubMailsCount -1].m_dwId;
    MailSvrMsgLayer::Instance().SendToZoneSvr(rstSsPkg);
}

DT_MAIL_PUB_SER_DATA* MailPubServData::GetPubSerDataById(uint32_t dwMailId)
{
    DT_MAIL_PUB_SER_DATA stPubSerMail;
    stPubSerMail.m_dwId = dwMailId;

    int iEqual = 0;
    int iIndex = MyBSearch(&stPubSerMail, m_astPubMails, m_iPubMailsCount, sizeof(DT_MAIL_PUB_SER_DATA), &iEqual, PubMailServerDataCmp);
    if (!iEqual)
    {
        LOGERR("server pub sys mail not found, id %u", dwMailId);
        return NULL;
    }

    return &m_astPubMails[iIndex];
}

DT_MAIL_PUB_SER_DATA* MailPubServData::GetPubSerDataByPos(int iPos)
{
    return &m_astPubMails[iPos];
}

int MailPubServData::GetPosById(uint32_t dwMailId)
{
    DT_MAIL_PUB_SER_DATA stPubSerMail;
    stPubSerMail.m_dwId = dwMailId;

    int iEqual = 0;
    int iIndex = MyBSearch(&stPubSerMail, m_astPubMails, m_iPubMailsCount, sizeof(DT_MAIL_PUB_SER_DATA), &iEqual, PubMailServerDataCmp);
    if (!iEqual)
    {
        LOGERR("server pub sys mail not found, id(%u), index(%d)", dwMailId, iIndex);
    }

    return iIndex;
}

void MailPubServData::AddPubMailToWaitQueue(DT_MAIL_PUB_SER_DATA& rstPubMailData)
{
    m_stPubMailWaitQueue.Push(rstPubMailData);
}


