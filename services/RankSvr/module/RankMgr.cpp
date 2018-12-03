#include "RankMgr.h"
#include "../framework/RankSvrMsgLayer.h"
#include "strutil.h"
#include "PKGMETA_metalib.h"
#include "../../common/sys/GameTime.h"

using namespace PKGMETA;

bool RankMgr::Init()
{
    int iMaxNum = MAX_RANK_TOP_NUM;

    if (!m_oRank6V6.Init("Rank6v6Table", iMaxNum, PKGMETA::MetaLib::getVersion()))
    {
        LOGERR("Init m_oRank6V6 error!");
        return false;
    }
    else if (!m_oRankLi.Init("RankLiTable", iMaxNum, PKGMETA::MetaLib::getVersion()))
    {
        LOGERR("Init m_oRankLi error!");
        return false;
    }
    else if (!m_oRankGCardLi.Init("RankGCardLiTable", iMaxNum, PKGMETA::MetaLib::getVersion()))
    {
        LOGERR("Init m_oRankGCardLi error!");
        return false;
    }
    else if (!m_oRankPveStar.Init("RankPveStarTable", iMaxNum, PKGMETA::MetaLib::getVersion()))
    {
        LOGERR("Init m_oRankPveStar error!");
        return false;
    }
    else if (!m_oRankGCardCnt.Init("RankGCardCntTable", iMaxNum, PKGMETA::MetaLib::getVersion()))
    {
        LOGERR("Init m_oRankGCardCnt error!");
        return false;
    }
    else if (!m_oRankDailyChallenge.Init("RankDailyChallengeTable", iMaxNum, PKGMETA::MetaLib::getVersion()))
    {
        LOGERR("Init m_oRankDailyChallenge error!");
        return false;
    }
	else if (!m_oRankGuild.Init("RankGuildTable", iMaxNum, PKGMETA::MetaLib::getVersion()))
	{
		LOGERR("Init m_oRankGuild error!");
		return false;
	}
	else if (!m_oRankGFight.Init("RankGFightTable", iMaxNum, PKGMETA::MetaLib::getVersion()))
	{
		LOGERR("Init m_oRankGFight error!");
		return false;
	}
    else if (!m_oRankPeakArena.Init("RankPeakArena", iMaxNum, PKGMETA::MetaLib::getVersion()))
    {
        LOGERR("Init m_oRankPeakArena error!");
		return false;
    }

    return true;
}


void RankMgr::OnExit()
{
    LOGRUN("####RankMgr::OnExit start!");
    m_oRank6V6.OnExit();
    m_oRankLi.OnExit();
    m_oRankGCardLi.OnExit();
    m_oRankPveStar.OnExit();
    m_oRankGCardCnt.OnExit();
    m_oRankDailyChallenge.OnExit();
	m_oRankGuild.OnExit();
	m_oRankGFight.OnExit();
    m_oRankPeakArena.OnExit();
    LOGRUN("####RankMgr::OnExit end!");
}


void RankMgr::BackUpRankList(const char* szFileName, vector<uint64_t>& UinList)
{
    char szBackUpFile[MAX_NAME_LENGTH];
    snprintf(szBackUpFile, MAX_NAME_LENGTH, "%s.bak", szFileName);
    FILE* fp = fopen(szFileName, "wb+");
    if (!fp)
    {
        return;
    }

    int iSize = UinList.size();

    fseek(fp, 0, SEEK_SET);

    if (fwrite(&iSize, sizeof(int), 1, fp) != 1)
    {
       LOGERR_r("Save size to file %s failed", szFileName);
       return;
    }

    for (int i = 0; i < iSize; i++)
    {
        if (fwrite(&UinList[i], sizeof(uint64_t), 1, fp) != 1)
        {
           LOGERR_r("Save ranklist[%d] to file %s failed", i, szFileName);
           return;
        }
    }

    fflush(fp);
    fclose(fp);

    return;
}


int RankMgr::ReadRankFromBackUp(const char* szFileName, vector<uint64_t>& UinList)
{
    char szBackUpFile[MAX_NAME_LENGTH];
    snprintf(szBackUpFile, MAX_NAME_LENGTH, "%s.bak", szFileName);
    FILE* fp = fopen(szFileName, "rb+");
    if (!fp)
    {
        return -1;
    }

    fseek(fp, 0, SEEK_SET);

    int iSize = 0;
    if (fread(&iSize, sizeof(int), 1, fp) != 1)
    {
        LOGERR_r("Read size from %s failed", szFileName);
        return -1;
    }

    for (int i = 0; i < iSize; i++)
    {
        uint64_t dwUin = 0;
        if (fread(&dwUin, sizeof(uint64_t), 1, fp) != 1)
        {
            LOGERR_r("Read ranklist from %s failed", szFileName);
            return  -1;
        }
        UinList[i] = dwUin;
    }

    fclose(fp);

    return 0;
}

void RankMgr::CRankDailyChallenge::SettleRank(bool bUseBak)
{
    LOGRUN("####RankMgr::CRankDailyChallenge::SettleRank start!");

    vector<uint64_t> UinList;
    if (!bUseBak)
    {
        int iRet = m_RankSys.GetTopKey(m_RankSys.Size(), UinList);
        assert(m_RankSys.Size() == iRet);
        RankMgr::Instance().BackUpRankList(m_szRankFileName, UinList);
        this->_SettleRank(UinList);
    }
    else
    {
        int iRet = RankMgr::Instance().ReadRankFromBackUp(m_szRankFileName, UinList);
        if (iRet != 0)
        {
            LOGERR("ReadRankFromBackUp failed");
            return;
        }
        this->_SettleRank(UinList);
    }

    LOGRUN("####RankMgr::CRankDailyChallenge::SettleRank end!");
}


void RankMgr::CRankDailyChallenge::_SettleRank(vector<uint64_t>& UinList)
{
    ResDailyChallengeRankRewardMgr_t& rstRankRewardMgr = CGameDataMgr::Instance().GetResDailyChallengeRankRewardMgr();
    RESDAILYCHALLENGERANKREWARD* pResReward = rstRankRewardMgr.GetResByPos(rstRankRewardMgr.GetResNum() -1);
    uint32_t dwStartRank = 1;
    uint32_t dwEndRank = MIN(pResReward->m_dwNoLower, UinList.size());

    RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(DAILY_RANK_SETTLE_MAIL_ID);
    RESPRIMAIL* poResPriMail1 = CGameDataMgr::Instance().GetResPriMailMgr().Find(DAILY_RANK_SETTLE_MAIL1_ID);
    if (NULL == poResPriMail || poResPriMail1 == NULL)
    {
        LOGERR("Can't find the mail<id %u>, mail<id %u>", DAILY_RANK_SETTLE_MAIL_ID, DAILY_RANK_SETTLE_MAIL1_ID);
        return;
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    SS_PKG_MAIL_ADD_REQ& rstMailAddReq = m_stSsPkg.m_stBody.m_stMailAddReq;
    rstMailAddReq.m_nUinCount = 0;
    DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
    rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
    rstMailData.m_bState = MAIL_STATE_UNOPENED;
    StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
    rstMailData.m_ullFromUin = 0;
    rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();

    int iResPos = 0;
    pResReward = rstRankRewardMgr.GetResByPos(iResPos);
    rstMailData.m_bAttachmentCount = pResReward->m_bRewardCount;
    for (int i = 0; i < rstMailData.m_bAttachmentCount; i++)
    {
        rstMailData.m_astAttachmentList[i].m_bItemType = pResReward->m_szRewardTypeList[i];
        rstMailData.m_astAttachmentList[i].m_dwItemId = pResReward->m_rewardIdList[i];
        rstMailData.m_astAttachmentList[i].m_iValueChg = pResReward->m_rewardCountList[i];
    }
    snprintf(rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN, poResPriMail->m_szContent, pResReward->m_dwNoLower);

    for (; dwStartRank <= dwEndRank; dwStartRank++)
    {
        //当前排名大于本条数据档最大排名
        if (dwStartRank > pResReward->m_dwNoLower)
        {
            if (rstMailAddReq.m_nUinCount > 0)
            {
                RankSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
                rstMailAddReq.m_nUinCount = 0;
            }

            pResReward = rstRankRewardMgr.GetResByPos(++iResPos);
            rstMailData.m_bAttachmentCount = pResReward->m_bRewardCount;
            for (int i = 0; i < rstMailData.m_bAttachmentCount; i++)
            {
                rstMailData.m_astAttachmentList[i].m_bItemType = pResReward->m_szRewardTypeList[i];
                rstMailData.m_astAttachmentList[i].m_dwItemId = pResReward->m_rewardIdList[i];
                rstMailData.m_astAttachmentList[i].m_iValueChg = pResReward->m_rewardCountList[i];
            }
            if (pResReward->m_dwNoLower <= 10)
            {
                snprintf(rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN, poResPriMail->m_szContent, pResReward->m_dwNoLower);
            }
            else
            {
                snprintf(rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN, poResPriMail1->m_szContent, pResReward->m_dwNoLower);
            }
        }

        //邮件的发送人数已达上限，发送邮件
        rstMailAddReq.m_UinList[rstMailAddReq.m_nUinCount++] = UinList[dwStartRank -1];
        if (rstMailAddReq.m_nUinCount >= MAX_MAIL_MULTI_USER_NUM)
        {
            RankSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
            rstMailAddReq.m_nUinCount = 0;
        }
    }

    if (rstMailAddReq.m_nUinCount >= 0)
    {
        RankSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
    }
}


void RankMgr::CRankLi::SettleRank(bool bUseBak)
{
    LOGRUN("####RankMgr::CRankLi::SettleRank start!");

    vector<uint64_t> UinList;
    if (!bUseBak)
    {
        int iRet = m_RankSys.GetTopKey(m_RankSys.Size(), UinList);
        assert(m_RankSys.Size() == iRet);
        RankMgr::Instance().BackUpRankList(m_szRankFileName, UinList);
        this->_SettleRank(UinList);
    }
    else
    {
        int iRet = RankMgr::Instance().ReadRankFromBackUp(m_szRankFileName, UinList);
        if (iRet != 0)
        {
            LOGERR("ReadRankFromBackUp failed");
            return;
        }
        this->_SettleRank(UinList);
    }

    LOGRUN("####RankMgr::CRankLi::SettleRank end!");
}


void RankMgr::CRankLi::_SettleRank(vector<uint64_t>& UinList)
{
    int iNum = CGameDataMgr::Instance().GetResLiRankRewardMgr().GetResNum();
    vector<RankRoleNode> PlayerVector;
    int iRet = m_RankSys.GetTopObj(iNum, PlayerVector);
    if (iRet > iNum)
    {
        LOGERR("Get TopObj failed");
        return;
    }

    RESBASIC* poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(8500);
    assert(poResBasic);

    RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(LI_RANK_SETTLE_MAIL_ID);
    assert(poResPriMail);

    RESPRIMAIL* poResConsolationMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(LI_RANK_CONSOLATION_MAIL_ID);
    assert(poResConsolationMail);

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    SS_PKG_MAIL_ADD_REQ& rstMailAddReq = m_stSsPkg.m_stBody.m_stMailAddReq;
    rstMailAddReq.m_nUinCount = 1;
    DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
    rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
    rstMailData.m_bState = MAIL_STATE_UNOPENED;
    rstMailData.m_ullFromUin = 0;
    rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();

    RESACTIVITYFORCERANKREWARD* pResReward = NULL;
    for (int i = 0; i < iRet; i++)
    {
        pResReward = CGameDataMgr::Instance().GetResLiRankRewardMgr().GetResByPos(i);
        assert(pResReward);

        //进入排名且战力达标
        if (PlayerVector[i].m_stScoreRankInfo.m_dwValue >= (uint32_t)poResBasic->m_para[0])
        {
            StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
            snprintf(rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN, poResPriMail->m_szContent, i+1);
            rstMailData.m_bAttachmentCount = pResReward->m_bRewardCount;
            for (int j = 0; j < rstMailData.m_bAttachmentCount; j++)
            {
                rstMailData.m_astAttachmentList[j].m_bItemType = pResReward->m_szRewardTypeList[j];
                rstMailData.m_astAttachmentList[j].m_dwItemId = pResReward->m_rewardIdList[j];
                rstMailData.m_astAttachmentList[j].m_iValueChg = pResReward->m_rewardNumList[j];
            }
        }
        //虽然进入排名，但战力不达标
        else
        {
            StrCpy(rstMailData.m_szTitle, poResConsolationMail->m_szTitle, MAX_MAIL_TITLE_LEN);
            snprintf(rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN, poResConsolationMail->m_szContent, i+1);
            rstMailData.m_bAttachmentCount = 1;
            rstMailData.m_astAttachmentList[0].m_bItemType = pResReward->m_szRewardTypeList[pResReward->m_bRewardCount-1];
            rstMailData.m_astAttachmentList[0].m_dwItemId = pResReward->m_rewardIdList[pResReward->m_bRewardCount-1];
            rstMailData.m_astAttachmentList[0].m_iValueChg = pResReward->m_rewardNumList[pResReward->m_bRewardCount-1];
        }

        rstMailAddReq.m_UinList[0] = PlayerVector[i].m_stScoreRankInfo.m_ullUin;
        RankSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MESSAGE_AUTO_SEND_NTF;
    SS_PKG_MESSAGE_AUTO_SEND_NTF& rstNtf = m_stSsPkg.m_stBody.m_stMessageAutoSendNtf;
    rstNtf.m_bCount = 0;
    rstNtf.m_astRecords[rstNtf.m_bCount].m_ullSenderUin = PlayerVector[0].m_stScoreRankInfo.m_ullUin;
    snprintf(rstNtf.m_astRecords[rstNtf.m_bCount].m_szRecord, MAX_MESSAGE_RECORD_LEN, "Name=%s", PlayerVector[0].m_stScoreRankInfo.m_szRoleName);
    rstNtf.m_astRecords[rstNtf.m_bCount].m_dwRecordType = 1901;
    rstNtf.m_astRecords[rstNtf.m_bCount].m_bChannel = MESSAGE_CHANNEL_SYS;
    rstNtf.m_bCount++;

    rstNtf.m_astRecords[rstNtf.m_bCount].m_ullSenderUin = PlayerVector[0].m_stScoreRankInfo.m_ullUin;
    rstNtf.m_astRecords[rstNtf.m_bCount].m_szRecord[0] = '\0';
    rstNtf.m_astRecords[rstNtf.m_bCount].m_dwRecordType = 1902;
    rstNtf.m_astRecords[rstNtf.m_bCount].m_bChannel = MESSAGE_CHANNEL_WORLD;
    rstNtf.m_bCount++;
    RankSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);

}

void RankMgr::CRankPeakArena::SettleRank(bool bUseBak)
{
    LOGRUN("####RankMgr::CRankPeakArena::SettleRank start!");

    vector<uint64_t> UinList;
    if (!bUseBak)
    {
        int iRet = m_RankSys.GetTopKey(m_RankSys.Size(), UinList);
        assert(m_RankSys.Size() == iRet);
        RankMgr::Instance().BackUpRankList(m_szRankFileName, UinList);
        this->_SettleRank(UinList);
    }
    else
    {
        int iRet = RankMgr::Instance().ReadRankFromBackUp(m_szRankFileName, UinList);
        if (iRet != 0)
        {
            LOGERR("ReadRankFromBackUp failed");
            return;
        }
        this->_SettleRank(UinList);
    }

    LOGRUN("####RankMgr::CRankPeakArena::SettleRank end!");
}

void RankMgr::CRankPeakArena::_SettleRank(vector<uint64_t>& UinList)
{
    ResPeakArenaRankRewardMgr_t& rstRankRewardMgr = CGameDataMgr::Instance().GetResPeakArenaRankRewardMgr();
    RESPEAKARENARANKREWARD* pResReward = rstRankRewardMgr.GetResByPos(rstRankRewardMgr.GetResNum() -1);
    uint32_t dwStartRank = 1;
    uint32_t dwEndRank = MIN(pResReward->m_dwRank, UinList.size());
    int iResPos = 0;

    RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(PEAK_ARENA_SETTLE_MAIL_ID);
    if (NULL == poResPriMail)
    {
        LOGERR("Can't find mail<id %u>", PEAK_ARENA_SETTLE_MAIL_ID);
        return;
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    SS_PKG_MAIL_ADD_REQ& rstMailAddReq = m_stSsPkg.m_stBody.m_stMailAddReq;
    rstMailAddReq.m_nUinCount = 0;
    DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
    rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
    rstMailData.m_bState = MAIL_STATE_UNOPENED;
    StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
    rstMailData.m_ullFromUin = 0;
    rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();

    //初始档位
    pResReward = rstRankRewardMgr.GetResByPos(iResPos);
    rstMailData.m_bAttachmentCount = pResReward->m_bCount;
    for (int i = 0; i < rstMailData.m_bAttachmentCount; i++)
    {
        rstMailData.m_astAttachmentList[i].m_bItemType = pResReward->m_szPropsType[i];
        rstMailData.m_astAttachmentList[i].m_dwItemId = pResReward->m_propsId[i];
        rstMailData.m_astAttachmentList[i].m_iValueChg = pResReward->m_propsNum[i];
    }
    snprintf(rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN, poResPriMail->m_szContent, pResReward->m_dwRank);

    for (; dwStartRank <= dwEndRank; dwStartRank++)
    {
        //当前排名大于本条数据档最大排名
        if (dwStartRank > pResReward->m_dwRank)
        {
            if (rstMailAddReq.m_nUinCount > 0)
            {
                RankSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
                rstMailAddReq.m_nUinCount = 0;
            }

            pResReward = rstRankRewardMgr.GetResByPos(++iResPos);
            if (pResReward == NULL)
            {
                LOGERR_r("the pResReward<pos=%d> is NULL", iResPos);
                continue;
            }
            rstMailData.m_bAttachmentCount = pResReward->m_bCount;
            for (int i = 0; i < rstMailData.m_bAttachmentCount; i++)
            {
                rstMailData.m_astAttachmentList[i].m_bItemType = pResReward->m_szPropsType[i];
                rstMailData.m_astAttachmentList[i].m_dwItemId = pResReward->m_propsId[i];
                rstMailData.m_astAttachmentList[i].m_iValueChg = pResReward->m_propsNum[i];
            }
            snprintf(rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN, poResPriMail->m_szContent, pResReward->m_dwRank);
        }

        //邮件的发送人数已达上限，发送邮件
        rstMailAddReq.m_UinList[rstMailAddReq.m_nUinCount++] = UinList[dwStartRank -1];

        if (rstMailAddReq.m_nUinCount >= MAX_MAIL_MULTI_USER_NUM)
        {
            RankSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
            rstMailAddReq.m_nUinCount = 0;
        }
    }

    if (rstMailAddReq.m_nUinCount >= 0)
    {
        RankSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
    }
}

