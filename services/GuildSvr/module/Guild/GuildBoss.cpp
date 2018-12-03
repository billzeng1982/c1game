#include "LogMacros.h"
#include "GuildBoss.h"
#include "../../gamedata/GameDataMgr.h"
#include "strutil.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "GuildBossMgr.h"
#include "FakeRandom.h"

using namespace std;
using namespace PKGMETA;

const int GuildBoss::GUILD_BOSS_SUBSECTION_REWARD_MAIL_ID = 10021;
const int GuildBoss::GUILD_BOSS_DAMAGE_RANK_REWARD_MAIL_ID = 10022;

static int BossListCmp( const void *key, const void *p )
{
    uint32_t dwBossID = *(uint32_t*)key;
    DT_GUILD_ONE_BOSS_INFO* pstBoss = (DT_GUILD_ONE_BOSS_INFO*)p;

    int iResult = (int)dwBossID - (int)pstBoss->m_dwBossId;

    return iResult;
}

template <typename T_PNODE>
static int UinCmp(const void * pFirst, const void * pSecond)
{
    //return ((T_PNODE *)pFirst)->m_ullUin - ((T_PNODE *)pSecond)->m_ullUin;
    if (((T_PNODE *)pFirst)->m_ullUin > ((T_PNODE *)pSecond)->m_ullUin)
    {
        return 1;
    }
    else if (((T_PNODE *)pFirst)->m_ullUin < ((T_PNODE *)pSecond)->m_ullUin)
    {
        return -1;
    }
    return 0;
}

static int DamageCmp(const void * pFirst, const void * pSecond)
{
    if (((DT_GUILD_BOSS_DAMAGE_NODE *)pSecond)->m_dwDamageHp > ((DT_GUILD_BOSS_DAMAGE_NODE *)pFirst)->m_dwDamageHp)
    {
        return 1;
    }
    else if (((DT_GUILD_BOSS_DAMAGE_NODE *)pSecond)->m_dwDamageHp < ((DT_GUILD_BOSS_DAMAGE_NODE *)pFirst)->m_dwDamageHp)
    {
        return -1;
    }
    return 0;
}

bool GuildBoss::InitFromDB(DT_GUILD_BOSS_BLOB& rstBlob, uint64_t ullGuildId, uint16_t wVersion)
{
    bzero(&m_oBossInfo, sizeof(m_oBossInfo));
    size_t ulUseSize = 0;
    int iRet = m_oBossInfo.unpack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);

    if (m_bIsAdapted == false)
    {
        ResetFightTimesInfo();
    }
    uint32_t dwBossMaxHp = GuildBossMgr::Instance().GetBossMaxHp(m_oBossInfo.m_dwCurBossId);

    //BossIndex查找对应Boss
    int iBossIndex = this->FindBossIndex(m_oBossInfo.m_dwCurBossId);

    if (iBossIndex >= MAX_GUILD_BOSS_NUM || iBossIndex < 0)
    {
        return false;
    }
    if (dwBossMaxHp > m_oBossInfo.m_astBossList[iBossIndex].m_dwDamegeHp)
    {
        m_oBossInfo.m_astBossList[iBossIndex].m_dwLeftHp = dwBossMaxHp - m_oBossInfo.m_astBossList[iBossIndex].m_dwDamegeHp;
    }
    else
    {
        m_oBossInfo.m_astBossList[iBossIndex].m_dwLeftHp = 0;
    }

    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Guild(%lu) init Apply failed, unpack DT_GUILD_BOSS_BLOB failed, Ret=%d", ullGuildId, iRet);
        return false;
    }
    m_ullGuildId = ullGuildId;
    if (!InitFromFile())
    {
        return false;
    }
    // 检查并添加新配置boss至boss list
    this->_AddNewBossFromRes();

    return true;
}

// 添加不存在的boss至boss list
void GuildBoss::_AddNewBossFromRes()
{
    if( 0 == m_oBossInfo.m_nBossNum )
    {
        // 新工会走 NewInit()流程
        return;
    }

    RESGUILDBOSSINFO* poResGuildBossInfo = NULL;
    uint16_t wResNum = CGameDataMgr::Instance().GetResGuildBossInfoMgr().GetResNum();
    for (int i = 0; i < wResNum && i < PKGMETA::MAX_GUILD_BOSS_NUM; i++)
    {
        poResGuildBossInfo = CGameDataMgr::Instance().GetResGuildBossInfoMgr().GetResByPos(i);
        assert( poResGuildBossInfo );

        uint32_t dwBossID = poResGuildBossInfo->m_dwId;
        size_t nmemb = (size_t)m_oBossInfo.m_nBossNum;
        if (nmemb >= MAX_GUILD_BOSS_NUM)
        {
            LOGERR("m_oBossInfo.m_nBossNum<%d> reaches the max.", m_oBossInfo.m_nBossNum);
            return;
        }

        MyBInsert(&dwBossID, m_oBossInfo.m_astBossList, &nmemb, sizeof(DT_GUILD_ONE_BOSS_INFO), 1, BossListCmp);
        m_oBossInfo.m_nBossNum = nmemb;
    }
}

// <0 表示出错
int GuildBoss::FindBossIndex( uint32_t dwBossID )
{
    int iEqual = 0;
    int idx = -1;
    idx = MyBSearch( &dwBossID, m_oBossInfo.m_astBossList, m_oBossInfo.m_nBossNum, sizeof(DT_GUILD_ONE_BOSS_INFO), &iEqual, BossListCmp);
    if( iEqual > 0 )
    {
        return idx;
    }else
    {
        return -1;
    }
}

void GuildBoss::SendSubsectionRwd(uint32_t dwBossId)
{
    int iBossIndex = FindBossIndex(dwBossId);
    if (iBossIndex < 0 && iBossIndex >= MAX_GUILD_BOSS_NUM)
    {
        LOGERR_r("Cannot find Boss");
        return;
    }
    int iCount = 0;
    uint64_t ullRet[5] = { 0 };
    GuildBossMgr::Instance().GetSubsectionAward(m_oBossInfo.m_astBossList[iBossIndex], iCount, ullRet);
    RESGUILDBOSSINFO* poGuildBossInfo = CGameDataMgr::Instance().GetResGuildBossInfoMgr().Find(dwBossId);
    if (poGuildBossInfo == NULL)
    {
        LOGERR_r("Boss<%u> is NULL", dwBossId);
        return;
    }
    const uint16_t wSubsectionNum = poGuildBossInfo->m_bSubsectionRwdTypeNum;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    SS_PKG_MAIL_ADD_REQ& rstMailAddReq = m_stSsPkg.m_stBody.m_stMailAddReq;
    DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
    rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
    rstMailData.m_bState = MAIL_STATE_UNOPENED;
    rstMailData.m_ullFromUin = 0;
    rstMailAddReq.m_nUinCount = 1;
    rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();
    RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(GUILD_BOSS_SUBSECTION_REWARD_MAIL_ID);
    if (NULL == poResPriMail)
    {
        LOGERR("Can't find the mail<id %u>", GUILD_BOSS_SUBSECTION_REWARD_MAIL_ID);
        return;
    }
    StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
    snprintf(rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN, poResPriMail->m_szContent, poGuildBossInfo->m_szName);
    DT_ITEM& rstItem = rstMailData.m_astAttachmentList[0];
    //当可领奖人数小于所有奖励数量时，随机生成奖励下标并分配给对应玩家
    uint32_t astResult[5] = { 0 };
    if (iCount != wSubsectionNum)
    {
        CFakeRandom::Instance().Random(wSubsectionNum, iCount, astResult);
    }
    for (int i = 0; i < iCount; ++i)
    {
        rstMailAddReq.m_UinList[0] = ullRet[i];
        rstMailData.m_bAttachmentCount = 1;
        rstItem.m_bItemType = poGuildBossInfo->m_szSubsectionRwdType[astResult[i]];
        rstItem.m_dwItemId = poGuildBossInfo->m_subsectionRwdId[astResult[i]];
        rstItem.m_iValueChg = poGuildBossInfo->m_szSubsectionRwdNum[astResult[i]];
        rstItem.m_bShowProperty = 1;
        SetSubRwdList(ullRet[i], 1, rstItem);
        GuildSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
    }
}

void GuildBoss::ResetMemFightTimesToZero()
{
    int iCount = m_oBossInfo.m_wMemNum;
    for (int i = 0; i < iCount; ++i)
    {
        m_oBossInfo.m_astMemFightBossTimes[i].m_bFightTimes = 0;
    }
}

void GuildBoss::SendDamageRankRwd(uint32_t dwBossId)
{
    int iBossIndex = FindBossIndex(dwBossId);
    if (iBossIndex < 0 && iBossIndex >= MAX_GUILD_BOSS_NUM)
    {
        return;
    }
    int iCount = m_oBossInfo.m_astBossList[iBossIndex].m_wAttackedMemNum;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    SS_PKG_MAIL_ADD_REQ& rstMailAddReq = m_stSsPkg.m_stBody.m_stMailAddReq;
    DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
    rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
    rstMailData.m_bState = MAIL_STATE_UNOPENED;
    rstMailData.m_ullFromUin = 0;
    rstMailAddReq.m_nUinCount = 1;
    rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();

    DT_GUILD_BOSS_DAMAGE_NODE astDamageList[MAX_GUILD_MEMBER_MAX_NUM];
    memcpy(astDamageList, m_oBossInfo.m_astBossList[iBossIndex].m_astDamageOfBossList, sizeof(DT_GUILD_BOSS_DAMAGE_NODE) * iCount);
    qsort(astDamageList, iCount, sizeof(DT_GUILD_BOSS_DAMAGE_NODE), DamageCmp);

    RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(GUILD_BOSS_DAMAGE_RANK_REWARD_MAIL_ID);
    if (NULL == poResPriMail)
    {
        LOGERR("Can't find the mail<id %u>", GUILD_BOSS_DAMAGE_RANK_REWARD_MAIL_ID);
        return;
    }
    RESGUILDBOSSINFO* poGuildBossInfo = CGameDataMgr::Instance().GetResGuildBossInfoMgr().Find(dwBossId);
    if (poGuildBossInfo == NULL)
    {
        LOGERR_r("poGuildBossInfo is NULL");
        return;
    }
    StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);

    RESREWARDSHOW* pstResRwdShow = NULL;
    int iBreak = 0;
    //前面几名的固定奖励
    for (int i = 0; i < iCount; ++i)
    {
        pstResRwdShow = CGameDataMgr::Instance().GetResRewardShowMgr().GetResByPos(i);
        if (CGameDataMgr::Instance().GetResRewardShowMgr().GetResByPos(i + 1) != NULL &&
            pstResRwdShow->m_dwId + 1 != CGameDataMgr::Instance().GetResRewardShowMgr().GetResByPos(i + 1)->m_dwId)
        {
            iBreak = i;
            break;
        }
        rstMailData.m_bAttachmentCount = pstResRwdShow->m_bRewardCount;
        for (int j = 0; j < rstMailData.m_bAttachmentCount; ++j)
        {
            rstMailData.m_astAttachmentList[j].m_bItemType = pstResRwdShow->m_szItemType[j];
            rstMailData.m_astAttachmentList[j].m_dwItemId = pstResRwdShow->m_itemId[j];
            rstMailData.m_astAttachmentList[j].m_iValueChg = pstResRwdShow->m_itemNumber[j];
            rstMailData.m_astAttachmentList[j].m_bShowProperty = 1;
        }
        rstMailAddReq.m_UinList[0] = astDamageList[i].m_ullUin;
        LOGRUN_r("Send Damage Rank Rwd To Player<%lu>", rstMailAddReq.m_UinList[0]);
        snprintf(rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN, poResPriMail->m_szContent, poGuildBossInfo->m_szName, i + 1);
        GuildSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
    }
    //发剩余排名的奖励
    if (iBreak != 0)
    {
        for (int i = iBreak; i < iCount; ++i)
        {
            pstResRwdShow = CGameDataMgr::Instance().GetResRewardShowMgr().GetResByPos(iBreak);
            rstMailData.m_bAttachmentCount = pstResRwdShow->m_bRewardCount;
            for (int j = 0; j < rstMailData.m_bAttachmentCount; ++j)
            {
                rstMailData.m_astAttachmentList[j].m_bItemType = pstResRwdShow->m_szItemType[j];
                rstMailData.m_astAttachmentList[j].m_dwItemId = pstResRwdShow->m_itemId[j];
                rstMailData.m_astAttachmentList[j].m_iValueChg = pstResRwdShow->m_itemNumber[j];
                rstMailData.m_astAttachmentList[j].m_bShowProperty = 1;
            }
            rstMailAddReq.m_UinList[0] = astDamageList[i].m_ullUin;
            snprintf(rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN, poResPriMail->m_szContent, poGuildBossInfo->m_szName, i + 1);
            GuildSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
        }
    }
}

void GuildBoss::SetSubRwdList(uint64_t ullUin, uint8_t bCount, DT_ITEM stItem)
{
    size_t nMemGetSubRwdNumTemp = (size_t)m_oBossInfo.m_wMemGetSubRwdNum;
    if (nMemGetSubRwdNumTemp >= MAX_GUILD_MEMBER_MAX_NUM)
    {
        LOGERR("m_oBossInfo.m_wMemGetSubRwdNum<%d> reaches the max.", m_oBossInfo.m_wMemGetSubRwdNum);
        return;
    }
    uint64_t ullTimeStamp = CGameTime::Instance().GetCurrSecond();
    DT_GUILD_BOSS_GET_SUB_RWD_NODE stTemp = { 0 };
    stTemp.m_ullUin = ullUin;
    stTemp.m_bSubRwdNum = bCount;
    stTemp.m_astSubRwdItem[0] = stItem;
    stTemp.m_ullTimeStamp = ullTimeStamp;
    MyBInsert(&stTemp, m_oBossInfo.m_astMemGetSubRwdList, &nMemGetSubRwdNumTemp,
        sizeof(DT_GUILD_BOSS_GET_SUB_RWD_NODE), 0, UinCmp<DT_GUILD_BOSS_GET_SUB_RWD_NODE>);
    m_oBossInfo.m_wMemGetSubRwdNum = nMemGetSubRwdNumTemp;
}

void GuildBoss::DelMemGetSubRwdInfo(uint64_t ullUin)
{
    DT_GUILD_BOSS_GET_SUB_RWD_NODE stTemp = { 0 };
    stTemp.m_ullUin = ullUin;
    int iEql = 0;
    size_t uMemNum = m_oBossInfo.m_wMemGetSubRwdNum;
    MyBSearch(&stTemp, m_oBossInfo.m_astMemGetSubRwdList, uMemNum,
        sizeof(DT_GUILD_BOSS_GET_SUB_RWD_NODE), &iEql, UinCmp<DT_GUILD_BOSS_GET_SUB_RWD_NODE>);
    while (iEql != 0 && uMemNum > 0)
    {
        MyBDelete(&stTemp, m_oBossInfo.m_astMemGetSubRwdList, &uMemNum,
            sizeof(DT_GUILD_BOSS_GET_SUB_RWD_NODE), UinCmp<DT_GUILD_BOSS_GET_SUB_RWD_NODE>);
        m_oBossInfo.m_wMemGetSubRwdNum = uMemNum;
    }
}

void GuildBoss::DelMemFightTimesInfo(uint64_t ullUin)
{
    size_t uMemNum = m_oBossInfo.m_wMemNum;
    DT_GUILD_BOSS_FIGHT_TIMES *pstFightTimes = m_oBossInfo.m_astMemFightBossTimes;
    DT_GUILD_BOSS_FIGHT_TIMES stFightTimes;
    stFightTimes.m_ullUin = ullUin;
    int iRet = MyBDelete(&stFightTimes, pstFightTimes, &uMemNum, sizeof(DT_GUILD_BOSS_FIGHT_TIMES), UinCmp<DT_GUILD_BOSS_FIGHT_TIMES>);
    if (iRet != 1)
    {
        LOGERR_r("Unable to delete player<%lu> boss fight times info.", ullUin);
        return;
    }
    m_oBossInfo.m_wMemNum = uMemNum;
    //删除玩家在公会中的Boss战斗伤害信息
    uint32_t dwCurBossId = m_oBossInfo.m_dwCurBossId;
    int iCurBossIndex = FindBossIndex(dwCurBossId);
    if (iCurBossIndex >= 0)
    {
        for (int i = 0; i < iCurBossIndex; i++)
        {
            DelMemDamageInfo(ullUin, i);
        }
    }
}

int GuildBoss::AddMemFightTimesInfo(size_t nMemNum, uint64_t ullUin)
{
    //DT_GUILD_BOSS_FIGHT_TIMES *pstFightTimes = m_oBossInfo.m_astMemFightBossTimes;
    DT_GUILD_BOSS_FIGHT_TIMES stTempFightTimes = { 0 };
    stTempFightTimes.m_ullUin = ullUin;
    stTempFightTimes.m_bFightTimes = 0;
    int iInsertRet = MyBInsert(&stTempFightTimes, m_oBossInfo.m_astMemFightBossTimes, &nMemNum, sizeof(DT_GUILD_BOSS_FIGHT_TIMES), 1, UinCmp<DT_GUILD_BOSS_FIGHT_TIMES>);
    if (iInsertRet == 0)
    {
        LOGERR_r("Insert player<%lu> fight times failed.", ullUin);
        return ERR_DEFAULT;
    }
    else
    {
        ++m_oBossInfo.m_wMemNum;
        return ERR_NONE;
    }
}

//bugfix
void GuildBoss::ResetFightTimesInfo()
{
    DT_GUILD_BOSS_FIGHT_TIMES* pstFightTimes = m_oBossInfo.m_astMemFightBossTimes;
    DT_GUILD_BOSS_FIGHT_TIMES astNewFightTimes[MAX_GUILD_MEMBER_MAX_NUM];
    size_t nNewCount = 0;
    for (int i = 0; i < m_oBossInfo.m_wMemNum; ++i)
    {
        MyBInsert(&pstFightTimes[i],
            astNewFightTimes, 
            &nNewCount, 
            sizeof(DT_GUILD_BOSS_FIGHT_TIMES), 
            1, 
            UinCmp<DT_GUILD_BOSS_FIGHT_TIMES>);
    }
    m_bIsAdapted = true;

    //重置数据
    for (int i = 0; i < nNewCount; ++i)
    {
        pstFightTimes[i] = astNewFightTimes[i];
    }
    m_oBossInfo.m_wMemNum = nNewCount;
}

bool GuildBoss::PackGuildBossInfo(PKGMETA::DT_GUILD_BOSS_BLOB& rstBlob, uint16_t wVersion)
{
    if (InitFromFile() == false)
    {
        LOGERR_r("Guild<%lu> init from file failed.", m_ullGuildId);
        return false;
    }
    size_t ulUseSize = 0;
    int iRet = m_oBossInfo.pack((char*)rstBlob.m_szData, MAX_LEN_GUILD_BOSS, &ulUseSize, wVersion);
    LOGRUN_r("There are <%d> bosses.", m_oBossInfo.m_nBossNum);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Guild(%lu) pack DT_GUILD_BOSS_BLOB failed, iRet=%d, UsedSize=%lu", m_ullGuildId, iRet, ulUseSize);
        return false;
    }
    rstBlob.m_iLen = (int)ulUseSize;
    return true;
}

void GuildBoss::Clear()
{
    m_ullGuildId = 0;
    bzero(&m_oBossInfo, sizeof(m_oBossInfo));
    //m_oBossInfo.m_dwCurBossId = 1;
}

bool GuildBoss::InitFromFile()
{
    int iCurrBossIndex = FindBossIndex(m_oBossInfo.m_dwCurBossId);
    if (iCurrBossIndex >= 0 && iCurrBossIndex < m_oBossInfo.m_nBossNum)
    {
        m_oBossInfo.m_astBossList[iCurrBossIndex].m_wPassedGuildNum = GuildBossMgr::Instance().GetPassedGuildNum(iCurrBossIndex);
        return true;
    }
    else
    {
        LOGERR_r("Cannot find Boss<%u>", m_oBossInfo.m_dwCurBossId);
        return false;
    }
}

//  创建公会时初始化
bool GuildBoss::NewInit()
{
    Clear();
    Reset();
    return true;
}

void GuildBoss::AddDamage(uint64_t ullUin, uint32_t dwFLevelId, uint32_t dwDamageHp, OUT uint8_t& bKilled, OUT uint32_t& dwBossId)
{
    dwBossId = GuildBossMgr::Instance().FindBossID(dwFLevelId);
    uint32_t dwBossMaxHp = GuildBossMgr::Instance().GetBossMaxHp( dwBossId );

    //BossIndex查找对应Boss
    int iBossIndex = this->FindBossIndex(dwBossId);

    if (iBossIndex >= MAX_GUILD_BOSS_NUM || iBossIndex < 0 )
    {
        LOGERR_r("FATAL ERROR: BossId Error Uin<%lu> Guild<%lu> BossId<%u> FLevel<%u> DamageHp<%u>",
            ullUin, m_ullGuildId, dwBossId, dwFLevelId, dwDamageHp );
        return;
    }

    if (dwBossId != m_oBossInfo.m_dwCurBossId || m_oBossInfo.m_astBossList[iBossIndex].m_bState == GUILD_BOSS_STATE_KILLED)
    {
    #ifdef _DEBUG
        LOGWARN_r("Boss add damage, not the current boss. Uin<%lu> Guild<%lu> BossId<%u> DamageHp<%u>",ullUin, m_ullGuildId,
            dwBossId, dwDamageHp);
    #endif
        return;
    }
    uint32_t dwOld = m_oBossInfo.m_astBossList[iBossIndex].m_dwDamegeHp;
    m_oBossInfo.m_astBossList[iBossIndex].m_dwDamegeHp += dwDamageHp;
    if ( m_oBossInfo.m_astBossList[iBossIndex].m_dwDamegeHp > dwBossMaxHp )
    {// Boss被击杀
        m_oBossInfo.m_astBossList[iBossIndex].m_bState = GUILD_BOSS_STATE_KILLED;
        bKilled = 1;
        LOGRUN_r("GuildBoss is killed! Uin<%lu>, GID<%lu>, BossId<%u>, MaxHp<%u>, TotalDamage<%u> ", ullUin, m_ullGuildId, dwBossId, dwBossMaxHp, m_oBossInfo.m_astBossList[iBossIndex].m_dwDamegeHp);
    }
    m_oBossInfo.m_astBossList[iBossIndex].m_dwLeftHp = (bKilled == 1) ? 0 : (dwBossMaxHp - m_oBossInfo.m_astBossList[iBossIndex].m_dwDamegeHp);
    SetMemDamageList(ullUin, dwBossId, dwDamageHp);
    LOGRUN_r("Uin<%lu>, GID<%lu>, BossId<%u> OldHp<%u> damage the boss Hp<%u>", ullUin, m_ullGuildId, dwBossId, dwOld, dwDamageHp);
}

// 隔月刷新重置
void GuildBoss::Reset()
{
    int iCurrBossIndex = this->FindBossIndex(m_oBossInfo.m_dwCurBossId);
    if (iCurrBossIndex >= 0)
    {
        for (size_t i = 1; i <= m_oBossInfo.m_dwCurBossId; ++i)
        {
            ResetMemDamageList(i);
            m_oBossInfo.m_astBossList[i].m_bPeriodStayed = 0;
        }
        for (int i = 0; i < m_oBossInfo.m_nBossNum; ++i)
        {
            GuildBossMgr::Instance().ResetPassedGuild(i);
        }
    }
    this->_ResetBossListFromRes();
    this->_Unlock1stBoss();
    iCurrBossIndex = this->FindBossIndex(m_oBossInfo.m_dwCurBossId);
    if (iCurrBossIndex >= 0 && iCurrBossIndex < m_oBossInfo.m_nBossNum)
    {
        m_oBossInfo.m_astBossList[iCurrBossIndex].m_wPassedGuildNum = GuildBossMgr::Instance().GetPassedGuildNum(iCurrBossIndex);
    }
}

//将玩家对Boss造成的伤害记录到伤害展示数组中
void GuildBoss::SetMemDamageList(uint64_t ullUin, uint32_t dwBossId, uint32_t dwDamage)
{
    //通过BossId查找下标
    int iBossIndex = this->FindBossIndex(dwBossId);
    if (iBossIndex >= MAX_GUILD_BOSS_NUM || iBossIndex < 0)
    {
        LOGERR_r("Cannot find Boss<ID=%u>", dwBossId);
        return;
    }

    //将该玩家的伤害信息插入到伤害展示数组中
    size_t nMemNum = (size_t)m_oBossInfo.m_astBossList[iBossIndex].m_wAttackedMemNum;
    if (nMemNum >= MAX_GUILD_MEMBER_MAX_NUM)
    {
        LOGERR("m_oBossInfo.m_astBossList[iBossIndex].m_wAttackedMemNum<%d> reaches the max.", m_oBossInfo.m_astBossList[iBossIndex].m_wAttackedMemNum);
        return;
    }
    DT_GUILD_BOSS_DAMAGE_NODE *pstDamageNode = m_oBossInfo.m_astBossList[iBossIndex].m_astDamageOfBossList;
    DT_GUILD_BOSS_DAMAGE_NODE stDamageNode = { 0 };
    stDamageNode.m_ullUin = ullUin;
    stDamageNode.m_dwDamageHp = dwDamage;
    int iEqual = 0;
    int iIndexOfDamageList = MyBSearch(&stDamageNode, pstDamageNode, nMemNum,
        sizeof(DT_GUILD_BOSS_DAMAGE_NODE), &iEqual, UinCmp<DT_GUILD_BOSS_DAMAGE_NODE>);
    if (iEqual == 0)
    {
        //找不到时，直接增加一个节点，并增加数组的有效长度
        /*pstDamageNode[iIndexOfDamageList].m_dwDamageHp = dwDamage;
        pstDamageNode[iIndexOfDamageList].m_ullUin = ullUin;
        ++m_oBossInfo.m_astBossList[iBossIndex].m_wAttackedMemNum;
        LOGRUN_r("Player<%lu> first attack Boss<%u>, damageHp is<%u>",
            pstDamageNode[nMemNum].m_ullUin, dwBossId, pstDamageNode[nMemNum].m_dwDamageHp);*/
        MyBInsert(&stDamageNode, pstDamageNode, &nMemNum, sizeof(DT_GUILD_BOSS_DAMAGE_NODE), 1, UinCmp<DT_GUILD_BOSS_DAMAGE_NODE>);
        m_oBossInfo.m_astBossList[iBossIndex].m_wAttackedMemNum = nMemNum;
        return;
    }
    //能找到，就增加伤害数值
    pstDamageNode[iIndexOfDamageList].m_dwDamageHp += dwDamage;
    LOGRUN_r("Player<%lu> attacked Boss<%u>, damageHp NOW is <%u>",
        pstDamageNode[iIndexOfDamageList].m_ullUin, dwBossId, pstDamageNode[iIndexOfDamageList].m_dwDamageHp);
}

void GuildBoss::ResetMemDamageList(uint64_t dwBossId)
{
    //通过BossId查找下标
    int iBossIndex = this->FindBossIndex(dwBossId);
    if (iBossIndex >= MAX_GUILD_BOSS_NUM || iBossIndex < 0)
    {
        LOGERR_r("Cannot find Boss<ID=%lu>", dwBossId);
        return;
    }

    //将该Boss所有玩家的伤害信息节点赋空
    bzero(m_oBossInfo.m_astBossList[iBossIndex].m_astDamageOfBossList,
        sizeof(DT_GUILD_BOSS_DAMAGE_NODE) * m_oBossInfo.m_astBossList[iBossIndex].m_wAttackedMemNum);
    m_oBossInfo.m_astBossList[iBossIndex].m_wAttackedMemNum = 0;
    LOGRUN_r("ResetMemDamageList success!");
}

//玩家退出公会时删除该玩家的伤害信息
void GuildBoss::DelMemDamageInfo(uint64_t ullUin, uint32_t dwBossIndex)
{
    if (dwBossIndex >= MAX_GUILD_BOSS_NUM)
    {
        LOGERR_r("Cannot find Boss<INDEX=%u>", dwBossIndex);
        return;
    }

    size_t nMemNum = (size_t)m_oBossInfo.m_astBossList[dwBossIndex].m_wAttackedMemNum;
    DT_GUILD_BOSS_DAMAGE_NODE *pstDamageNode = m_oBossInfo.m_astBossList[dwBossIndex].m_astDamageOfBossList;
    DT_GUILD_BOSS_DAMAGE_NODE stDamageNode = { 0 };
    stDamageNode.m_ullUin = stDamageNode.m_ullUin;
    if (!MyBDelete(&stDamageNode, pstDamageNode, &nMemNum, sizeof(DT_GUILD_BOSS_DAMAGE_NODE), UinCmp<DT_GUILD_BOSS_DAMAGE_NODE>))
    {
        LOGERR_r("Delete player<%lu> damage info of Boss<INDEX=%u> failed", ullUin, dwBossIndex);
        return;
    }
    LOGRUN_r("Delete player<%lu> damage info of Boss<INDEX=%u>", ullUin, dwBossIndex);
}

DT_GUILD_ONE_BOSS_INFO * GuildBoss::GetOneBossInfo(uint32_t dwBossId)
{
    int iBossIndex = this->FindBossIndex(dwBossId);
    if (iBossIndex < 0)
    {
        LOGERR_r("Boss<%u> not found.", dwBossId);
        return NULL;
    }
    return &m_oBossInfo.m_astBossList[iBossIndex];
}

void GuildBoss::_ResetBossListFromRes()
{
    RESGUILDBOSSINFO* poResGuildBossInfo = NULL;
    RESFIGHTLEVELGENERAL* poResFightLevelGeneralInfo = NULL;
    uint16_t wBossNum = CGameDataMgr::Instance().GetResGuildBossInfoMgr().GetResNum();

    for (int i = 0; i < wBossNum && i < PKGMETA::MAX_GUILD_BOSS_NUM; i++)
    {
        poResGuildBossInfo = CGameDataMgr::Instance().GetResGuildBossInfoMgr().GetResByPos(i);
        assert(poResGuildBossInfo);
        poResFightLevelGeneralInfo = CGameDataMgr::Instance().GetResFightLevelGeneralMgr().Find(poResGuildBossInfo->m_dwGeneralId);
        assert(poResFightLevelGeneralInfo);

        m_oBossInfo.m_astBossList[i].m_dwBossId = poResGuildBossInfo->m_dwId;
        m_oBossInfo.m_astBossList[i].m_bState = GUILD_BOSS_STATE_LOCK;
        m_oBossInfo.m_astBossList[i].m_dwDamegeHp = 0;
        m_oBossInfo.m_astBossList[i].m_dwLeftHp = poResFightLevelGeneralInfo->m_dwInitHP;
    }
    m_oBossInfo.m_nBossNum = (int16_t)wBossNum;
}

void GuildBoss::_Unlock1stBoss()
{
    m_oBossInfo.m_dwCurBossId = 1;
    m_oBossInfo.m_astBossList[0].m_bState = GUILD_BOSS_STATE_UNLOCK;
    m_oBossInfo.m_ullLastUptTime = CGameTime::Instance().GetCurrSecond();
    m_oBossInfo.m_ullLastSingleUptTime = CGameTime::Instance().GetCurrSecond();
}

void GuildBoss::SingleReset()
{
    int iCurBossIdx = this->FindBossIndex( m_oBossInfo.m_dwCurBossId );
    assert( iCurBossIdx >= 0 );

    uint32_t dwMaxHp = GuildBossMgr::Instance().GetBossMaxHp(m_oBossInfo.m_dwCurBossId);

    m_oBossInfo.m_astBossList[ iCurBossIdx ].m_dwDamegeHp = 0;
    m_oBossInfo.m_astBossList[iCurBossIdx].m_dwLeftHp = dwMaxHp;
    ++m_oBossInfo.m_astBossList[iCurBossIdx].m_bPeriodStayed;

    m_oBossInfo.m_wMemGetSubRwdNum = 0;

    ResetMemDamageList(m_oBossInfo.m_dwCurBossId);
}

bool GuildBoss::UnlockNextBoss(uint8_t bGuildLv)
{
    int iCurBossIdx = this->FindBossIndex( m_oBossInfo.m_dwCurBossId );
    assert( iCurBossIdx >= 0 );

    if ( iCurBossIdx >= m_oBossInfo.m_nBossNum )
    {//已全部击杀
        return false;
    }
    uint32_t dwNextBossIndex = iCurBossIdx+1;

    RESGUILDBOSSINFO* poResGuildBossInfo = CGameDataMgr::Instance().GetResGuildBossInfoMgr().GetResByPos(dwNextBossIndex);
    if (!poResGuildBossInfo)
    {
        LOGERR_r("Find the gamedata of RESGUILDBOSSINFO error GuildId<%lu>, <%u>",m_ullGuildId, m_oBossInfo.m_dwCurBossId);
        return false;
    }
    if (poResGuildBossInfo->m_wGuildLvLimit > bGuildLv)
    {
        return false;
    }

    int iPreBossIndex = this->FindBossIndex(poResGuildBossInfo->m_dwPreId);
    assert( iPreBossIndex>=0 );

    if ( 0 != poResGuildBossInfo->m_dwPreId  &&
         GUILD_BOSS_STATE_KILLED != m_oBossInfo.m_astBossList[iPreBossIndex].m_bState )
    {
        return false;
    }
    m_oBossInfo.m_dwCurBossId = poResGuildBossInfo->m_dwId;
    m_oBossInfo.m_astBossList[dwNextBossIndex].m_bState = GUILD_BOSS_STATE_UNLOCK;
    m_oBossInfo.m_astBossList[dwNextBossIndex].m_dwDamegeHp = 0;

    uint32_t dwMaxHp = GuildBossMgr::Instance().GetBossMaxHp(m_oBossInfo.m_dwCurBossId);
    m_oBossInfo.m_astBossList[dwNextBossIndex].m_dwLeftHp = dwMaxHp;

    SendSubsectionRwd(poResGuildBossInfo->m_dwPreId);
    SendDamageRankRwd(poResGuildBossInfo->m_dwPreId);
    m_oBossInfo.m_astBossList[iCurBossIdx].m_bPeriodStayed = 0;
    GuildBossMgr::Instance().SetPassedGuildNum(iCurBossIdx);

    return true;
}

int GuildBoss::GetBossLeftHp(uint64_t ullUin, uint32_t dwFLevelId , OUT uint32_t& dwBossLeftHp)
{
    uint32_t dwBossId = GuildBossMgr::Instance().FindBossID(dwFLevelId);
    if (0 == dwBossId)
    {
        LOGERR_r("Uin<%lu> GuildId<%lu> dwFevelId<%u> is error!", ullUin, m_ullGuildId, dwFLevelId);
        return ERR_SYS;
    }

    uint32_t dwBossMaxHp = GuildBossMgr::Instance().GetBossMaxHp( dwBossId );

    int iBossIndex = this->FindBossIndex(dwBossId);
    if( iBossIndex < 0 )
    {
        LOGERR_r("FATAL ERROR: BossId Error Uin<%lu> Guild<%lu> BossId<%u> FLevel<%u>",
            ullUin, m_ullGuildId, dwBossId, dwFLevelId );
        return ERR_GUILD_BOSS_NOT_FOUND;
    }

    if (m_oBossInfo.m_astBossList[iBossIndex].m_bState == GUILD_BOSS_STATE_KILLED)
    {
        LOGERR_r("Uin<%lu> GuildId<%lu> dwFevelId<%u> BossId<%u> is died!", ullUin, m_ullGuildId, dwFLevelId, dwBossId);
        return ERR_GUILD_BOSS_DIED;
    }
    dwBossLeftHp = dwBossMaxHp - m_oBossInfo.m_astBossList[iBossIndex].m_dwDamegeHp;
    return ERR_NONE;
}

int GuildBoss::GetBossLeftHp(uint32_t dwBossId)
{
    if (dwBossId == 0)
    {
        LOGERR_r("GuildId<%lu> is error!", m_ullGuildId);
        return ERR_SYS;
    }

    uint32_t dwBossMaxHp = GuildBossMgr::Instance().GetBossMaxHp( dwBossId );
    int iBossIndex = this->FindBossIndex(dwBossId);
    assert(iBossIndex>=0);

    if (m_oBossInfo.m_astBossList[iBossIndex].m_bState == GUILD_BOSS_STATE_KILLED)
    {
        LOGERR_r("GuildId<%lu> BossId<%u> is died!", m_ullGuildId, dwBossId);
        return ERR_GUILD_BOSS_DIED;
    }
    uint32_t dwBossLeftHp;
    dwBossLeftHp = dwBossMaxHp - m_oBossInfo.m_astBossList[iBossIndex].m_dwDamegeHp;
    return dwBossLeftHp;
}
