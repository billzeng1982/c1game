#include "MailBox.h"
#include "LogMacros.h"
#include "MailSvrMsgLayer.h"
#include "MailSvrPool.h"
#include "MailPubServData.h"
#include "GameDataMgr.h"
#include "GameTime.h"
#include "strutil.h"
#include "PKGMETA_metalib.h"
#include "MailBoxMgr.h"

using namespace PKGMETA;

int PubMailDataCmp(const void *pstFirst, const void *pstSecond)
{
    DT_MAIL_PUB_PLAYER_DATA* pstItemFirst = (DT_MAIL_PUB_PLAYER_DATA*)pstFirst;
    DT_MAIL_PUB_PLAYER_DATA* pstItemSecond = (DT_MAIL_PUB_PLAYER_DATA*)pstSecond;

    int iResult = (int)pstItemFirst->m_dwMailId - (int)pstItemSecond->m_dwMailId;
    return iResult;
}

MailBoxInfo::MailBoxInfo()
{
    m_wPubMailCount = 0;
    m_wPriMailCount = 0;
    m_oPriMailMap.clear();
}

MailBoxInfo::~MailBoxInfo()
{
    this->Clear();
}

void MailBoxInfo::Clear()
{
    m_oMailMapIter = m_oPriMailMap.begin();
    for (int i=0; m_oMailMapIter != m_oPriMailMap.end() && i<m_wPriMailCount; i++, m_oMailMapIter++)
    {
        DT_MAIL_DATA* pstMailData = m_oMailMapIter->second;
        MailSvrPool::Instance().PriMailDataPool().Release(pstMailData);
    }
    m_oPriMailMap.clear();
}

int MailBoxInfo::PackMailBoxInfo(DT_MAIL_BOX_DATA& rstData)
{
    DT_MAIL_BOX_INFO& stMailBoxInfo = MailBoxMgr::Instance().m_stMailBoxInfo4Pack;

    //私有邮件
    stMailBoxInfo.m_wPrivateCount = m_wPriMailCount;

    m_oMailMapIter = m_oPriMailMap.begin();
    for (int i=0; m_oMailMapIter!=m_oPriMailMap.end() && i<m_wPriMailCount; i++, m_oMailMapIter++)
    {
        stMailBoxInfo.m_astPrivateMailList[i] = *(m_oMailMapIter->second);
    }

    //公共邮件
    stMailBoxInfo.m_wPublicCount = m_wPubMailCount;
    memcpy(stMailBoxInfo.m_astPublicMailList, m_astPubMailData, sizeof(DT_MAIL_PUB_PLAYER_DATA)*m_wPubMailCount);

    rstData.m_stBaseInfo = m_stBaseInfo;

    size_t ulUseSize = 0;
    int iRet = stMailBoxInfo.pack((char*)rstData.m_stMailBoxBlob.m_szData, sizeof(rstData.m_stMailBoxBlob.m_szData), &ulUseSize);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("Uin(%lu) pack DT_MAIL_BOX_BLOB failed! Ret(%d) PriCnt(%u) PubCnt(%u)",
                m_stBaseInfo.m_ullUin, iRet, m_wPriMailCount, m_wPubMailCount);
        return ERR_SYS;
    }
    rstData.m_stMailBoxBlob.m_iLen = (int)ulUseSize;

    return ERR_NONE;
}

int MailBoxInfo::InitFromDB(DT_MAIL_BOX_DATA& rstData)
{
    m_stBaseInfo = rstData.m_stBaseInfo;

    uint16_t wVersion = m_stBaseInfo.m_wVersion;
    size_t ulUseSize = 0;
    m_stBaseInfo.m_wVersion = PKGMETA::MetaLib::getVersion();

    MailBoxMgr::Instance().DebugCheck();
    DT_MAIL_BOX_INFO& stMailBoxInfo = MailBoxMgr::Instance().m_stMailBoxInfo4Unpack;
    int iRet = stMailBoxInfo.unpack((char*)rstData.m_stMailBoxBlob.m_szData, sizeof(rstData.m_stMailBoxBlob.m_szData), &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("Uin(%lu) unpack DT_MAIL_BOX_PRI_BLOB failed, Ret(%d)", rstData.m_stBaseInfo.m_ullUin, iRet);
        return iRet;
    }
    MailBoxMgr::Instance().DebugCheck();

    //私人邮件信息
    m_wPriMailCount = stMailBoxInfo.m_wPrivateCount;
    for (int i=0; i<m_wPriMailCount; i++)
    {
        DT_MAIL_DATA* pstMailData = MailSvrPool::Instance().PriMailDataPool().Get();
        if (!pstMailData)
        {
            LOGERR("Uin(%lu) unpack DT_MAIL_BOX_PRI_BLOB failed, get mem from pool failed", rstData.m_stBaseInfo.m_ullUin);
            return ERR_SYS;
        }

        *pstMailData = stMailBoxInfo.m_astPrivateMailList[i];
        m_oPriMailMap.insert(MapId2Mail_t::value_type(pstMailData->m_dwId, pstMailData));
    }

    //公共邮件信息
    //MailBoxMgr::Instance().DebugCheck();
    m_wPubMailCount = stMailBoxInfo.m_wPublicCount;
    assert(m_wPubMailCount <= MAX_MAIL_BOX_PUBLIC_NUM);
    memcpy(m_astPubMailData, stMailBoxInfo.m_astPublicMailList, sizeof(DT_MAIL_PUB_PLAYER_DATA)*m_wPubMailCount);
    //MailBoxMgr::Instance().DebugCheck();

    return ERR_NONE;
}

int MailBoxInfo::ChgMailState(uint8_t bMailType, uint32_t dwId, uint8_t bNewState)
{
    if (MAIL_TYPE_PUBLIC_SYSTEM == bMailType)
    {
        return this->_ChgPubMailState(dwId, bNewState);
    }
    else
    {
        return this->_ChgPriMailState(dwId, bNewState);
    }

    MailBoxMgr::Instance().AddToDirtyList(this);
}

int MailBoxInfo::AddPriMail(DT_MAIL_DATA& rstData)
{
    if (m_wPriMailCount >= MAX_MAIL_BOX_PRIVATE_NUM)
    {
        return ERR_SYS;
    }

    DT_MAIL_DATA* pstMailData = MailSvrPool::Instance().PriMailDataPool().Get();
    if (!pstMailData)
    {
        LOGERR("Uin(%lu) get mem from pool failed", m_stBaseInfo.m_ullUin);
        return ERR_SYS;
    }

    rstData.m_dwId = ++m_stBaseInfo.m_dwPriSeq;
    if (rstData.m_ullTimeStampMs == 0)
    {
        rstData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();
    }
    *pstMailData = rstData;

    m_oPriMailMap.insert(MapId2Mail_t::value_type(pstMailData->m_dwId, pstMailData));
    m_wPriMailCount++;

    LOGRUN("Uin(%lu) AddPriMail(%u)", m_stBaseInfo.m_ullUin, rstData.m_dwId);

    //将添加邮件消息发给客户端
    this->_SendOneMail(rstData);

    MailBoxMgr::Instance().AddToDirtyList(this);

    return ERR_NONE;
}

int MailBoxInfo::AddPriMail(uint32_t dwResId)
{
    RESPRIMAIL* pstPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(dwResId);
    if (!pstPriMail)
    {
        LOGERR("Uin(%lu) AddPriMail failed, MailResId(%u) not found", m_stBaseInfo.m_ullUin, dwResId);
        return ERR_NOT_FOUND;
    }

    DT_MAIL_DATA stMailData;

    stMailData.m_ullFromUin = 0;
    stMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
    stMailData.m_bState = MAIL_STATE_UNOPENED;
    StrCpy(stMailData.m_szTitle, pstPriMail->m_szTitle, RES_MAX_MAIL_TITLE_LEN);
    StrCpy(stMailData.m_szContent, pstPriMail->m_szContent, RES_MAX_MAIL_CONTENT_LEN);
    stMailData.m_bAttachmentCount = pstPriMail->m_bAttachmentNum > RES_MAX_MAIL_REWARD_NUM ? \
                                 RES_MAX_MAIL_REWARD_NUM : pstPriMail->m_bAttachmentNum;
    for (int i=0; i<stMailData.m_bAttachmentCount; i++)
    {
        stMailData.m_astAttachmentList[i].m_bItemType = pstPriMail->m_astAttachmentList[i].m_bType;
        stMailData.m_astAttachmentList[i].m_dwItemId = pstPriMail->m_astAttachmentList[i].m_dwId;
        stMailData.m_astAttachmentList[i].m_iValueChg = pstPriMail->m_astAttachmentList[i].m_dwNum;
    }
    stMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();

    return this->AddPriMail(stMailData);
}

int MailBoxInfo::UpdatePubMail(bool bSendFlag)
{
    DT_MAIL_PUB_SER_DATA* pstMailPubData = NULL;

    SSPKG& rstSsPkg = MailSvrMsgLayer::Instance().GetSsPkg();
    rstSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_SYNC_PLAYER_DATA_NTF;
    SS_PKG_MAIL_SYNC_PLAYER_DATA_NTF& rstSsSyncMailNtf = rstSsPkg.m_stBody.m_stMailSyncPlayerDataNtf;
    rstSsSyncMailNtf.m_ullUin = m_stBaseInfo.m_ullUin;
    rstSsSyncMailNtf.m_wCount = 0;

    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();

    //当前公共邮件的Seq小于系统公共邮件的Seq,需要更新
    if (m_stBaseInfo.m_dwPubSeq < MailPubServData::Instance().GetPubMailSeq())
    {
        int iPos = (m_stBaseInfo.m_dwPubSeq==0) ? 0 : (MailPubServData::Instance().GetPosById(m_stBaseInfo.m_dwPubSeq));
        for ( ;iPos<MailPubServData::Instance().GetPubMailCount() && m_wPubMailCount < MAX_MAIL_BOX_PUBLIC_NUM; iPos++)
        {
            pstMailPubData = MailPubServData::Instance().GetPubSerDataByPos(iPos);
            if ((pstMailPubData->m_bState) && (pstMailPubData->m_ullEndTimeSec >= ullCurTime)
                && (m_stBaseInfo.m_dwPubSeq < pstMailPubData->m_dwId))
            {
                m_astPubMailData[m_wPubMailCount].m_bState = MAIL_STATE_UNOPENED;
                m_astPubMailData[m_wPubMailCount++].m_dwMailId = pstMailPubData->m_dwId;
                if (bSendFlag)
                {
                    this->_GenMailData(*pstMailPubData, rstSsSyncMailNtf.m_astMailList[rstSsSyncMailNtf.m_wCount++], MAIL_STATE_UNOPENED);
                }
            }
        }
    }

    if (pstMailPubData)
    {
        m_stBaseInfo.m_dwPubSeq = pstMailPubData->m_dwId;
    }

    if (bSendFlag)
    {
        rstSsSyncMailNtf.m_dwPubSeq = m_stBaseInfo.m_dwPubSeq;
        MailSvrMsgLayer::Instance().SendToZoneSvr(rstSsPkg);
    }

    MailBoxMgr::Instance().AddToDirtyList(this);

    return ERR_NONE;
}

int MailBoxInfo::SendMailBoxInfoToClient()
{
    SSPKG& rstSsPkg = MailSvrMsgLayer::Instance().GetSsPkg();
    rstSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_SYNC_PLAYER_DATA_NTF;
    SS_PKG_MAIL_SYNC_PLAYER_DATA_NTF& rstMailDataNtf = rstSsPkg.m_stBody.m_stMailSyncPlayerDataNtf;

    rstMailDataNtf.m_ullUin = m_stBaseInfo.m_ullUin;
    rstMailDataNtf.m_wCount = 0;
    rstMailDataNtf.m_dwPubSeq = m_stBaseInfo.m_dwPubSeq;

    //公共邮件
    for (int i=0; i<m_wPubMailCount; i++)
    {
        DT_MAIL_PUB_SER_DATA* pstMailPubData = MailPubServData::Instance().GetPubSerDataById(m_astPubMailData[i].m_dwMailId);
        if (!pstMailPubData)
        {
            LOGERR("Uin(%lu) PubMail(%u) not found", m_stBaseInfo.m_ullUin, m_astPubMailData[i].m_dwMailId);
            continue;
        }

        this->_GenMailData(*pstMailPubData, rstMailDataNtf.m_astMailList[rstMailDataNtf.m_wCount++], m_astPubMailData[i].m_bState);
    }

    //私有邮件
    m_oMailMapIter = m_oPriMailMap.begin();
    for (int i=0; m_oMailMapIter != m_oPriMailMap.end() && i<m_wPriMailCount; i++, m_oMailMapIter++)
    {
        DT_MAIL_DATA* pstMailData = m_oMailMapIter->second;
        rstMailDataNtf.m_astMailList[rstMailDataNtf.m_wCount++] = *pstMailData;
    }

    if (rstMailDataNtf.m_astMailList > 0)
    {
        MailSvrMsgLayer::Instance().SendToZoneSvr(rstSsPkg);
    }

    return ERR_NONE;
}

int MailBoxInfo::_SendOneMail(DT_MAIL_DATA& rstData)
{
    SSPKG& rstSsPkg = MailSvrMsgLayer::Instance().GetSsPkg();
    rstSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_SYNC_PLAYER_DATA_NTF;
    SS_PKG_MAIL_SYNC_PLAYER_DATA_NTF& rstMailDataNtf = rstSsPkg.m_stBody.m_stMailSyncPlayerDataNtf;

    rstMailDataNtf.m_ullUin = m_stBaseInfo.m_ullUin;
    rstMailDataNtf.m_wCount = 1;
    rstMailDataNtf.m_dwPubSeq = m_stBaseInfo.m_dwPubSeq;

    rstMailDataNtf.m_astMailList[0] = rstData;

    MailSvrMsgLayer::Instance().SendToZoneSvr(rstSsPkg);

    return ERR_NONE;
}

void MailBoxInfo::_GenMailData(DT_MAIL_PUB_SER_DATA& rstPubMailData, DT_MAIL_DATA& rstMailData, uint8_t bState)
{
    rstMailData.m_dwId = rstPubMailData.m_dwId;
    rstMailData.m_bType = MAIL_TYPE_PUBLIC_SYSTEM;
    rstMailData.m_bState = bState;
    rstMailData.m_bAttachmentCount = rstPubMailData.m_bAttachmentCount;
    for (int i=0; i<rstMailData.m_bAttachmentCount; i++)
    {
        rstMailData.m_astAttachmentList[i].m_dwItemId = rstPubMailData.m_astAttachmentList[i].m_dwId;
        rstMailData.m_astAttachmentList[i].m_bItemType = rstPubMailData.m_astAttachmentList[i].m_bType;
        rstMailData.m_astAttachmentList[i].m_iValueChg = rstPubMailData.m_astAttachmentList[i].m_dwNum;
    }
    rstMailData.m_ullFromUin = 0;
    rstMailData.m_ullTimeStampMs = rstPubMailData.m_ullStartTimeSec;

    StrCpy(rstMailData.m_szTitle, rstPubMailData.m_szTitle, MAX_MAIL_TITLE_LEN);
    StrCpy(rstMailData.m_szContent, rstPubMailData.m_szContent, MAX_MAIL_CONTENT_LEN);
}

int MailBoxInfo::_ChgPriMailState(uint32_t dwId, uint8_t bNewState)
{
    m_oMailMapIter = m_oPriMailMap.find(dwId);
    if (m_oMailMapIter == m_oPriMailMap.end())
    {
        LOGERR("Uin(%lu) not found PriMail(%u)", m_stBaseInfo.m_ullUin, dwId);
        return ERR_NOT_FOUND;
    }
    DT_MAIL_DATA* pstMailData = m_oMailMapIter->second;

    //检查状态变化的合理性
    int iRet = this->_CheckPriMailState(pstMailData, bNewState);
    if (iRet != ERR_NONE)
    {
        return iRet;
    }
    pstMailData->m_bState = bNewState;

    //如果新状态为DRAWED,需要发状态改变通知
    if (bNewState == MAIL_STATE_DRAWED)
    {
        SSPKG& rstSsPkg = MailSvrMsgLayer::Instance().GetSsPkg();
        rstSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_STAT_NTF;
        rstSsPkg.m_stHead.m_ullUin = m_stBaseInfo.m_ullUin;

        SS_PKG_MAIL_STAT_NTF& rstSsMailStateNtf = rstSsPkg.m_stBody.m_stMailStatNtf;
        rstSsMailStateNtf.m_ullUin = m_stBaseInfo.m_ullUin;
        rstSsMailStateNtf.m_nErrNo = ERR_NONE;
        rstSsMailStateNtf.m_bNewStat = MAIL_STATE_DRAWED;
        rstSsMailStateNtf.m_stMailData = *pstMailData;

        MailSvrMsgLayer::Instance().SendToZoneSvr(rstSsPkg);
    }

    //如果新状态为DELETE,则删除邮件
    if (bNewState == MAIL_STATE_DELETED)
    {
        this->_DelPriMail(dwId);
    }

    return ERR_NONE;
}

int MailBoxInfo::_CheckPriMailState(DT_MAIL_DATA* pstPriMailData, uint8_t bNewState)
{
    if (pstPriMailData->m_bState == (bNewState -1) ||
       (pstPriMailData->m_bAttachmentCount == 0 && bNewState == MAIL_STATE_DELETED))
    {
        return ERR_NONE;
    }

    LOGERR("Uin(%lu) chg PriMail(%u) from stat(%u) to (%u) failed",
             m_stBaseInfo.m_ullUin, pstPriMailData->m_dwId, pstPriMailData->m_bState, bNewState);

    return ERR_NOT_SATISFY_COND;
}

int MailBoxInfo::_ChgPubMailState(uint32_t dwId, uint8_t bNewState)
{
    DT_MAIL_PUB_PLAYER_DATA stPubMailData;
    stPubMailData.m_dwMailId = dwId;

    int iEqual = 0;
    int iIndex = MyBSearch(&stPubMailData, m_astPubMailData, m_wPubMailCount, sizeof(DT_MAIL_PUB_PLAYER_DATA), &iEqual, PubMailDataCmp);
    if (!iEqual)
    {
        LOGERR("Uin(%lu) PubMail(%u) not found", m_stBaseInfo.m_ullUin, dwId);
        return ERR_NOT_FOUND;
    }

    //检查状态变化的合理性
    int iRet = this->_CheckPubMailState(&m_astPubMailData[iIndex], bNewState);
    if (iRet != ERR_NONE)
    {
        return iRet;
    }
    m_astPubMailData[iIndex].m_bState = bNewState;

    //如果新状态为DRAWED,需要发状态改变通知
    if (bNewState == MAIL_STATE_DRAWED)
    {
        SSPKG& rstSsPkg = MailSvrMsgLayer::Instance().GetSsPkg();
        rstSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_STAT_NTF;
        rstSsPkg.m_stHead.m_ullUin = m_stBaseInfo.m_ullUin;

        SS_PKG_MAIL_STAT_NTF& rstSsMailStateNtf = rstSsPkg.m_stBody.m_stMailStatNtf;
        rstSsMailStateNtf.m_ullUin = m_stBaseInfo.m_ullUin;
        rstSsMailStateNtf.m_nErrNo = ERR_NONE;
        rstSsMailStateNtf.m_bNewStat = MAIL_STATE_DRAWED;
        rstSsMailStateNtf.m_stMailData.m_dwId = dwId;
        rstSsMailStateNtf.m_stMailData.m_bType = MAIL_TYPE_PUBLIC_SYSTEM;

        //获取公共邮件附件的具体数据
        DT_MAIL_PUB_SER_DATA* pstMailPubData = MailPubServData::Instance().GetPubSerDataById(dwId);
        if (!pstMailPubData)
        {
            LOGERR("Uin(%lu) PubMail(%u) not found in PubMailSvrData", m_stBaseInfo.m_ullUin, dwId);
            return ERR_NOT_FOUND;;
        }
        this->_GenMailData(*pstMailPubData, rstSsMailStateNtf.m_stMailData, MAIL_STATE_DRAWED);

        MailSvrMsgLayer::Instance().SendToZoneSvr(rstSsPkg);
    }

    //如果新状态为DELETE,则删除邮件
    if (bNewState == MAIL_STATE_DELETED)
    {
        this->_DelPubMail(dwId);
    }

    return ERR_NONE;
}

int MailBoxInfo::_CheckPubMailState(DT_MAIL_PUB_PLAYER_DATA* pstPubMailData, uint8_t bNewState)
{
    if (pstPubMailData->m_bState == (bNewState -1))
    {
        return ERR_NONE;
    }

    if (bNewState == MAIL_STATE_DELETED)
    {
        DT_MAIL_PUB_SER_DATA* pstPubSerData = MailPubServData::Instance().GetPubSerDataById(pstPubMailData->m_dwMailId);
        if (!pstPubSerData)
        {
            LOGERR("Uin(%lu) chg PubMail(%u) to MAIL_STATE_DELETED failed, PubMail not found",
                    m_stBaseInfo.m_ullUin, pstPubMailData->m_dwMailId);
            return ERR_NOT_FOUND;
        }

        if (pstPubSerData->m_bAttachmentCount == 0)
        {
            return ERR_NONE;
        }
    }

    LOGERR("Uin(%lu) chg PubMail(%u) from stat (%u) to (%u) failed",
             m_stBaseInfo.m_ullUin, pstPubMailData->m_dwMailId, pstPubMailData->m_bState, bNewState);

    return ERR_NOT_SATISFY_COND;
}

int MailBoxInfo::_DelPriMail(uint32_t dwId)
{
    m_oMailMapIter = m_oPriMailMap.find(dwId);
    if (m_oMailMapIter != m_oPriMailMap.end())
    {
        MailSvrPool::Instance().PriMailDataPool().Release(m_oMailMapIter->second);
        m_oPriMailMap.erase(m_oMailMapIter);
        m_wPriMailCount--;
        return ERR_NONE;
    }
    else
    {
        LOGERR("Uin(%lu) delete PriMail(%u) failed, mail not found", m_stBaseInfo.m_ullUin, dwId);
        return ERR_NOT_FOUND;
    }
}

int MailBoxInfo::_DelPubMail(uint32_t dwId)
{
    DT_MAIL_PUB_PLAYER_DATA stPubMailData;
    stPubMailData.m_dwMailId = dwId;

    size_t nmemb = (size_t)m_wPubMailCount;
    if (!MyBDelete(&stPubMailData, m_astPubMailData, &nmemb, sizeof(DT_MAIL_PUB_PLAYER_DATA), PubMailDataCmp))
    {
        LOGERR("Uin(%lu) delete PubMail(%u) failed, mail not found", m_stBaseInfo.m_ullUin, dwId);
        return ERR_NOT_FOUND;
    }
    m_wPubMailCount = (uint16_t)nmemb;

    return ERR_NONE;
}

