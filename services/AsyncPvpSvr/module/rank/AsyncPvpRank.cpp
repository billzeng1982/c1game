#include "AsyncPvpRank.h"
#include "LogMacros.h"
#include "AsyncPvpSvr.h"
#include "AsyncPvpPlayerMgr.h"
#include "GameDataMgr.h"
#include "AsyncPvpTransaction.h"
#include "AsyncPvpTransFrame.h"
#include "GameObjectPool.h"
#include "AsyncPvpSvrMsgLayer.h"
#include "../../common/sys/GameTime.h"

using namespace PKGMETA;
using namespace std;

bool AsyncPvpRank::Init()
{
    ASYNCPVPSVRCFG& rstCfg = AsyncPvpSvr::Instance().GetConfig();
    m_fp = fopen(rstCfg.m_szRankFile, "rb+");
    if (!m_fp)
    {
        LOGERR_r("AsyncPvpRank init failed,open file(%s) failed", rstCfg.m_szRankFile);
        return false;
    }

    if (fread(&m_iCount, sizeof(int), 1, m_fp) != 1)
	{
		LOGERR_r("AsyncPvpRank init failed, read count failed");
		return false;
	}

    m_iCapacity = m_iCount + 5000;
    m_Rank = new uint64_t[m_iCapacity];

    //从文件中读排名
    if (fread(m_Rank, sizeof(uint64_t), m_iCount, m_fp) != (size_t)m_iCount)
    {
        LOGERR_r("AsyncPvpRank init failed, read ranklist failed");
        return false;
    }

    //初始化Id2RankMap
    for (int i=0; i<m_iCount; i++)
    {
        m_oIter = m_oRankMap.find(m_Rank[i]);
        if (m_oIter != m_oRankMap.end())
        {
            LOGERR_r("AsyncPvpRank init failed, init RankMap failed");
            return false;
        }
        m_oRankMap.insert(Id2RankMap_t::value_type(m_Rank[i], i+1));
    }

    m_iDirtyNodeMax = rstCfg.m_iDirtyNodeMax;
    m_iWriteTimeVal = rstCfg.m_iWriteTimeVal;

    //创建初始化TopList的transaction
    m_bNtfFlag = false;

    GetTopListTransAction* poTrans = GET_GAMEOBJECT(GetTopListTransAction, GAMEOBJ_GET_TOPLIST_TRANS);
    assert(poTrans);

    CompositeAction* poCompositeAction = GET_GAMEOBJECT(CompositeAction, GAMEOBJ_COMPOSITE_ACTION);
    assert(poCompositeAction);
    for (int i=0; i<MAX_RANK_TOP_NUM; i++)
    {
        GetPlayerAction* poGetPlayerAction = GET_GAMEOBJECT(GetPlayerAction, GAMEOBJ_GET_PLAYER_ACTION);
        assert(poGetPlayerAction);
        poGetPlayerAction->SetPlayerId(m_Rank[i]);

        poCompositeAction->AddAction(poGetPlayerAction);
    }
    poTrans->AddAction(poCompositeAction);

    AsyncPvpTransFrame::Instance().ScheduleTransaction(poTrans);

    //结算时间
    RESBASIC* poResBasic= CGameDataMgr::Instance().GetResBasicMgr().Find(DAILY_RANK_SETTLE_TIME_ID);
    if (!poResBasic)
    {
        LOGERR_r("AsyncPvpRank init failed, ResBasic id(%d) not found", DAILY_RANK_SETTLE_TIME_ID);
        return false;
    }
    m_iSettleTime = (int)poResBasic->m_para[0];

    int iHour = CGameTime::Instance().GetCurrHour();
    m_bSettleFlag = (m_iSettleTime==iHour) ? true : false;

    return true;
}


void AsyncPvpRank::SendRankReward(bool bUseBak)
{
    //记录当时的排名
    if (!bUseBak)
    {
        _BackUpRankFile();
        _SendRankReward(m_iCount, m_Rank);
    }
    else
    {
        ASYNCPVPSVRCFG& rstCfg = AsyncPvpSvr::Instance().GetConfig();
        char szFileName[MAX_NAME_LENGTH];
        snprintf(szFileName, MAX_NAME_LENGTH, "%s.bak", rstCfg.m_szRankFile);
        FILE* fp = fopen(szFileName, "rb+");
        if (!fp)
        {
            LOGERR_r("Can't open file %s", szFileName);
            return;
        }

        fseek(m_fp, 0, SEEK_SET);

        int iSize = 0;
        if (fread(&iSize, sizeof(int), 1, m_fp) != 1)
    	{
    		LOGERR_r("Read size from %s failed", szFileName);
    		return;
    	}

        uint64_t* pUinList = new uint64_t[iSize];
        if (!pUinList)
        {
            LOGERR_r("pUinList is NULL");
    		return;
        }

        if (fread(pUinList, sizeof(uint64_t), iSize, m_fp) != (size_t)iSize)
        {
            LOGERR_r("Read ranklist from %s failed", szFileName);
            return;
        }

        fclose(fp);

        _SendRankReward(iSize, pUinList);

        delete[] pUinList;
    }
}


void AsyncPvpRank::_SendRankReward(int iSize, uint64_t* pUinList)
{
    ResRankRewardMgr_t& rstRankRewardMgr = CGameDataMgr::Instance().GetResRankRewardMgr();
    RESASYNCPVPRANKREWARD* pResReward = rstRankRewardMgr.GetResByPos(rstRankRewardMgr.GetResNum() -1);
    uint32_t dwStartRank = 1;
    uint32_t dwEndRank = MIN(pResReward->m_dwRank, iSize);
    //assert(false);

    RESPRIMAIL* poResPriMail1 = CGameDataMgr::Instance().GetResPriMailMgr().Find(DAILY_RANK_SETTLE_MAIL1_ID);
    RESPRIMAIL* poResPriMail2 = CGameDataMgr::Instance().GetResPriMailMgr().Find(DAILY_RANK_SETTLE_MAIL2_ID);
    if (NULL == poResPriMail1 || NULL == poResPriMail2)
    {
        LOGERR_r("Can't find the mail <id%u> <id%u>", DAILY_RANK_SETTLE_MAIL1_ID, DAILY_RANK_SETTLE_MAIL2_ID);
        return;
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    SS_PKG_MAIL_ADD_REQ& rstMailAddReq = m_stSsPkg.m_stBody.m_stMailAddReq;
    rstMailAddReq.m_nUinCount = 0;
    DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
    rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
    rstMailData.m_bState = MAIL_STATE_UNOPENED;
    StrCpy(rstMailData.m_szTitle, poResPriMail1->m_szTitle, MAX_MAIL_TITLE_LEN);
    rstMailData.m_ullFromUin = 0;
    rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();

    //初始档位
    int iResPos = 0;
    pResReward = rstRankRewardMgr.GetResByPos(iResPos);
    rstMailData.m_bAttachmentCount = pResReward->m_bCount;
    for (int i = 0; i < rstMailData.m_bAttachmentCount; i++)
    {
        rstMailData.m_astAttachmentList[i].m_bItemType = pResReward->m_szPropsType[i];
        rstMailData.m_astAttachmentList[i].m_dwItemId = pResReward->m_propsId[i];
        rstMailData.m_astAttachmentList[i].m_iValueChg = pResReward->m_propsNum[i];
    }
    snprintf(rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN, poResPriMail1->m_szContent, pResReward->m_dwRank);

    for (; dwStartRank <= dwEndRank; dwStartRank++)
    {
        //当前排名大于本条数据档最大排名
        if (dwStartRank > pResReward->m_dwRank)
        {
            if (rstMailAddReq.m_nUinCount > 0)
            {
                AsyncPvpSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
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

            if (pResReward->m_dwRank <= 10)
            {
                snprintf(rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN, poResPriMail1->m_szContent, pResReward->m_dwRank);
            }
            else
            {
                snprintf(rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN, poResPriMail2->m_szContent, pResReward->m_dwRank);
            }
        }

        //假玩家不发邮件
        if (pUinList[dwStartRank -1] < MAX_NUM_ASYNC_PVP_FAKE_PLAYER_UIN)
        {
            continue;
        }
        rstMailAddReq.m_UinList[rstMailAddReq.m_nUinCount++] = pUinList[dwStartRank -1];

        //邮件的发送人数已达上限，发送邮件
        if (rstMailAddReq.m_nUinCount >= MAX_MAIL_MULTI_USER_NUM)
        {
            AsyncPvpSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
            rstMailAddReq.m_nUinCount = 0;
        }
    }

    if (rstMailAddReq.m_nUinCount >= 0)
    {
        AsyncPvpSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MESSAGE_AUTO_SEND_NTF;
    SS_PKG_MESSAGE_AUTO_SEND_NTF& rstNtf = m_stSsPkg.m_stBody.m_stMessageAutoSendNtf;
    rstNtf.m_bCount = 0;
    rstNtf.m_astRecords[rstNtf.m_bCount].m_ullSenderUin = pUinList[0];
    snprintf(rstNtf.m_astRecords[rstNtf.m_bCount].m_szRecord, MAX_MESSAGE_RECORD_LEN, "Name=%s", m_astTopList[0].m_stBaseInfo.m_szName);
    rstNtf.m_astRecords[rstNtf.m_bCount].m_dwRecordType = 1008;
    rstNtf.m_astRecords[rstNtf.m_bCount].m_bChannel = MESSAGE_CHANNEL_SYS;
    rstNtf.m_bCount++;
    AsyncPvpSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);

}


void AsyncPvpRank::SendRefreshTopListNtf()
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_REFRESH_TOPLIST_NTF;
    SS_PKG_ASYNC_PVP_REFRESH_TOPLIST_NTF& rstNtf = m_stSsPkg.m_stBody.m_stAsyncpvpRefreshTopListNtf;

    for (int i=0; i<MAX_RANK_TOP_NUM; i++)
    {
        rstNtf.m_astTopList[i] = m_astTopList[i];
    }

    rstNtf.m_ullTimeStamp = CGameTime::Instance().GetCurrSecond();

    AsyncPvpSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);

    m_bNtfFlag = false;
}


void AsyncPvpRank::Update()
{
    //每日结算
    int iHour = CGameTime::Instance().GetCurrHour();
    if ((iHour == m_iSettleTime) && !m_bSettleFlag)
    {
        this->SendRankReward();
        m_bSettleFlag = true;
    }
    else if ((iHour != m_iSettleTime))
    {
        m_bSettleFlag = false;
    }

    //排行榜通知
    if (m_bNtfFlag)
    {
        this->SendRefreshTopListNtf();
    }

    static uint64_t LastWriteFileTime = 0;
    //TODO:增加脏数据节点
    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();
    if (int(ullCurTime -LastWriteFileTime) >= m_iWriteTimeVal)
    {
        LastWriteFileTime = ullCurTime;
        this->_SaveToFile();
    }
}


void AsyncPvpRank::InitTopList()
{
    //初始化TopList
    AsyncPvpPlayer* poPlayer = NULL;
    for (int i = 0; i < MAX_RANK_TOP_NUM; i++)
    {
        poPlayer = AsyncPvpPlayerMgr::Instance().GetPlayer(m_Rank[i]);
        assert(poPlayer);
        poPlayer->GetShowData(m_astTopList[i]);
    }

    m_bNtfFlag = true;
}


void AsyncPvpRank::_Resize()
{
    m_iCapacity += 5000;
    uint64_t* temp = new uint64_t[m_iCapacity];
    memcpy(temp, m_Rank, sizeof(uint64_t)*m_iCount);
    delete[] m_Rank;
    m_Rank = temp;
}


bool AsyncPvpRank::SwapRank(AsyncPvpPlayer* poPlayerLeft, AsyncPvpPlayer* poPlayerRight)
{
    uint32_t dwRankLeft = poPlayerLeft->GetRank();
    uint32_t dwRankRight = poPlayerRight->GetRank();

    poPlayerLeft->SetRank(dwRankRight);
    poPlayerRight->SetRank(dwRankLeft);

    m_Rank[dwRankLeft-1] = poPlayerRight->GetPlayerId();
    m_Rank[dwRankRight-1] = poPlayerLeft->GetPlayerId();

    m_oIter = m_oRankMap.find(poPlayerLeft->GetPlayerId());
    if (m_oIter != m_oRankMap.end())
    {
        m_oIter->second = dwRankRight;
    }

    m_oIter = m_oRankMap.find(poPlayerRight->GetPlayerId());
    if (m_oIter != m_oRankMap.end())
    {
        m_oIter->second = dwRankLeft;
    }

    if (dwRankLeft <= MAX_RANK_TOP_NUM)
    {
        poPlayerRight->GetShowData(m_astTopList[dwRankLeft-1]);
        m_bNtfFlag = true;
    }

    if (dwRankRight <= MAX_RANK_TOP_NUM)
    {
        poPlayerLeft->GetShowData(m_astTopList[dwRankRight-1]);
        m_bNtfFlag = true;
    }

    return true;
}


uint32_t AsyncPvpRank::GetRankByPlayer(uint64_t ullUin)
{
    m_oIter = m_oRankMap.find(ullUin);
    if (m_oIter != m_oRankMap.end())
    {
        return m_oIter->second;
    }
    else
    {
        if (m_iCount >= m_iCapacity)
        {
            this->_Resize();
        }

        m_Rank[m_iCount++] = ullUin;
        m_oRankMap.insert(Id2RankMap_t::value_type(ullUin, m_iCount));

        return m_iCount;
    }
}


uint64_t AsyncPvpRank::GetPlayerByRank(uint32_t dwRank)
{
    if (dwRank > (uint32_t)m_iCount || dwRank < 1)
    {
        return 0;
    }
    return m_Rank[dwRank -1];
}


uint8_t AsyncPvpRank::GetTop10List(DT_ASYNC_PVP_PLAYER_SHOW_DATA astOpponentList[])
{
    memcpy(astOpponentList, m_astTopList, sizeof(DT_ASYNC_PVP_PLAYER_SHOW_DATA) * 10);
    return 10;
}


void AsyncPvpRank::_SaveToFile()
{
    fseek(m_fp, 0, SEEK_SET);

    if (fwrite(&m_iCount, sizeof(int), 1, m_fp) != 1)
    {
       LOGERR_r("Save count to file failed");
       return;
    }

    if (fwrite(m_Rank, sizeof(uint64_t), m_iCount, m_fp) != (size_t)m_iCount)
    {
       LOGERR_r("Save ranklist to file failed");
       return;
    }

    fflush(m_fp);
}


void AsyncPvpRank::_BackUpRankFile()
{
    ASYNCPVPSVRCFG& rstCfg = AsyncPvpSvr::Instance().GetConfig();
    char szFileName[MAX_NAME_LENGTH];
    snprintf(szFileName, MAX_NAME_LENGTH, "%s.bak", rstCfg.m_szRankFile);

    FILE* fp = fopen(szFileName, "wb+");
    if (!fp)
    {
        LOGERR_r("Can't open file %s", szFileName);
    }

    fseek(fp, 0, SEEK_SET);

    if (fwrite(&m_iCount, sizeof(int), 1, fp) != 1)
    {
       LOGERR_r("Save count to file failed");
       return;
    }

    if (fwrite(m_Rank, sizeof(uint64_t), m_iCount, fp) != (size_t)m_iCount)
    {
       LOGERR_r("Save ranklist to file failed");
       return;
    }

    fflush(fp);
    fclose(fp);

    return;
}


void AsyncPvpRank::Fini()
{
    this->_SaveToFile();
    fclose(m_fp);
    delete[] m_Rank;
}
