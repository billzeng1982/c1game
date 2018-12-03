#include "Guild.h"
#include "Item.h"
#include "../utils/FakeRandom.h"
#include "common_proto.h"
#include "GameTime.h"
#include <list>
#include "Consume.h"
#include "dwlog_def.h"
#include "ZoneLog.h"
#include "dwlog_svr.h"
#include "player/Player.h"
#include "Lottery.h"
#include "Majesty.h"
#include "Match.h"
#include "Props.h"
#include "SvrTime.h"
#include "Task.h"

using namespace std;
using namespace PKGMETA;
using namespace DWLOG;



bool Guild::_InitGuildTask()
{
    //初始化时间戳和更新时间
    ResBasicMgr_t& rstResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* pUpdateTime = rstResBasicMgr.Find(COMMON_UPDATE_TIME);
    if (pUpdateTime == NULL)
    {
        LOGERR("pUpdateTime is NULL");
        return false;
    }
    m_iTaskUptTime = (int)(pUpdateTime->m_para[0]);


    m_ullTaskLastUptTime = CGameTime::Instance().GetSecOfHourInCurrDay(m_iTaskUptTime);
    if (CGameTime::Instance().GetCurrHour() < m_iTaskUptTime)
    {
        m_ullTaskLastUptTime -= SECONDS_OF_DAY;
    }
    m_bTaskUptFlag = false;

    //初始化Randomlist
    for (int i=0; i<MAX_GUILD_LEVELS_NUM; i++)
    {
        RandomList[i] = i;
    }

    //初始化公会任务列表
    ResGuildTaskMgr_t& rstResGuildTaskMgr = CGameDataMgr::Instance().GetResGuildTaskMgr();
    int iNum = rstResGuildTaskMgr.GetResNum();
    if (iNum > MAX_GUILD_TASKNODE_NUM)
    {
        LOGERR("Guild Init fail, GuildTask ResNum is larger than MaxNum");
        return false;
    }

    m_TaskNodeCount = iNum;
    RESGUILDTASK* pstResGuildTask = NULL;
    for (int i=0; i<iNum; i++)
    {
        pstResGuildTask = rstResGuildTaskMgr.GetResByPos(i);
        m_TaskNodeList[i].bLevel = pstResGuildTask->m_bLevel;
        m_TaskNodeList[i].pstResGuildTask = pstResGuildTask;
        for (int j=0; j<pstResGuildTask->m_bLevelCount; j++)
        {
            m_LevelToRewardMap.insert(map<uint32_t, uint32_t>::value_type(pstResGuildTask->m_levelIdList[j], pstResGuildTask->m_rewardIdList[j]));
        }
    }

    return true;
}

bool Guild::_InitGuildDonate()
{
    ResBasicMgr_t& rstResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* pResBasic = rstResBasicMgr.Find(GUILD_DONATE_BY_GOLD);
    if (pResBasic == NULL)
    {
        LOGERR("Guild _InitGuildDonate fail, pResBasic is NULL");
        return false;
    }
    m_iGoldDonateConsume = (int)(pResBasic->m_para[0]);
    m_iGoldDonateFund = (int)(pResBasic->m_para[1]);
    m_iGoldDonateContribute = (int)(pResBasic->m_para[2]);
    m_iGoldDonateVitality = (int)pResBasic->m_para[3];

    pResBasic = rstResBasicMgr.Find(GUILD_DONATE_BY_DIAMOND);
    if (pResBasic == NULL)
    {
        LOGERR("Guild _InitGuildDonate fail, pResBasic is NULL");
        return false;
    }
    m_iDiamandDonateConsume = (int)(pResBasic->m_para[0]);
    m_iDiamandDonateFund = (int)(pResBasic->m_para[1]);
    m_iDiamandDonateContribute = (int)(pResBasic->m_para[2]);
    m_iDiamandDonateVitality = (int)(pResBasic->m_para[3]);

	pResBasic = rstResBasicMgr.Find(GUILD_VIP_DONATE_BY_DIAMOND);
	if (pResBasic == NULL)
	{
		LOGERR("Guild _InitGuildDonate fail, pResBasic is NULL");
		return false;
	}
	m_iVipDonateConsume = (int)(pResBasic->m_para[0]);
	m_iVipDonateFund = (int)(pResBasic->m_para[1]);
	m_iVipDonateContribute = (int)(pResBasic->m_para[2]);
    m_iVipDonateVitality = (int)(pResBasic->m_para[3]);

    pResBasic = rstResBasicMgr.Find(GUILD_DONATE_MAX_TIMES);
    if (pResBasic == NULL)
    {
        LOGERR("Guild _InitGuildDonate fail, pResBasic is NULL");
        return false;
    }
    m_iDonateMaxTimes = (int)(pResBasic->m_para[0]);

    return true;
}

bool Guild::_InitGuildBoss()
{
    m_ullGuildBossFightNumUptTime = CGameTime::Instance().GetSecOfHourInCurrDay(m_iGuildBossFightNumUptHour);
    if (CGameTime::Instance().GetCurrHour() < m_iGuildBossFightNumUptHour)
    {
        m_ullGuildBossFightNumUptTime -= SECONDS_OF_DAY;
    }
    m_bGuildBossFightNumUpteFlag = false;
    return LoadBossGameData();
}


bool Guild::LoadBossGameData()
{
    RESGUILDBOSSINFO* poResGuildBossInfo = NULL;
    RESGUILDBOSSREWARD* poResGuildBossReward = NULL;
    RESFIGHTLEVELGENERAL* poResFightLevelGeneral = NULL;

	uint32_t dwBossNum = CGameDataMgr::Instance().GetResGuildBossInfoMgr().GetResNum();
	if (dwBossNum > MAX_GUILD_BOSS_NUM)
	{
		LOGERR("dwBossNum<%u>  is out limit<%d>", dwBossNum, MAX_GUILD_BOSS_NUM);
		return false;
	}

    for (int i = 0; i < (int)dwBossNum; i++)
    {
        poResGuildBossInfo = CGameDataMgr::Instance().GetResGuildBossInfoMgr().GetResByPos(i);
        if (!poResGuildBossInfo)
        {
            LOGERR("Load gamedata error, RESGUILDBOSSINFO is NULL Pos<%d>", i);
            return false;
        }
        poResFightLevelGeneral = CGameDataMgr::Instance().GetResFightLevelGeneralMgr().Find(poResGuildBossInfo->m_dwGeneralId);
        if (!poResFightLevelGeneral)
        {
            LOGERR("Load gamedata error, RESGENERAL is NULL GeneralId<%d> the RESGUILDBOSSINFO pos<%d>",
                poResGuildBossInfo->m_dwGeneralId, i);
            return false;
        }

        m_BossHp[i] = poResFightLevelGeneral->m_dwInitHP;
        m_FLevelToBossIdMap[poResGuildBossInfo->m_dwLevelId] = poResGuildBossInfo->m_dwId;
    }

    uint32_t dwPreBossId = 0;
    uint32_t dwLastHigh = 0;
    BossSection stTemp ;

	uint32_t dwRewardNum = CGameDataMgr::Instance().GetResGuildBossRewardMgr().GetResNum();
    for (uint32_t i = 0 ; i < dwRewardNum; i++)
    {
        poResGuildBossReward = CGameDataMgr::Instance().GetResGuildBossRewardMgr().GetResByPos(i);
        if (!poResGuildBossReward)
        {
            LOGERR("the poResGuildBossReward is NULL!! Pos<%u> ", i);
            return false;
        }
        if (dwPreBossId != poResGuildBossReward->m_dwBossId)
        {
            dwLastHigh = 0;
        }
        stTemp.m_dwResId = poResGuildBossReward->m_dwId;
        stTemp.m_dwLow = dwLastHigh;
        stTemp.m_dwHight = poResGuildBossReward->m_dwDamagePercent;
        m_oBossRewardMap[poResGuildBossReward->m_dwBossId-1].push_back(stTemp);
        dwLastHigh = poResGuildBossReward->m_dwDamagePercent + 1;
        dwPreBossId = poResGuildBossReward->m_dwBossId;
        //LOGWARN("Boss Section: ResId<%u> Low<%u> Hight<%u>", stTemp.m_dwResId, stTemp.m_dwLow, stTemp.m_dwHight);
    }

    return true;
    //LOGRUN("LoadBossGameData ok!");
}

bool Guild::_InitPara()
{
    //初始化各种变量
    ResBasicMgr_t& rstResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* pResBasic = rstResBasicMgr.Find(CONSUME_OF_CREATE_GUILD);
    if (pResBasic == NULL)
    {
        LOGERR("Guild Init fail, pResBasic is NULL");
        return false;
    }
    m_wCreateConsume = (uint16_t)(pResBasic->m_para[0]);

    pResBasic = rstResBasicMgr.Find(GUILD_CREATE_LOWEST_LEVEL);
    if (pResBasic==NULL)
    {
        LOGERR("guild init failed, pResBasic is null");
        return false;
    }
    m_wCreateLowestLv = (uint16_t)(pResBasic->m_para[0]);

    pResBasic = rstResBasicMgr.Find(GUILD_FREE_CREATE_LOWEST_LEVEL);
    if (pResBasic==NULL)
    {
        LOGERR("guild init failed, pResBasic is null");
        return false;
    }
    m_wFreeCreateLowestLv = (uint16_t)(pResBasic->m_para[0]);

    pResBasic = rstResBasicMgr.Find(GUILD_JOIN_GUILD_INTERVAL);
    if (pResBasic==NULL)
    {
        LOGERR("guild init failed, pResBasic is null");
        return false;
    }
    m_dwJoinGuildVal = (uint32_t)(pResBasic->m_para[0]);

    pResBasic = rstResBasicMgr.Find(GUILD_QUIT_KEEP_CONTRIBUTE_RATIO);
    if (pResBasic==NULL)
    {
        LOGERR("guild init failed, pResBasic is null");
        return false;
    }
    m_dwQuitKeepRatio = (uint32_t)(pResBasic->m_para[0]);

    pResBasic = rstResBasicMgr.Find(GUILD_MARKET_INIT_ITEM_NUM);
    if (pResBasic==NULL)
    {
        LOGERR("guild init failed, pResBasic is null");
        return false;
    }
    m_bMarketInitNum = (uint32_t)(pResBasic->m_para[0]);

    pResBasic = rstResBasicMgr.Find(GUILD_BOSS_PLAYER_RESET_TIME);
    if (pResBasic==NULL)
    {
        LOGERR("guild init failed, pResBasic is null");
        return false;
    }

    m_iGuildBossFightNumUptHour = (int)pResBasic->m_para[0];

    bzero(&m_stGuildFightApplyList, sizeof(m_stGuildFightApplyList));
    bzero(&m_stGuildFightAgainstInfo, sizeof(m_stGuildFightAgainstInfo));

    pResBasic = rstResBasicMgr.Find(GUILD_GENERAL_HANG);
    if (pResBasic==NULL)
    {
        LOGERR("guild init failed, pResBasic is null");
        return false;
    }

    RESCONSUME* pResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(pResBasic->m_para[2]);
    if (pResConsume==NULL)
    {
        LOGERR("guild init failed, pResBasic is null");
        return false;
    }
    m_bLevelSlotNum = pResConsume->m_dwLvCount;
    pResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(pResBasic->m_para[3]);
    if (pResConsume==NULL)
    {
        LOGERR("guild init failed, pResBasic is null");
        return false;
    }
    m_bVipSlotNum = pResConsume->m_dwLvCount;

	pResBasic = rstResBasicMgr.Find(9501); assert(pResBasic);
	m_iExpeditionServerOpenDay = pResBasic->m_para[0];

	pResBasic = rstResBasicMgr.Find(9502); assert(pResBasic);
	m_iExpeditionMaxFightNum = pResBasic->m_para[0];
	m_iExpeditionRecoverSec = pResBasic->m_para[1];

	m_iExpeditionWinDamage = pResBasic->m_para[3];
	m_iExpeditionLoseDamage = pResBasic->m_para[4];
    return true;
}

bool Guild::Init()
{
    if (!_InitPara())
    {
        LOGERR("Init Para failed");
        return false;
    }

    if (!_InitGuildTask())
    {
        LOGERR("Init GuildTask failed");
        return false;
    }

    //if (!_InitGuildMarket())
    //{
    //    LOGERR("Init GuildTask failed");
    //    return false;
    //}

    if (!_InitGuildDonate())
    {
        LOGERR("Init GuildDonate failed");
        return false;
    }
    if (!_InitGuildBoss())
    {
        LOGERR("Init GuildBoss failed");
        return false;
    }

    return true;
}

void Guild::UpdateGuidContribution(PlayerData* pstData)
{
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
    rstGuildInfo.m_dwGuildContribution = rstGuildInfo.m_dwGuildContribution * m_dwQuitKeepRatio / 100;
}

int Guild::CheckCreateGuild(PlayerData* pstData)
{
    //退出公会后一段时间内不能创建公会
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();

    uint64_t ullTimeStamp = CGameTime::Instance().GetCurrSecond();
    if (ullTimeStamp - rstGuildInfo.m_ullGuildLastQuitTime < m_dwJoinGuildVal)
    {
        return ERR_JUST_QUIT;
    }

    if (rstMajestyInfo.m_wLevel < m_wCreateLowestLv)
    {
       return ERR_NOT_SATISFY_COND;
    }

    //判断资源是否够
    if ((rstMajestyInfo.m_wLevel < m_wFreeCreateLowestLv) && (pstData->GetDiamond() < m_wCreateConsume))
    {
        return ERR_RES_NOT_ENOUGH;
    }

    return ERR_NONE;
}

int Guild::CheckJoinGuild(PlayerData* pstData)
{
    //退出公会后一段时间内不能申请公会
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
    uint64_t ullTimeStamp = CGameTime::Instance().GetCurrSecond();
    if (ullTimeStamp - rstGuildInfo.m_ullGuildLastQuitTime < m_dwJoinGuildVal)
    {
        return ERR_JUST_QUIT;
    }
    return ERR_NONE;
}

void Guild::AfterCreateGuild(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    bzero(&rstSyncItemInfo, sizeof(rstSyncItemInfo));
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

    if (rstMajestyInfo.m_wLevel < m_wFreeCreateLowestLv)
    {
        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -m_wCreateConsume, rstSyncItemInfo, METHOD_GUILD_OP_CREATE);
    }
}

RESGUILDTASK* Guild::GetTaskResByLevel(uint8_t bLevel)
{
    int head = 0;
    int tail = m_TaskNodeCount - 1;
    RESGUILDTASK* pstResGuildTask = NULL;
    while(head <= tail)
    {
        int medium = (head+tail) / 2;
        if (bLevel < m_TaskNodeList[medium].bLevel)
        {
            tail = medium -1;
        }
        else if (bLevel > m_TaskNodeList[medium].bLevel)
        {
            head = medium + 1;
        }
        else
        {
            pstResGuildTask = m_TaskNodeList[medium].pstResGuildTask;
            LOGRUN("Level=%d RESGUILDTASK Level=%d", bLevel, pstResGuildTask->m_bLevel);
            return pstResGuildTask;
        }
    }
    pstResGuildTask = m_TaskNodeList[head -1].pstResGuildTask;
    //LOGRUN("Level=%d RESGUILDTASK Level=%d", bLevel, pstResGuildTask->m_bLevel);
    return pstResGuildTask;
}

void Guild::GenerateLevelIdList(PlayerData* pstData)
{
    DT_ROLE_MAJESTY_INFO & rstMajestyInfo = pstData->GetMajestyInfo();
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();

    RESGUILDTASK* pstResGuildTask = GetTaskResByLevel(rstMajestyInfo.m_wLevel);
    if (pstResGuildTask== NULL)
    {
        LOGERR("GenerateLevelIdList fail, pstResGuildTask is null");
        return;
    }
    if (rstGuildInfo.m_bGuildTaskCount > pstResGuildTask->m_bLevelCount)
    {
        LOGERR("GuildTaskCount is more than m_bLevelCount");
        return;
    }
    //下面的代码主要是生成m个1-n范围内不重复的随机数，来生成随机任务
    //首先我们生成一个[L,R]的一维数组,然后在[L,R]中随机一个下标作为新产生的随机数
    //然后将该随机数与下标为R的元素交换,然后再在[L,R-1]中随机新的随机数
    //以此递归，就可以在短时间内取到想要的随机数列

    /*
    for (int i=0; i<rstGuildInfo.m_bGuildTaskCount; i++)
    {
        int iRight = (pstResGuildTask->m_bLevelCount -1) -i;
        int bRandom = (uint8_t)CFakeRandom::Instance().Random(iRight+1);

        int iPos = RandomList[bRandom];
        RandomList[bRandom] = RandomList[iRight];
        RandomList[iRight] = iPos;

        bzero(&rstGuildInfo.m_astGuildTaskList[i], sizeof(DT_GUILD_TASK_INFO));
        rstGuildInfo.m_astGuildTaskList[i].m_dwLevelId = pstResGuildTask->m_levelIdList[iPos];
    }
    */


    int iGroupNum = 0;
    int iRight = pstResGuildTask->m_szGroupRegion[iGroupNum] - 1 ;
    int iLeft = 0;
    int bRandom = 0;
    int iPos = 0;
    int iGroupSelectNum = 0;
    int iTaskCount = 0;
    while (iTaskCount < rstGuildInfo.m_bGuildTaskCount && iGroupNum < pstResGuildTask->m_bGroupNum)
    {
        bRandom = (uint8_t)CFakeRandom::Instance().Random(iLeft, iRight+1);	//实际随机范围为[iLeft, iRight]
        iPos = RandomList[bRandom];
        RandomList[bRandom] = RandomList[iRight];
        RandomList[iRight] = iPos;
        bzero(&rstGuildInfo.m_astGuildTaskList[iTaskCount], sizeof(DT_GUILD_TASK_INFO));
        rstGuildInfo.m_astGuildTaskList[iTaskCount].m_dwLevelId = pstResGuildTask->m_levelIdList[iPos];
        rstGuildInfo.m_astGuildTaskList[iTaskCount].m_dwRewardId = pstResGuildTask->m_rewardIdList[iPos];//增加一个字段 ,前台需要预览奖励

        iGroupSelectNum++;
        iTaskCount++;
        iRight--;
        if (iGroupSelectNum >= pstResGuildTask->m_szGroupSelect[iGroupNum])
        {//下一组
            iGroupSelectNum = 0;
            iGroupNum++;
            iRight = pstResGuildTask->m_szGroupRegion[iGroupNum] - 1 ;
            iLeft = pstResGuildTask->m_szGroupRegion[iGroupNum - 1];
        }
        LOGRUN("bRandom =%d, iPos=%d, Id=%u PlayerName=%s", bRandom, iPos, pstResGuildTask->m_rewardIdList[iPos], rstMajestyInfo.m_szName);
    }

    return;
}

void Guild::InitMemberExtraInfo(PlayerData* pstData, DT_GUILD_MEMBER_EXTRA_INFO& rstGuildMemberExtraInfo, uint8_t bIsOnline)
{
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
    DT_ROLE_PEAK_ARENA_INFO& rstPeakArenaInfo = pstData->GetELOInfo().m_stPeakArenaInfo;
    DT_ROLE_GCARD_INFO& rstGCardInfo =  pstData->GetGCardInfo();
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();

    rstGuildMemberExtraInfo.m_bIsOnline = bIsOnline;
    rstGuildMemberExtraInfo.m_dwGuildVitality = 0;
	rstGuildMemberExtraInfo.m_wHeadIconId = rstMajestyInfo.m_wIconId;
	rstGuildMemberExtraInfo.m_wHeadFrameId = rstMajestyInfo.m_wFrameId;
	rstGuildMemberExtraInfo.m_wHeadTitleId = rstMajestyInfo.m_wTitleId;
    rstGuildMemberExtraInfo.m_bELOLvId = rstPeakArenaInfo.m_bELOLvId;
    rstGuildMemberExtraInfo.m_dwELOFightNum = rstPeakArenaInfo.m_wLoseCount + rstPeakArenaInfo.m_wWinCount;
    rstGuildMemberExtraInfo.m_wELOFightWinNum = rstPeakArenaInfo.m_wWinCount;
    rstGuildMemberExtraInfo.m_wOwnGeneralCount = rstGCardInfo.m_iCount;
    rstGuildMemberExtraInfo.m_bMajestyLevel = rstMajestyInfo.m_wLevel;
    rstGuildMemberExtraInfo.m_bStarNum = rstPeakArenaInfo.m_dwScore;
    rstGuildMemberExtraInfo.m_dwLi = rstMajestyInfo.m_dwHighestLi;
    rstGuildMemberExtraInfo.m_bGuildBossFightTimes = rstGuildInfo.m_bGuildBossFightNum;
    rstGuildMemberExtraInfo.m_dwGuildContribution = rstGuildInfo.m_dwGuildContribution;
    memcpy(rstGuildMemberExtraInfo.m_szGuildBossAwardStateList, rstGuildInfo.m_szGuildBossAwardStateList,
        sizeof(rstGuildInfo.m_szGuildBossAwardStateList));

	rstGuildMemberExtraInfo.m_bVipLv = rstMajestyInfo.m_bVipLv;
}

void Guild::InitMemberInfo(PlayerData* pstData, DT_ONE_GUILD_MEMBER& rstGuildMemberInfo)
{
    rstGuildMemberInfo.m_ullUin = pstData->GetRoleBaseInfo().m_ullUin;
    StrCpy(rstGuildMemberInfo.m_szName, pstData->GetRoleBaseInfo().m_szRoleName, PKGMETA::MAX_NAME_LENGTH);
    rstGuildMemberInfo.m_bIdentity = GUILD_IDENTITY_MEMBER;
	rstGuildMemberInfo.m_bSalaryIdentityToday = GUILD_IDENTITY_MEMBER;
    rstGuildMemberInfo.m_ullJoinGuildTimeStap = 0;
    rstGuildMemberInfo.m_ullLastLogoutTimeStap = 0;
    rstGuildMemberInfo.m_bKickCount = 0;
    rstGuildMemberInfo.m_dwFundToday = 0;
    rstGuildMemberInfo.m_dwFundTotle = 0;

    rstGuildMemberInfo.m_stHangBeSpeededInfo.m_bCount = 0;
    rstGuildMemberInfo.m_stHangBeSpeededInfo.m_bSpeededToday = 0;
    rstGuildMemberInfo.m_stHangBeSpeededInfo.m_bTotalNum = 0;

    rstGuildMemberInfo.m_stHangSpeedPartnerInfo.m_bCount = 0;

    rstGuildMemberInfo.m_stHangStarInfo.m_bCount = 0;

    this->InitMemberExtraInfo(pstData, rstGuildMemberInfo.m_stExtraInfo, 1);
}

void Guild::RefreshMemberInfo(PlayerData* pstData, uint8_t bIsOnline)
{
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
    DT_ROLE_BASE_INFO& rstBaseInfo = pstData->GetRoleBaseInfo();

    if (rstGuildInfo.m_ullGuildId == 0 || rstBaseInfo.m_ullUin == 0)
    {
        return;
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_REFRESH_MEMBERINFO_NTF;
    m_stSsPkg.m_stHead.m_ullUin = pstData->GetRoleBaseInfo().m_ullUin;

    SS_PKG_REFRESH_MEMBERINFO_NTF& rstSsRefreshMemInfoNtf = m_stSsPkg.m_stBody.m_stRefreshMemInfoNtf;
    rstSsRefreshMemInfoNtf.m_ullUin = rstBaseInfo.m_ullUin;
    rstSsRefreshMemInfoNtf.m_ullGuildId = rstGuildInfo.m_ullGuildId;
	StrCpy(rstSsRefreshMemInfoNtf.m_szName, rstBaseInfo.m_szRoleName, PKGMETA::MAX_NAME_LENGTH);

    this->InitMemberExtraInfo(pstData, rstSsRefreshMemInfoNtf.m_stPlayerInfo, bIsOnline);
    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
}

// 在线更新领取奖励
void Guild::UpdatePlayerData(PlayerData* pstData)
{
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
    if (rstGuildInfo.m_ullGuildTaskUpdateTime < m_ullTaskLastUptTime)
    {
        GenerateLevelIdList(pstData);
        rstGuildInfo.m_ullGuildTaskUpdateTime = m_ullTaskLastUptTime;
        rstGuildInfo.m_bGuildDonateTimes = 0;
        rstGuildInfo.m_bGuildResetTaskCnt = 0;
		rstGuildInfo.m_bGuildSalaryDrawed = 0;
        SendTaskNtfMsg(pstData);
    }
    //if (rstGuildInfo.m_ullGuildMarketUpdateTime < m_ullMarketLastUptTime)
    //{

    //    if (!CGameTime::Instance().IsInSameDay(rstGuildInfo.m_ullGuildMarketUpdateTime, m_ullMarketLastUptTime))
    //    {
    //        //商店刷新次数只在玩家当天第一次刷新商店时重置
    //        rstGuildInfo.m_bGuildResetMarketCnt = 0;
    //    }
    //    RefreshMarket(pstData);
    //    rstGuildInfo.m_ullGuildMarketUpdateTime = m_ullMarketLastUptTime;
    //    SendMarketNtfMsg(pstData);
    //}
    if (rstGuildInfo.m_ullGuildBossFightNumUptTime < m_ullGuildBossFightNumUptTime)
    {
        rstGuildInfo.m_bGuildBossFightNum = 0;
        rstGuildInfo.m_ullGuildBossFightNumUptTime = m_ullGuildBossFightNumUptTime;
        LOGRUN("Reset player<%lu> boss fight times to zero.", pstData->GetRoleBaseInfo().m_ullUin);
    }


	// 远征恢复
	uint64_t ullNow = CGameTime::Instance().GetCurrSecond();
	DT_ROLE_GUILD_EXPEDITION_INFO& rstExpeditionInfo = rstGuildInfo.m_stGuildExpeditionInfo;
	if (rstExpeditionInfo.m_bFightCount != 0
		&& (rstExpeditionInfo.m_ullLastRecoverTime + m_iExpeditionRecoverSec) <= ullNow)
	{
		int iOld = rstExpeditionInfo.m_bFightCount;
		int iRecoverCount = (ullNow - rstExpeditionInfo.m_ullLastRecoverTime) / m_iExpeditionRecoverSec;
		if (iRecoverCount >= rstExpeditionInfo.m_bFightCount)
		{
			rstExpeditionInfo.m_bFightCount = 0;
		}
		else
		{
			rstExpeditionInfo.m_bFightCount = rstGuildInfo.m_stGuildExpeditionInfo.m_bFightCount - iRecoverCount;
		}
		rstExpeditionInfo.m_ullLastRecoverTime = ullNow;
		LOGRUN("Uin<%lu> Expedition fight count recover from iOld<%d> to New<%d>, Time<%lu>", 
			pstData->m_ullUin, iOld, rstExpeditionInfo.m_bFightCount, rstExpeditionInfo.m_ullLastRecoverTime);
	}

    return;
}

int Guild::UpdateServer()
{
    int iHour = CGameTime::Instance().GetCurrHour();
    //每日公会任务重置
    if ((iHour == m_iTaskUptTime) && m_bTaskUptFlag)
    {
        m_bTaskUptFlag = false;
        m_ullTaskLastUptTime = CGameTime::Instance().GetCurrSecond();
    }
    else if ((iHour != m_iTaskUptTime))
    {
        m_bTaskUptFlag = true;
    }


    ////公会商店重置
    //if ((iHour == m_iMarketUptTime1 || iHour == m_iMarketUptTime2 || iHour== m_iMarketUptTime3)  && m_bMarketUptFlag)
    //{
    //    m_bMarketUptFlag = false;
    //    m_ullMarketLastUptTime = CGameTime::Instance().GetCurrSecond();
    //}
    //else if (iHour != m_iMarketUptTime1 && iHour != m_iMarketUptTime2 && iHour != m_iMarketUptTime3)
    //{
    //    m_bMarketUptFlag = true;
    //}

    //公会Boss战斗次数刷新
    if ((iHour == m_iGuildBossFightNumUptHour) && m_bGuildBossFightNumUpteFlag)
    {
        m_bGuildBossFightNumUpteFlag = false;
		m_ullGuildBossFightNumUptTime = CGameTime::Instance().GetCurrSecond();
    }
    else if ((iHour != m_iGuildBossFightNumUptHour))
    {
        m_bGuildBossFightNumUpteFlag = true;
    }


    return 0;
}

//非初始化玩家更新
void Guild::InitPlayerData(PlayerData* pstData)
{
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
    rstGuildInfo.m_bGuildTaskCount = MAX_GUILD_TASK_NUM;

    //更新公会任务
    if (rstGuildInfo.m_ullGuildTaskUpdateTime < m_ullTaskLastUptTime)
    {
       GenerateLevelIdList(pstData);
       rstGuildInfo.m_ullGuildTaskUpdateTime = m_ullTaskLastUptTime;
       rstGuildInfo.m_bGuildDonateTimes = 0;
       rstGuildInfo.m_bGuildResetTaskCnt = 0;
	   rstGuildInfo.m_bGuildSalaryDrawed = 0;
       SendTaskNtfMsg(pstData);
    }

    ////更新公会商店
    //if (rstGuildInfo.m_ullGuildMarketUpdateTime < m_ullMarketLastUptTime)
    //{
    //   //rstGuildInfo.m_ullGuildTaskUpdateTime = m_ullTaskLastUptTime;
    //   if (!CGameTime::Instance().IsInSameDay(rstGuildInfo.m_ullGuildMarketUpdateTime, m_ullMarketLastUptTime))
    //   {
    //       //商店刷新次数只在玩家当天第一次刷新商店时重置
    //       rstGuildInfo.m_bGuildResetMarketCnt = 0;
    //   }
    //   RefreshMarket(pstData);
    //   rstGuildInfo.m_ullGuildMarketUpdateTime = m_ullMarketLastUptTime;
    //   SendMarketNtfMsg(pstData);
    //}

    //更新军团Boss战斗次数
    if (rstGuildInfo.m_ullGuildBossFightNumUptTime < m_ullGuildBossFightNumUptTime)
    {
        rstGuildInfo.m_bGuildBossFightNum = 0;
        rstGuildInfo.m_ullGuildBossFightNumUptTime = m_ullGuildBossFightNumUptTime;
    }

    //更改练兵场栏位个数
    rstGuildInfo.m_stGuildHangInfo.m_bLevelSlotCount = m_bLevelSlotNum < GUILD_GENERAL_HANG_LEVEL_NUM ? m_bLevelSlotNum : GUILD_GENERAL_HANG_LEVEL_NUM;
    rstGuildInfo.m_stGuildHangInfo.m_bVipSlotCount = m_bVipSlotNum < GUILD_GENERAL_HANG_VIP_NUM ? m_bVipSlotNum : GUILD_GENERAL_HANG_VIP_NUM;

    return;
}

//发任务更新通知
void Guild::SendTaskNtfMsg(PlayerData* pstData)
{
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_REFRESH_GUILDTASK_NTF;
    SC_PKG_GUILD_REFRESH_GUILDTASK_NTF& rstScRefreshGuildTaskNtf = m_stScPkg.m_stBody.m_stRefreshGuildTaskNtf;
    rstScRefreshGuildTaskNtf.m_bRefreshTimes = rstGuildInfo.m_bGuildResetTaskCnt;
    rstScRefreshGuildTaskNtf.m_bTaskCount = rstGuildInfo.m_bGuildTaskCount;
    for (int i=0; i<rstScRefreshGuildTaskNtf.m_bTaskCount; i++)
    {
        rstScRefreshGuildTaskNtf.m_astTaskList[i] = rstGuildInfo.m_astGuildTaskList[i];
    }

    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
}

//void Guild::SendMarketNtfMsg(PlayerData* pstData)
//{
//    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
//
//    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_REFRESH_MARKET_NTF;
//    SC_PKG_GUILD_REFRESH_MARKET_NTF& rstScRefreshMarketNtf = m_stScPkg.m_stBody.m_stRefreshGuildMarketNtf;
//
//    rstScRefreshMarketNtf.m_ullUpdateTime = rstGuildInfo.m_ullGuildMarketUpdateTime;
//    rstScRefreshMarketNtf.m_bRefreshTimes = rstGuildInfo.m_bGuildResetMarketCnt;
//    rstScRefreshMarketNtf.m_stMarketInfo.m_bShowItemCount = rstGuildInfo.m_stGuildMarketInfo.m_bShowItemCount;
//    for (int i=0; i<rstScRefreshMarketNtf.m_stMarketInfo.m_bShowItemCount; i++)
//    {
//        rstScRefreshMarketNtf.m_stMarketInfo.m_astShowItemList[i] = rstGuildInfo.m_stGuildMarketInfo.m_astShowItemList[i];
//    }
//
//    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
//}

//添加公会任务奖励
int Guild::SettleGuilTask(PlayerData* pstData, CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsSettle, SC_PKG_FIGHT_PVE_SETTLE_RSP &rSettleRsp)
{
    uint32_t dwPveLevelId = rstCsSettle.m_dwFLevelID;

    //公会任务中找此关卡对应的奖励ID
    m_LevelToRewardMapIter = m_LevelToRewardMap.find(dwPveLevelId);
    if (m_LevelToRewardMapIter == m_LevelToRewardMap.end())
    {
        LOGERR("PveLevel(%u) not found ", dwPveLevelId);
        return ERR_NOT_FOUND;
    }

    //角色身上找此关卡ID
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
    int iPos = -1;
    for (int i=0; i<rstGuildInfo.m_bGuildTaskCount; i++)
    {
        if (rstGuildInfo.m_astGuildTaskList[i].m_dwLevelId == dwPveLevelId)
        {
            iPos = i;
            break;
        }
    }
    if (iPos == -1)
    {
        LOGERR("Role don't have PveLevel(%u)", dwPveLevelId);
        return ERR_NOT_FOUND;
    }

    //保存PVE结果
    // 评星规则变为累计评星
    uint8_t bOldStar = rstGuildInfo.m_astGuildTaskList[iPos].m_bStarCount;
    for (int i=0; i<rstCsSettle.m_bStarEvalResult; i++)
    {
        uint32_t dwStarEvalId = rstCsSettle.m_StarEvalIDList[i];
        int j=0;
        for (; j<rstGuildInfo.m_astGuildTaskList[iPos].m_bStarCount; j++)
        {
            if (rstGuildInfo.m_astGuildTaskList[iPos].m_StarList[j] == dwStarEvalId)
            {
                break;
            }
        }

        if ((j == rstGuildInfo.m_astGuildTaskList[iPos].m_bStarCount) && (j < RES_MAX_PVE_STAR))
        {
            // 拥有新评星
            rstGuildInfo.m_astGuildTaskList[iPos].m_StarList[j] = dwStarEvalId;
            rstGuildInfo.m_astGuildTaskList[iPos].m_bStarCount++;
        }
    }
    uint8_t bStar = rstGuildInfo.m_astGuildTaskList[iPos].m_bStarCount;

    rSettleRsp.m_bStarEvalResult = bStar;
    for (uint8_t k=0; k<bStar; k++)
    {
        rSettleRsp.m_StarEvalIDList[k] = rstGuildInfo.m_astGuildTaskList[iPos].m_StarList[k];
    }

    if (bStar <= bOldStar)
    {
        return ERR_NONE;
    }

    //根据奖励表发奖励
    uint32_t dwId = m_LevelToRewardMapIter->second;
    ResGuildRewardMgr_t& rstResGuildTaskMgr = CGameDataMgr::Instance().GetResGuildRewardMgr();
    RESGUILDREWARD* pResGuildReward = rstResGuildTaskMgr.Find(dwId);
    if (!pResGuildReward )
    {
        LOGERR("pResGuildReward is null");
        return ERR_NOT_FOUND;
    }

    //公会等级数据档
    ResGuildLevelMgr_t& rstResGuildLevelMgr = CGameDataMgr::Instance().GetResGuildLevelMgr();
    RESGUILDLEVEL* pResGuildLevel = rstResGuildLevelMgr.Find(rstGuildInfo.m_bGuildLevel);
    if (!pResGuildLevel )
    {
        LOGERR("pResGuildLevel is null");
        return ERR_NOT_FOUND;
    }
    //RESGUILDREWARD
    int iCount = pResGuildReward->m_bRewardCount;
    for (int j=0; j<iCount; j++)
    {
        if (bOldStar < pResGuildReward->m_szRequireStarNumList[j] && bStar >= pResGuildReward->m_szRequireStarNumList[j])
        {
			uint32_t dwCommonRewardId = pResGuildReward->m_rewardIdList[j];
			RESCOMMONREWARD* pResCommonReward =  CGameDataMgr::Instance().GetResCommonRewardMgr().Find(dwCommonRewardId);
			if (pResCommonReward == NULL)
			{
				LOGERR("pResCommonReward is null");
				return ERR_NONE;
			}

			for (int k=0; k<pResCommonReward->m_bNum; k++)
			{
				if (pResCommonReward->m_propsNum[k] > 0)
				{
					Item::Instance().RewardItem(pstData, pResCommonReward->m_szPropstype[k], pResCommonReward->m_propsId[k], pResCommonReward->m_propsNum[k], rSettleRsp.m_stSyncItemInfo, METHOD_GUILD_TASK_PVE_SETTLE);
				}
			}
        }
    }

	//军团关卡
	do
	{
		if (bStar < 3)
		{
			break;
		}

		m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_LEVEL_EVENT_NTF;
		m_stSsPkg.m_stHead.m_ullUin = pstData->GetRoleBaseInfo().m_ullUin;
		SS_PKG_GUILD_LEVEL_EVENT_NTF& rstNtf = m_stSsPkg.m_stBody.m_stGuildLevelEventNtf;
		rstNtf.m_dwLevelId = dwPveLevelId;
		rstNtf.m_bType = CHAPTER_TYPE_GUILD;
		rstNtf.m_dwValue = bStar;
		rstNtf.m_ullGuildId = rstGuildInfo.m_ullGuildId;
		ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
	} while (false);

    //公会日志
    ZoneLog::Instance().WriteGuildLog(pstData, METHOD_GUILD_TASK_PVE_SETTLE, pstData->GetGuildInfo().m_ullGuildId, NULL, 0, rstCsSettle.m_dwFLevelID);
    return ERR_NONE;
}

int Guild::SettleBoss(PlayerData* pstData, CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsSettle, SC_PKG_FIGHT_PVE_SETTLE_RSP& rSettleRsp)
{
    rSettleRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    //结算前再检查一次
    if (ERR_NONE != CheckFightBoss(pstData, rstCsSettle.m_dwFLevelID))
    {
        return ERR_NOT_SATISFY_COND;
    }
    uint32_t dwBossId = m_FLevelToBossIdMap[rstCsSettle.m_dwFLevelID];

    if (0 == dwBossId || dwBossId > MAX_GUILD_BOSS_NUM)
    {
        LOGERR("Uin<%lu> BossId<%u> or FLevelId<%u>  error", pstData->m_ullUin, dwBossId, rstCsSettle.m_dwFLevelID);
        return ERR_SYS;
    }
    uint32_t dwBossMaxHp = m_BossHp[dwBossId-1];
    uint64_t ullDamageRate = ((uint64_t)rstCsSettle.m_dwDamageInBoss) * 10000 / dwBossMaxHp;
    if (ullDamageRate >= 10000)
    {
        ullDamageRate = 10000;
    }
    LOGRUN("BossId<%d>, MaxHp<%u>, DamageRate<%lu>", dwBossId, dwBossMaxHp, ullDamageRate);



    // 血量奖励
    list<BossSection>::iterator iter = find_if(m_oBossRewardMap[dwBossId - 1].begin(), m_oBossRewardMap[dwBossId - 1].end(), BossSectionCmp(ullDamageRate));
    if (iter == m_oBossRewardMap[dwBossId - 1].end())
    {
        LOGERR("Uin<%lu> BossId<%u> FLevelId<%u> ullDamageRate<%lu> find the DamagePercent error", pstData->m_ullUin, dwBossId, rstCsSettle.m_dwFLevelID, ullDamageRate);
        return ERR_SYS;
    }
    RESGUILDBOSSREWARD* poResGuildBossReward = CGameDataMgr::Instance().GetResGuildBossRewardMgr().Find(iter->m_dwResId);
    if (!poResGuildBossReward)
    {
        LOGERR("Uin<%lu> BossId<%u> FLevelId<%u>  the RewardId<%u> is NULL", pstData->m_ullUin, dwBossId, rstCsSettle.m_dwFLevelID, iter->m_dwResId);
        return ERR_SYS;
    }
	//消耗
	pstData->GetGuildInfo().m_bGuildBossFightNum++;
	//参加军团Boss
	Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_GUILD_OTHER, 1, 8, 1);
    size_t sRewardNum = sizeof(poResGuildBossReward->m_bonusId) / sizeof(poResGuildBossReward->m_bonusId[0]);
    for (size_t i = 0; i < sRewardNum; i++)
    {
        if (poResGuildBossReward->m_bonusId[i] == 0)
        {
            continue;
        }
        Lottery::Instance().DrawLotteryByPond(pstData, poResGuildBossReward->m_bonusId[i], rSettleRsp.m_stSyncItemInfo, METHOD_GUILD_OP_GET_BOSS_FIGHT_AWARD);
    }

    Item::Instance().RewardItem(pstData, ITEM_TYPE_GUILD_VITALITY, 0, poResGuildBossReward->m_dwVitality, rSettleRsp.m_stSyncItemInfo, METHOD_GUILD_OP_GET_BOSS_FIGHT_AWARD);

    //公会BOSS扣血
    bzero(&m_stSsPkg, sizeof(m_stSsPkg));
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_BOSS_FIGHT_SETTLE_NTF;
    m_stSsPkg.m_stHead.m_ullUin = pstData->GetRoleBaseInfo().m_ullUin;
    SS_PKG_GUILD_BOSS_FIGHT_SETTLE_NTF& rstSsGuildBossFightSettleNtf = m_stSsPkg.m_stBody.m_stGuildBossFightSettleNtf;
    rstSsGuildBossFightSettleNtf.m_ullGuildId = pstData->GetGuildInfo().m_ullGuildId;
    rstSsGuildBossFightSettleNtf.m_dwDamageHp = rstCsSettle.m_dwDamageInBoss;
    rstSsGuildBossFightSettleNtf.m_dwFigheLevelId = rstCsSettle.m_dwFLevelID;
    LOGRUN("DamageHp:<%u>", rstSsGuildBossFightSettleNtf.m_dwDamageHp);
    ZoneLog::Instance().WriteGuildBossLog(pstData, rstCsSettle.m_dwDamageInBoss, dwBossId);
    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);

    return ERR_NONE;
}

int Guild::AddGuildContribution(PlayerData* pstData, int iNum, uint32_t dwApproach)
{
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
    uint8_t bChgType = 0;
	uint32_t dwChgValue = 0;
    if (iNum >= 0)
    {
        rstGuildInfo.m_dwGuildContribution += iNum;
        bChgType = 1;
		dwChgValue = iNum;
    }
    else
    {
        uint32_t dwMinusNum = (uint32_t)(-iNum);
        bChgType = 2;
        if (rstGuildInfo.m_dwGuildContribution > dwMinusNum)
        {
            rstGuildInfo.m_dwGuildContribution -= dwMinusNum;
        }
        else
        {
            rstGuildInfo.m_dwGuildContribution = 0;
        }
		dwChgValue = dwMinusNum;
    }

    //公会贡献点变化后需及时刷新到公会
    this->RefreshMemberInfo(pstData);
    ZoneLog::Instance().WriteCoinLog(pstData, dwChgValue, ITEM_TYPE_GUILD_CONTRIBUTION, bChgType, pstData->GetGuildInfo().m_dwGuildContribution, dwApproach);
    if (dwApproach == 0)
    {
        LOGERR("Approach = 0 in AddGuildContribution");
    }
    return rstGuildInfo.m_dwGuildContribution;
}

int Guild::AddGuidFund(PlayerData* pstData, int iNum, uint8_t bType, uint8_t bDonateType)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_UPDATE_FUND_NTF;
    m_stSsPkg.m_stHead.m_ullUin = pstData->GetRoleBaseInfo().m_ullUin;
    SS_PKG_GUILD_UPDATE_FUND_NTF& rstSsGuildUpdateFundNtf = m_stSsPkg.m_stBody.m_stGuildUpdateFundNtf;
    rstSsGuildUpdateFundNtf.m_iGuildFund = iNum;
	rstSsGuildUpdateFundNtf.m_bType = bType;
	rstSsGuildUpdateFundNtf.m_bDonateType = bDonateType;

    rstSsGuildUpdateFundNtf.m_ullGuildId = pstData->GetGuildInfo().m_ullGuildId;
    if (rstSsGuildUpdateFundNtf.m_ullGuildId == 0)
    {
        LOGERR("Player(%lu) AddGuidFund failed, because GuildId is 0", pstData->GetRoleBaseInfo().m_ullUin);
        return ERR_SYS;
    }

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return ERR_NONE;
}

int Guild::AddGuildVitality(PlayerData* pstData, int iNum, uint8_t dwApproach)
{
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
    DT_ROLE_BASE_INFO& rstBaseInfo = pstData->GetRoleBaseInfo();

    if (rstGuildInfo.m_ullGuildId == 0 || rstBaseInfo.m_ullUin == 0)
    {
        return 0;
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_REFRESH_MEMBERINFO_NTF;
    m_stSsPkg.m_stHead.m_ullUin = pstData->GetRoleBaseInfo().m_ullUin;

    SS_PKG_REFRESH_MEMBERINFO_NTF& rstSsRefreshMemInfoNtf = m_stSsPkg.m_stBody.m_stRefreshMemInfoNtf;
    rstSsRefreshMemInfoNtf.m_ullUin = rstBaseInfo.m_ullUin;
    rstSsRefreshMemInfoNtf.m_ullGuildId = rstGuildInfo.m_ullGuildId;
	StrCpy(rstSsRefreshMemInfoNtf.m_szName, rstBaseInfo.m_szRoleName, PKGMETA::MAX_NAME_LENGTH);
    rstSsRefreshMemInfoNtf.m_stPlayerInfo.m_szGuildBossAwardStateList[0] = 0;

    rstSsRefreshMemInfoNtf.m_stPlayerInfo.m_dwGuildVitality = iNum;

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);

    return iNum;
}

int Guild::Donate(PlayerData* pstData, CS_PKG_GUILD_DONATE_REQ& rstCsPkgReq, SC_PKG_GUILD_DONATE_RSP& rstScPkgRsp)
{
    rstScPkgRsp.m_stSyncItems.m_bSyncItemCount = 0;

    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
	if (rstGuildInfo.m_ullGuildId == 0)
	{
		return ERR_DEFAULT;
	}

    if (rstGuildInfo.m_bGuildDonateTimes >= m_iDonateMaxTimes)
    {
        return ERR_NOT_ENOUGH_TIMES;
    }

	RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(pstData->GetVIPLv()+1);
	if(3==rstCsPkgReq.m_bDonateType && !pResVip->m_bIsGuildShopOpen)
	{
		return ERR_DEFAULT;
	}

	if (rstCsPkgReq.m_bDonateType == 3)
	{
		return _VipDonate(pstData, rstScPkgRsp.m_stSyncItems, rstCsPkgReq.m_bTargetType);
	}
    if (rstCsPkgReq.m_bDonateType == 2)
    {
        return _DiamandDonate(pstData, rstScPkgRsp.m_stSyncItems, rstCsPkgReq.m_bTargetType);
    }

    if (rstCsPkgReq.m_bDonateType == 1)
    {
        return _GoldDonate(pstData, rstScPkgRsp.m_stSyncItems, rstCsPkgReq.m_bTargetType);
    }

    return ERR_NONE;
}

int Guild::_GoldDonate(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncInfo, uint8_t bType)
{
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();

    if (!Consume::Instance().IsEnough(pstData, ITEM_TYPE_GOLD, m_iGoldDonateConsume))
    {
        return ERR_RES_NOT_ENOUGH;
    }

    int iRet = ERR_NONE;
    do
    {
        int iRet = this->AddGuidFund(pstData, m_iGoldDonateFund, bType, 1);
        if (iRet != ERR_NONE)
        {
            LOGERR("Add GoldDonate failed. Type<%d>", bType);
            break;
        }

        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_GOLD, 0, -m_iGoldDonateConsume, rstSyncInfo, METHOD_GUILD_OP_DONATE_GOLD);
        Item::Instance().RewardItem(pstData, ITEM_TYPE_GUILD_CONTRIBUTION, 0, m_iGoldDonateContribute, rstSyncInfo, METHOD_GUILD_OP_DONATE_GOLD);
        Item::Instance().RewardItem(pstData, ITEM_TYPE_GUILD_VITALITY, 0, m_iGoldDonateVitality, rstSyncInfo, METHOD_GUILD_OP_DONATE_GOLD);

        rstGuildInfo.m_bGuildDonateTimes++;
    }while(false);

    return iRet;
}

int Guild::_DiamandDonate(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncInfo, uint8_t bType)
{
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();

    if (!Consume::Instance().IsEnough(pstData, ITEM_TYPE_DIAMOND, m_iDiamandDonateConsume))
    {
        return ERR_RES_NOT_ENOUGH;
    }

    int iRet = ERR_NONE;
    do
    {
        int iRet = this->AddGuidFund(pstData, m_iDiamandDonateFund, bType, 2);
        if (iRet != ERR_NONE)
        {
            LOGERR("Add DiamondDonate failed. Type<%d>", bType);
            break;
        }

        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -m_iDiamandDonateConsume, rstSyncInfo, METHOD_GUILD_OP_DONATE_DIAMAND);
        Item::Instance().RewardItem(pstData, ITEM_TYPE_GUILD_CONTRIBUTION, 0, m_iDiamandDonateContribute, rstSyncInfo, METHOD_GUILD_OP_DONATE_DIAMAND);
        Item::Instance().RewardItem(pstData, ITEM_TYPE_GUILD_VITALITY, 0, m_iDiamandDonateVitality, rstSyncInfo, METHOD_GUILD_OP_DONATE_DIAMAND);

        rstGuildInfo.m_bGuildDonateTimes++;
    }while(false);

    return iRet;
}

int Guild::_VipDonate(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncInfo, uint8_t bType)
{
	DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();

	if (!Consume::Instance().IsEnough(pstData, ITEM_TYPE_DIAMOND, m_iVipDonateConsume))
	{
		return ERR_RES_NOT_ENOUGH;
	}

    int iRet = ERR_NONE;
    do
    {
        int iRet = this->AddGuidFund(pstData, m_iVipDonateFund, bType, 3);
        if (iRet != ERR_NONE)
        {
            LOGERR("Add DiamondDonate failed. Type<%d>", bType);
            break;
        }

        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -m_iVipDonateConsume, rstSyncInfo, METHOD_GUILD_OP_VIP_DONATE);
        Item::Instance().RewardItem(pstData, ITEM_TYPE_GUILD_CONTRIBUTION, 0, m_iVipDonateContribute, rstSyncInfo, METHOD_GUILD_OP_VIP_DONATE);
        Item::Instance().RewardItem(pstData, ITEM_TYPE_GUILD_VITALITY, 0, m_iVipDonateVitality, rstSyncInfo, METHOD_GUILD_OP_VIP_DONATE);

        rstGuildInfo.m_bGuildDonateTimes++;
    }while(false);

    return iRet;
}

void Guild::GetGuildFightApplyList(uint64_t ullGuildId, SC_PKG_GUILD_FIGHT_GET_APPLY_LIST_RSP& rstGetApplyListRsp)
{
    rstGetApplyListRsp.m_nErrNo = ERR_NONE;
    rstGetApplyListRsp.m_bCount = m_stGuildFightApplyList.m_bCount;
    memcpy(rstGetApplyListRsp.m_astApplyList, m_stGuildFightApplyList.m_astApplyList, sizeof(rstGetApplyListRsp.m_astApplyList));
    memcpy(&rstGetApplyListRsp.m_stLastWinner, &m_stGuildFightApplyList.m_stLastWinner, sizeof(DT_GUILD_FIGHT_APPLY_INFO));
}

void Guild::SetGuildFightApplyList(DT_GUILD_FIGHT_APPLY_LIST_INFO& rstGuildFightApplyList)
{
    memcpy(&m_stGuildFightApplyList, &rstGuildFightApplyList, sizeof(m_stGuildFightApplyList));
}

void Guild::GetGuildFightAgainstInfo(DT_GUILD_FIGHT_AGAINST_INFO& rstGuildFightAgainstInfo)
{
    memcpy(&rstGuildFightAgainstInfo, &m_stGuildFightAgainstInfo, sizeof(m_stGuildFightAgainstInfo));
}

void Guild::SetGuildFightAgainstInfo(DT_GUILD_FIGHT_AGAINST_INFO& rstGuildFightAgainstInfo)
{
    memcpy(&m_stGuildFightAgainstInfo, &rstGuildFightAgainstInfo, sizeof(m_stGuildFightAgainstInfo));
}

//刷新公会商店或任务
int Guild::ResetMarketOrTask(PlayerData* pstData, CS_PKG_GUILD_RESET_MARKET_OR_TASK_REQ& rstReq, SC_PKG_GUILD_RESET_MARKET_OR_TASK_RSP& rstRsp)
{
    int iRet = ERR_NONE;
    switch (rstReq.m_bType)
    {
    //case 1:
    //    iRet = _ResetMarket(pstData, rstReq, rstRsp);
    //    break;
    case 2:
        iRet = _ResetTask(pstData, rstReq, rstRsp);
        break;
    default:
        iRet = ERR_WRONG_PARA;
        break;
    }
    return iRet;
}

//刷新次数消耗表key
#define CONSUME_RESET_TASK 904
int Guild::_ResetTask(PlayerData* pstData, CS_PKG_GUILD_RESET_MARKET_OR_TASK_REQ& rstReq, SC_PKG_GUILD_RESET_MARKET_OR_TASK_RSP& rstRsp)
{
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
    RESCONSUME* pResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(CONSUME_RESET_TASK);
    if (NULL == pResConsume)
    {
        LOGERR("RESCONSUME not find  Uin<%lu>",pstData->m_pOwner->GetUin());
        return ERR_NOT_FOUND;
    }

    //刷新次数根据VIP等级变化
    RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(pstData->GetVIPLv()+1);
    if (rstGuildInfo.m_bGuildResetTaskCnt >= pResVip->m_dwUpdateGuildTask)
    {
        return ERR_PVE_RESET_TIMES_NOT_ENOUGH;
    }

    uint32_t dwConsumeDia = pResConsume->m_lvList[rstGuildInfo.m_bGuildResetTaskCnt];
    if (!Consume::Instance().IsEnoughDiamond(pstData, dwConsumeDia))
    {//钻石不够
        return ERR_NOT_ENOUGH_DIAMOND;
    }

    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -dwConsumeDia, rstRsp.m_stSyncItemInfo, DWLOG::METHOD_GUILD_OP_RESET_GUILD_TASK);
    rstGuildInfo.m_bGuildResetTaskCnt ++ ;
    GenerateLevelIdList(pstData);
    SendTaskNtfMsg(pstData);
    rstRsp.m_bType = rstReq.m_bType;
    LOGRUN("Player<%lu> reset GuildTask, consume dia<%u>, GuildResetTaskCnt<%u> ", pstData->m_pOwner->GetUin(), dwConsumeDia, rstGuildInfo.m_bGuildResetTaskCnt);
    return ERR_NONE;

}

////刷新次数消耗表key
//#define CONSUME_RESET_MARKET 905
//int Guild::_ResetMarket(PlayerData* pstData, CS_PKG_GUILD_RESET_MARKET_OR_TASK_REQ& rstReq, SC_PKG_GUILD_RESET_MARKET_OR_TASK_RSP& rstRsp)
//{
//    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
//    RESCONSUME* pResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(CONSUME_RESET_MARKET);
//    if (NULL == pResConsume)
//    {
//        LOGERR("RESCONSUME not find  Uin<%lu>",pstData->m_pOwner->GetUin());
//        return ERR_NOT_FOUND;
//    }
//
//    //刷新次数根据VIP等级变化
//    RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(pstData->GetVIPLv()+1);
//    if (rstGuildInfo.m_bGuildResetMarketCnt >= pResVip->m_dwUpdateGuildShop)
//    {
//        return ERR_PVE_RESET_TIMES_NOT_ENOUGH;
//    }
//    uint32_t dwConsumeDia = pResConsume->m_lvList[rstGuildInfo.m_bGuildResetMarketCnt];
//    if (!Consume::Instance().IsEnoughDiamond(pstData, dwConsumeDia))
//    {//钻石不够
//        return ERR_NOT_ENOUGH_DIAMOND;
//    }
//    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
//    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -dwConsumeDia, rstRsp.m_stSyncItemInfo, DWLOG::METHOD_GUILD_OP_RESET_GUILD_MARKET);
//    rstGuildInfo.m_bGuildResetMarketCnt++ ;
//    RefreshMarket(pstData);
//    SendMarketNtfMsg(pstData);
//    rstRsp.m_bType = rstReq.m_bType;
//    LOGRUN("Player<%lu> reset GuildMarket, consume dia<%u>, GuildResetTaskCnt<%u> ", pstData->m_pOwner->GetUin(), dwConsumeDia, rstGuildInfo.m_bGuildResetMarketCnt);
//    return ERR_NONE;
//}

int Guild::ActivityRecvReward(PlayerData* pstData, uint32_t dwId, SC_PKG_GUILD_ACTIVITY_REWARD_RSP& rstScPkgRsp)
{
    if (dwId==0 || dwId >=32)
    {
        return ERR_SYS;
    }

    time_t ullCurTime = CGameTime::Instance().GetCurrSecond();

    DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();
    if (CGameTime::Instance().IsDayAlternate(ullCurTime, (time_t)rstMiscInfo.m_ullGuildActivityTimestamp))
    {
        //跨天则清空
        rstMiscInfo.m_dwGuildActivityRecvList = 0;
    }

    RESGUILDACTIVITY* pResGuildActivity = CGameDataMgr::Instance().GetResGuildActivityMgr().Find(dwId);
    if (pResGuildActivity==NULL)
    {
        return ERR_SYS;
    }

    int iCurHour = CGameTime::Instance().GetCurrHour();
    int iCurMin = CGameTime::Instance().GetCurrMin();

    if ((iCurHour < pResGuildActivity->m_bBeginHour) ||(iCurHour > pResGuildActivity->m_bEndHour) ||
       (iCurHour == pResGuildActivity->m_bBeginHour && iCurMin <pResGuildActivity->m_bBeginMinute) ||
       (iCurHour == pResGuildActivity->m_bEndHour && iCurMin > pResGuildActivity->m_bEndinMinute))
    {
        return ERR_NOT_SATISFY_COND;
    }

    uint32_t dwFlag = 1 << (dwId -1);
    if ((rstMiscInfo.m_dwGuildActivityRecvList & dwFlag) > 0)
    {
        return ERR_NOT_SATISFY_COND;
    }

    for (uint8_t i=0; i<pResGuildActivity->m_bLotteryPoolCount; i++)
    {
        Lottery::Instance().DrawLotteryByPond(pstData, pResGuildActivity->m_lotteryPoolList[i], rstScPkgRsp.m_stSyncItemInfo, METHOD_GUILD_ACTIVITY_REWARD);
    }

    rstMiscInfo.m_dwGuildActivityRecvList = rstMiscInfo.m_dwGuildActivityRecvList | dwFlag;
    rstMiscInfo.m_ullGuildActivityTimestamp = (uint64_t)ullCurTime;
    //记录领取军饷log
    ZoneLog::Instance().WriteGuildLog(pstData, METHOD_GUILD_OP_ACTIVITY_REWARD, pstData->GetGuildInfo().m_ullGuildId, NULL, 0, 0);

    return ERR_NONE;
}

void Guild::GuildBossSettleUpdate(PlayerData* pstData, DT_GUILD_PKG_UPDATE_GUILD_BOSS& rstUpdateGuildBoss)
{
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();

    if (rstGuildInfo.m_ullGuildId == 0 || rstGuildInfo.m_bGuildIdentify == 0)
    {
        LOGERR("Uin<%lu> GuildBossSettleUpdate error! RoleGuildInfo not the upt!GuildId<%lu>, Identify<%hhu> ", pstData->m_ullUin, rstGuildInfo.m_ullGuildId, rstGuildInfo.m_bGuildIdentify);
        return;
    }

    uint32_t dwBossIndex =rstUpdateGuildBoss.m_dwKilledBossId-1;
    if (rstUpdateGuildBoss.m_bKilled)
    {// Boss被击杀,存储奖励ID
        RESGUILDBOSSINFO* poResGuildBossInfo = CGameDataMgr::Instance().GetResGuildBossInfoMgr().GetResByPos(dwBossIndex);
        if (!poResGuildBossInfo)
        {
            LOGERR("poResGuildBossInfo is NULL id<%u>", rstUpdateGuildBoss.m_dwKilledBossId);
            return;
        }
        rstGuildInfo.m_szGuildBossAwardStateList[dwBossIndex] = COMMON_AWARD_STATE_AVAILABLE;
    }
    pstData->m_bBossResetNum = rstUpdateGuildBoss.m_stBossInfo.m_bResetNum;
    SendBossNtfMsg(pstData, rstUpdateGuildBoss.m_stBossInfo);
}

void Guild::SendBossNtfMsg(PlayerData* pstData, DT_GUILD_BOSS_INFO& rstBossInfo)
{
    bzero(&m_stScPkg, sizeof(m_stScPkg));
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_BOSS_UPDATE_INFO_NTF;
    SC_PKG_GUILD_BOSS_UPDATE_INFO_NTF& rstScNtf = m_stScPkg.m_stBody.m_stGuildBossUpdateInfoNtf;
    rstScNtf.m_nErrNo = ERR_NONE;
    memcpy(rstScNtf.m_AwardList, rstGuildInfo.m_GuildBossAwardList, sizeof(rstScNtf.m_AwardList));
    memcpy(rstScNtf.m_szGuildBossAwardStateList, rstGuildInfo.m_szGuildBossAwardStateList, sizeof(rstScNtf.m_szGuildBossAwardStateList));
    rstScNtf.m_stBossInfo = rstBossInfo;
    rstScNtf.m_bBossFightNum = rstGuildInfo.m_bGuildBossFightNum;
    rstScNtf.m_ullBossFightNumUptTime = rstGuildInfo.m_ullGuildBossFightNumUptTime;
    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
}

void Guild::GuildBossReset(PlayerData* pstData)
{
    bzero(&m_stScPkg, sizeof(m_stScPkg));
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_BOSS_RESET_RSP;
    SC_PKG_GUILD_BOSS_RESET_RSP& rstScRsp = m_stScPkg.m_stBody.m_stGuildBossResetRsp;
    rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    int iRet = ERR_NONE;
    do
    {
        //@TODO:检查扣除消耗
        if (rstGuildInfo.m_ullGuildId == 0 )
        {
            LOGERR("<%lu> Not in the guild", pstData->m_ullUin);
            iRet = ERR_NOT_HAVE_GUILD;
            break;
        }
        if (rstGuildInfo.m_bGuildIdentify != GUILD_IDENTITY_LEADER)
        {
            iRet = ERR_GUILD_PERMISSION_DENIED;
            LOGERR("<%lu> not the guild leader GuildID<%lu> Job<%hhu>", pstData->m_ullUin, rstGuildInfo.m_ullGuildId,
                rstGuildInfo.m_bGuildIdentify);
            break;
        }
        RESBASIC *poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(GUILD_BOSS_RESET_PARA);
        RESBASIC *poResBasicConsume = CGameDataMgr::Instance().GetResBasicMgr().Find(GUILD_BOSS_RESET_CONSUME);
        if (!poResBasic || !poResBasicConsume)
        {
            LOGERR("Uin<%lu> the BasicId<%u> or<%u> is NULL", pstData->m_ullUin, GUILD_BOSS_RESET_PARA, GUILD_BOSS_RESET_CONSUME);
            iRet = ERR_SYS ;
            break;
        }
        RESCONSUME *poResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find((uint32_t)poResBasic->m_para[0]);
        RESCONSUME *poResConsumeNumLimit = CGameDataMgr::Instance().GetResConsumeMgr().Find((uint32_t)poResBasic->m_para[1]);

        if (!poResConsume || !poResConsumeNumLimit)
        {
            LOGERR("Uin<%lu> the poResConsume<%f> or poResConsumeLimit<%f> is NULL", pstData->m_ullUin, poResBasic->m_para[0], poResBasic->m_para[1]);
            iRet = ERR_SYS ;
            break;
        }
        if ( !(pstData->m_bBossResetNum < poResConsumeNumLimit->m_lvList[rstGuildInfo.m_bGuildLevel-1] &&
            Item::Instance().IsEnough(pstData, poResBasicConsume->m_para[0], poResBasicConsume->m_para[1], poResConsume->m_lvList[pstData->m_bBossResetNum])))
        {
            LOGERR("Uin<%lu> not satify the condi, the ResetNum<%hhu> ", pstData->m_ullUin, pstData->m_bBossResetNum);
            iRet = ERR_NOT_SATISFY_COND;
            break;
        }
        //消耗
        Item::Instance().RewardItem(pstData, poResBasicConsume->m_para[0], poResBasicConsume->m_para[1], poResConsume->m_lvList[pstData->m_bBossResetNum],
             rstScRsp.m_stSyncItemInfo, METHOD_GUILD_OP_GET_BOSS_RESET);
        pstData->m_bBossResetNum++; //这里++ 是防止在返回前多次操作扣除消耗
    } while (0);
    rstScRsp.m_nErrNo = iRet;
    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);

    if (ERR_NONE == iRet)
    {
        bzero(&m_stSsPkg, sizeof(m_stSsPkg));
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_BOSS_RESET_REQ;
        m_stSsPkg.m_stHead.m_ullUin = pstData->m_ullUin;
        m_stSsPkg.m_stBody.m_stGuildBossResetReq.m_ullGuildId = pstData->GetGuildInfo().m_ullGuildId;
        ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
        LOGRUN("Uin<%lu> reset the GuildBoss Gid<%lu> ", pstData->m_ullUin, pstData->GetGuildInfo().m_ullGuildId);
    }
}

bool Guild::CheckBossFightNum(PlayerData* pstData, uint8_t bNum)
{

    RESBASIC* poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(GUILD_BOSS_PLAYER_RESET_PARA);
    if (!poResBasic)
    {
        LOGERR("Uin<%lu> The poResBasic is NULL id<%u>", pstData->m_ullUin, GUILD_BOSS_PLAYER_RESET_PARA);
        return false;
    }
    if ((bNum + pstData->GetGuildInfo().m_bGuildBossFightNum) <= (uint8_t)poResBasic->m_para[0])
    {
        return true;
    }
    LOGERR("Uin<%lu> GuildBoss fight num limit Add<%hhu> Own<%hhu> ", pstData->m_ullUin, bNum, pstData->GetGuildInfo().m_bGuildBossFightNum);
    return false;
}

void Guild::GuildBossGetKilledAward(PlayerData* pstData)
{
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_BOSS_GET_KILLED_AWARD_RSP;
    SC_PKG_GUILD_BOSS_GET_KILLED_AWARD_RSP& rstRsp = m_stScPkg.m_stBody.m_stGuildBossGetKilledAwardRsp;
    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    int iRet = ERR_NONE;
    do
    {
        for (int iBossIndex = 0; iBossIndex != MAX_GUILD_BOSS_NUM; ++iBossIndex)
        {
            RESGUILDBOSSINFO* resGuildBossInfo = CGameDataMgr::Instance().GetResGuildBossInfoMgr().GetResByPos(iBossIndex);
            //dwBossIndex索引到的Boss为空
            if (resGuildBossInfo == NULL)
            {
                LOGWARN("Boss in %d is NULL", iBossIndex);
                break;
            }
            //击杀奖励可以领取
            if (rstGuildInfo.m_szGuildBossAwardStateList[iBossIndex] == COMMON_AWARD_STATE_AVAILABLE)
            {
                //首杀奖励可领取
                if ((rstGuildInfo.m_ullGuildBossFirstAwardMap & ((uint64_t)1 << iBossIndex)) == 0)
                {
                    uint8_t bFirstKillRwdTypeNum = resGuildBossInfo->m_bFirstKillRwdTypeNum;
                    for (int i = 0; i != bFirstKillRwdTypeNum; ++i)
                    {
                        Item::Instance().RewardItem(
                            pstData,
                            resGuildBossInfo->m_szFirstKillRwdType[i],
                            resGuildBossInfo->m_firstKillRewardId[i],
                            resGuildBossInfo->m_firstKillRwdNum[i],
                            rstRsp.m_stSyncItemInfo,
                            METHOD_GUILD_OP_GET_BOSS_KILLED_AWARD);
                    }
                    rstGuildInfo.m_szGuildBossAwardStateList[iBossIndex] = COMMON_AWARD_STATE_DRAWED;
                    rstGuildInfo.m_ullGuildBossFirstAwardMap |= ((uint64_t)1 << iBossIndex);
                }
                //首杀奖励已领取过
                else
                {
                    uint8_t bKillRwdTypeNum = resGuildBossInfo->m_bKillRwdTypeNum;
                    for (int i = 0; i != bKillRwdTypeNum; ++i)
                    {
                        Item::Instance().RewardItem(
                            pstData,
                            resGuildBossInfo->m_szKillRwdType[i],
                            resGuildBossInfo->m_killRewardId[i],
                            resGuildBossInfo->m_killRwdNum[i],
                            rstRsp.m_stSyncItemInfo,
                            METHOD_GUILD_OP_GET_BOSS_KILLED_AWARD);
                    }
                }
                rstGuildInfo.m_szGuildBossAwardStateList[iBossIndex] = COMMON_AWARD_STATE_DRAWED;
            }
        }
    } while (0);

    rstRsp.m_nErrNo = iRet;
    memcpy(rstRsp.m_szGuildBossAwardStateList, rstGuildInfo.m_szGuildBossAwardStateList,
        sizeof(rstGuildInfo.m_szGuildBossAwardStateList));
    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
}

int Guild::CheckFightBoss(PlayerData* pstData, uint32_t m_dwLevelId)
{
    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
    int iRet = ERR_NONE;
    do
    {
        if (0 == rstGuildInfo.m_ullGuildId)
        {
            LOGERR("Uin<%lu> fight guild boss error! not in the guild ", pstData->m_ullUin);
             iRet = ERR_NOT_HAVE_GUILD;
             break;
        }
        if (!CheckBossFightNum(pstData, 1))
        {
            iRet = ERR_NOT_SATISFY_COND;
            break;
        }
    } while (0);
    return iRet;
}


int Guild::ExpeditionFightRequest(PlayerData* pstData)
{
	if (!IsExpeditionOpen())
	{
		return ERR_MODULE_NOT_OPEN;
	}
	if (pstData->GetGuildId() == 0)
	{
		LOGERR("Uin<%lu> the guild id is 0", pstData->m_ullUin);
		return ERR_NOT_HAVE_GUILD;
	}
	if (pstData->GetGuildInfo().m_stGuildExpeditionInfo.m_bFightCount >= m_iExpeditionMaxFightNum)
	{
		LOGERR("Uin<%lu> fight num<%d> limit", pstData->m_ullUin, (int)pstData->GetGuildInfo().m_stGuildExpeditionInfo.m_bFightCount);
		return ERR_NOT_ENOUGH_TIMES;
	}
	return ERR_NONE;
}

int Guild::SetExpeditionArray(PlayerData* pstData , CS_PKG_GUILD_EXPEDITION_SET_BATTLE_ARRAY_REQ& rstReq)
{
	if (pstData->GetGuildId() == 0)
	{
		LOGERR("Uin<%lu> not in the guild", pstData->m_ullUin);
		return ERR_NOT_HAVE_GUILD;
	}
	if (!IsExpeditionOpen())
	{
		return ERR_MODULE_NOT_OPEN;
	}
	set<uint64_t> GcardSet;
	size_t Count = 0;
	bool bIsAllEmpty1 = true;
	bool bIsAllEmpty2 = true;
	for (int i = 0; i < MAX_TROOP_NUM_PVP; i++)
	{
		if (i < rstReq.m_bGCardIdCount1 && rstReq.m_GCardIdList1[i] != 0)
		{
			GcardSet.insert(rstReq.m_GCardIdList1[i]);
			Count++;
			bIsAllEmpty1 = false;
		}
		if (i < rstReq.m_bGCardIdCount2 && rstReq.m_GCardIdList2[i] != 0)
		{
			GcardSet.insert(rstReq.m_GCardIdList2[i]);
			Count++;
			bIsAllEmpty2 = false;
		}
	}
	if (bIsAllEmpty1 || bIsAllEmpty2)
	{
		LOGERR("Uin<%lu> ERR_GUILD_EXPEDITION_MUST_HAVE_A_CARD", pstData->m_ullUin);
		return ERR_GUILD_EXPEDITION_MUST_HAVE_A_CARD;
	}
	if (GcardSet.size() != Count)
	{
		LOGERR("Uin<%lu> set GuildExpeditionArray error", pstData->m_ullUin);
		return ERR_SYS;
	}
	int iRet = ERR_NONE;
	do 
	{

		if (rstReq.m_bType == 1)
		{
			//防守阵容设置
			iRet = Majesty::Instance().SetBattleGeneralList(pstData, rstReq.m_bGCardIdCount1, rstReq.m_GCardIdList1, BATTLE_ARRAY_TYPE_GUILD_EXPEDITION_DEFEND_1);
			if (iRet != ERR_NONE)
			{
				return iRet;
			}
			iRet = Majesty::Instance().SetBattleGeneralList(pstData, rstReq.m_bGCardIdCount2, rstReq.m_GCardIdList2, BATTLE_ARRAY_TYPE_GUILD_EXPEDITION_DEFEND_2);
			if (iRet != ERR_NONE)
			{
				return iRet;
			}
			iRet = UploadDefendFightInfo(pstData);
		}
		else
		{
			//战斗阵容设置
			iRet = Majesty::Instance().SetBattleGeneralList(pstData, rstReq.m_bGCardIdCount1, rstReq.m_GCardIdList1, BATTLE_ARRAY_TYPE_GUILD_EXPEDITION_FIGHT_1);
			if (iRet != ERR_NONE)
			{
				return iRet;
			}
			iRet = Majesty::Instance().SetBattleGeneralList(pstData, rstReq.m_bGCardIdCount2, rstReq.m_GCardIdList2, BATTLE_ARRAY_TYPE_GUILD_EXPEDITION_FIGHT_2);
			if (iRet != ERR_NONE)
			{
				return iRet;
			}
		}
	} while (0);




	return iRet;
}


int Guild::UploadDefendFightInfo(PlayerData* pstData)
{
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_SET_FIGHT_INFO_NTF;
	m_stSsPkg.m_stHead.m_ullUin = pstData->m_ullUin;
	SS_PKG_GUILD_EXPEDITION_SET_FIGHT_INFO_NTF& rstNtf = m_stSsPkg.m_stBody.m_stGuildExpeditionSetFightInfoNtf;
	bzero(&rstNtf, sizeof(rstNtf));

	if (ERR_NONE != Match::Instance().InitBattleArrayFightInfo(pstData, BATTLE_ARRAY_TYPE_GUILD_EXPEDITION_DEFEND_1, rstNtf.m_stFightInfo.m_astBattleArrayFightInfo[0]))
	{
		LOGERR("Uin<%lu> InitBattleArrayFightInfo1 error", pstData->m_ullUin);
		return ERR_SYS;
	}

	if (ERR_NONE != Match::Instance().InitBattleArrayFightInfo(pstData, BATTLE_ARRAY_TYPE_GUILD_EXPEDITION_DEFEND_2, rstNtf.m_stFightInfo.m_astBattleArrayFightInfo[1]))
	{
		LOGERR("Uin<%lu> InitBattleArrayFightInfo2 error", pstData->m_ullUin);
		return ERR_SYS;
	}
	rstNtf.m_ullGuildId = pstData->GetGuildId();
	rstNtf.m_stShowInfo.m_dwHigheastLi = pstData->GetHighestLi();
	rstNtf.m_stShowInfo.m_dwLi = rstNtf.m_stFightInfo.m_astBattleArrayFightInfo[0].m_dwTroopLi + rstNtf.m_stFightInfo.m_astBattleArrayFightInfo[1].m_dwTroopLi;
	rstNtf.m_stShowInfo.m_wLv = pstData->GetLv();
	rstNtf.m_stShowInfo.m_bVipLv = pstData->GetVIPLv();
	rstNtf.m_stShowInfo.m_wHeadIconId = pstData->GetHeadIconId();
	rstNtf.m_stShowInfo.m_wHeadFrameId = pstData->GetHeadFrameId();
	pstData->GetRoleName(rstNtf.m_stShowInfo.m_szName);
	m_stSsPkg.m_stHead.m_ullReservId = 0;
	ZoneSvrMsgLayer::Instance().SendToGuildExpeditionSvr(m_stSsPkg);
	return ERR_NONE;
}


int Guild::ExpeditionFightResult(PlayerData* pstData, CS_PKG_GUILD_EXPEDITION_FIGHT_RESULT_REQ& rstCSReq)
{
	if (!IsExpeditionOpen())
	{
		return ERR_MODULE_NOT_OPEN;
	}
	//战斗分2场,且要先完成第一场,然后才能完成第二场
	//完成第一场会消耗战斗次数
	if ((rstCSReq.m_bSceneNum != 1 && rstCSReq.m_bSceneNum != 2) || 
		!((pstData->m_bGuildExpeditionFightSceneNum == 1 && rstCSReq.m_bSceneNum == 2) ||
		(rstCSReq.m_bSceneNum == 1)))
	{
		LOGERR("Uin<%lu> GuildExpedition fight result error, PlayerSceneNum<%d>, ReqSceneNum<%d>",
			pstData->m_ullUin, pstData->m_bGuildExpeditionFightSceneNum, rstCSReq.m_bSceneNum);
		return ERR_WRONG_PARA;
	}
	if (pstData->GetGuildId() == 0)
	{
		LOGERR("Uin<%lu> GuildExpedition fight result not in guild", pstData->m_ullUin);
		return ERR_NOT_HAVE_GUILD;
	}
	if (rstCSReq.m_bSceneNum == 1 && pstData->GetGuildInfo().m_stGuildExpeditionInfo.m_bFightCount >= m_iExpeditionMaxFightNum)
	{
		LOGERR("Uin<%lu> fight num<%d> limit", pstData->m_ullUin, (int)pstData->GetGuildInfo().m_stGuildExpeditionInfo.m_bFightCount);
		return ERR_NOT_ENOUGH_TIMES;
	}
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_FIGHT_RESULT_REQ;
	m_stSsPkg.m_stHead.m_ullUin = pstData->m_ullUin;
	SS_PKG_GUILD_EXPEDITION_FIGHT_RESULT_REQ& rstSSReq = m_stSsPkg.m_stBody.m_stGuildExpeditionFightResultReq;
	rstSSReq.m_ullGuildId = pstData->GetGuildId();
	rstSSReq.m_ullFoeUin = rstCSReq.m_ullFoeUin;
	rstSSReq.m_ullFoeGuildId = rstCSReq.m_ullFoeGuildId;
	rstSSReq.m_bIsWin = rstCSReq.m_bIsWin;
	rstSSReq.m_bSceneNum = rstCSReq.m_bSceneNum;
	m_stSsPkg.m_stHead.m_ullReservId = 0;
	ZoneSvrMsgLayer::Instance().SendToGuildExpeditionSvr(m_stSsPkg);
	return ERR_NONE;
}


int Guild::SettleExpeditionFight(PlayerData* pstData, SS_PKG_GUILD_EXPEDITION_FIGHT_RESULT_RSP& rstRsp, DT_SYNC_ITEM_INFO& rstSyncInfo)
{

	uint32_t dwAwardId = rstRsp.m_bIsWin == 1 ? 2001 : 2002;
	int iDamage = rstRsp.m_bIsWin == 1 ? m_iExpeditionWinDamage : m_iExpeditionLoseDamage;
	Props::Instance().GetCommonAward(pstData, dwAwardId, rstSyncInfo, METHOD_GUILD_EXPDITION_FIGHT_SETTLE);
	pstData->m_bGuildExpeditionFightSceneNum = rstRsp.m_bSceneNum;
	if (pstData->m_bGuildExpeditionFightSceneNum == 1)
	{
		pstData->GetGuildInfo().m_stGuildExpeditionInfo.m_bFightCount++;
	}
	//重新记录恢复时间
	if (pstData->GetGuildInfo().m_stGuildExpeditionInfo.m_bFightCount == 1)
	{
		pstData->GetGuildInfo().m_stGuildExpeditionInfo.m_ullLastRecoverTime = CGameTime::Instance().GetCurrSecond();
	}
	//造成伤害
	Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_GUILD_OTHER, iDamage, 5, 2);
	//战斗次数
	Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_GUILD_OTHER, 1, 5, 1);
	return ERR_NONE;
}


void Guild::GetRoleExpeditionInfo(PlayerData* pstData, OUT DT_ROLE_GUILD_EXPEDITION_INFO& rstExpeditionInfo)
{
	rstExpeditionInfo = pstData->GetGuildInfo().m_stGuildExpeditionInfo;
}

int Guild::ExpeditionMatch(PlayerData* pstData)
{
	if (!IsExpeditionOpen())
	{
		return ERR_MODULE_NOT_OPEN;
	}
	if (pstData->GetGuildId() == 0)
	{
		LOGERR("Uin<%lu> GuildExpedition fight result not in guild", pstData->m_ullUin);
		return ERR_NOT_HAVE_GUILD;
	}
	DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();

	if (rstGuildInfo.m_bGuildIdentify != GUILD_IDENTITY_LEADER
		&& rstGuildInfo.m_bGuildIdentify != GUILD_IDENTITY_DEPUTY_LEADER
		&& rstGuildInfo.m_bGuildIdentify != GUILD_IDENTITY_ELDERS)
	{
		LOGERR("Uin<%lu> expedition match error GuildIdentify<%d>", pstData->m_ullUin, (int)rstGuildInfo.m_bGuildIdentify);
		return ERR_IDENTITY;
	}

	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_MATCH_REQ;
	m_stSsPkg.m_stHead.m_ullUin = pstData->m_ullUin;
	SS_PKG_GUILD_EXPEDITION_MATCH_REQ& rstSSReq = m_stSsPkg.m_stBody.m_stGuildExpeditionMatchReq;
	m_stSsPkg.m_stHead.m_ullReservId = 0;
    rstSSReq.m_ullGuildId = pstData->GetGuildId();
	ZoneSvrMsgLayer::Instance().SendToGuildExpeditionSvr(m_stSsPkg);
	return ERR_NONE;
}

int Guild::DrawSalary(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncInfo, uint8_t bSalaryIdentity)
{
	DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();

	ResGuildSalaryMgr_t& rResGuildSalaryMgr = CGameDataMgr::Instance().GetResGuildSalaryMgr();
	RESGUILDSALARY* pstGuildSalary = rResGuildSalaryMgr.Find(bSalaryIdentity);
	if ( !pstGuildSalary )
	{
		LOGERR("pstGuildSalary is null. index<%d>", bSalaryIdentity);
		return ERR_SYS;
	}

	rstGuildInfo.m_bGuildSalaryDrawed = 1;
	for (int i=0; i<pstGuildSalary->m_bCount; i++)
	{
		Item::Instance().RewardItem(pstData, pstGuildSalary->m_szRewardTypeList[i], pstGuildSalary->m_rewardIdList[i],
									pstGuildSalary->m_rewardCountList[i], rstSyncInfo, METHOD_GUILD_OP_DRAW_SALARY);
	}

	//金币奖励
	int iGoldNum = pstGuildSalary->m_rewardGoldList[rstGuildInfo.m_bGuildLevel - 1];
	Item::Instance().RewardItem(pstData, ITEM_TYPE_GOLD, 0, iGoldNum, rstSyncInfo, METHOD_GUILD_OP_DRAW_SALARY);


	return ERR_NONE;
}

void Guild::UpdateSocietyBuffs(DT_ROLE_GUILD_INFO& rstGuildInfo, DT_GUILD_PKG_UPDATE_GUILD_SOCIETY& rstUpdateGuildSociety)
{
	switch (rstUpdateGuildSociety.m_bType)
	{
	case GUILD_SOCIETY_TYPE_GOLD_TIME:
		rstGuildInfo.m_bGuildGoldTime = rstUpdateGuildSociety.m_bUpData;
		break;
	case GUILD_SOCIETY_TYPE_GOLD_SCORE:
		rstGuildInfo.m_bGuildGoldScoreRatio = rstUpdateGuildSociety.m_bUpData;
		break;
	case GUILD_SOCIETY_TYPE_LEVEL_GOLD:
		rstGuildInfo.m_bGuildLevelGoldRatio = rstUpdateGuildSociety.m_bUpData;
		break;
	case GUILD_SOCIETY_TYPE_LEVEL_CONTRI:
		rstGuildInfo.m_bGuildLevelContrRatio = rstUpdateGuildSociety.m_bUpData;
		break;
	default:
		break;
	}

}

void Guild::GuildBossGetSingleReward(PlayerData * pstData, uint8_t bBossId)
{
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_BOSS_SINGLE_REWARD_RSP;
    RESGUILDBOSSINFO* resGuildBossInfo = CGameDataMgr::Instance().GetResGuildBossInfoMgr().Find(bBossId);
    if (resGuildBossInfo == NULL)
    {
        LOGERR("Current Boss is NULL");
        m_stScPkg.m_stBody.m_stGuildBossGetSingleRewardRsp.m_nErrNo = ERR_GUILD_BOSS_NOT_FOUND;

        ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
        return;
    }

    DT_ROLE_GUILD_INFO& rstGuildInfo = pstData->GetGuildInfo();
    SC_PKG_GUILD_BOSS_SINGLE_REWARD_RSP& rstPkgRsp = m_stScPkg.m_stBody.m_stGuildBossGetSingleRewardRsp;
    rstPkgRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    uint8_t bBossIndex = --bBossId;
    int iRet = ERR_NONE;
    //可以领取奖励
    if (rstGuildInfo.m_szGuildBossAwardStateList[bBossIndex] == COMMON_AWARD_STATE_AVAILABLE)
    {
        //可以领取首杀奖励
        if ((rstGuildInfo.m_ullGuildBossFirstAwardMap & ((uint64_t)1 << bBossIndex)) == 0)
        {
            uint8_t bFirstKillRwdTypeNum = resGuildBossInfo->m_bFirstKillRwdTypeNum;
            for (int i = 0; i != bFirstKillRwdTypeNum; ++i)
            {
                Item::Instance().RewardItem(
                    pstData,
                    resGuildBossInfo->m_szFirstKillRwdType[i],
                    resGuildBossInfo->m_firstKillRewardId[i],
                    resGuildBossInfo->m_firstKillRwdNum[i],
                    rstPkgRsp.m_stSyncItemInfo,
                    METHOD_GUILD_OP_FIRST_KILL_BOSS_REWARD);
            }
            rstGuildInfo.m_szGuildBossAwardStateList[bBossIndex] = COMMON_AWARD_STATE_DRAWED;
            rstGuildInfo.m_ullGuildBossFirstAwardMap |= ((uint64_t)1 << bBossIndex);
        }
        else
        {
            uint8_t bKillRwdTypeNum = resGuildBossInfo->m_bKillRwdTypeNum;
            for (int i = 0; i != bKillRwdTypeNum; ++i)
            {
                Item::Instance().RewardItem(
                    pstData,
                    resGuildBossInfo->m_szKillRwdType[i],
                    resGuildBossInfo->m_killRewardId[i],
                    resGuildBossInfo->m_killRwdNum[i],
                    rstPkgRsp.m_stSyncItemInfo,
                    METHOD_GUILD_OP_GET_BOSS_KILLED_AWARD);
            }
            rstGuildInfo.m_szGuildBossAwardStateList[bBossIndex] = COMMON_AWARD_STATE_DRAWED;
        }
    }
    else
    {
        iRet = ERR_AWARD_FINISHED;
    }

    rstPkgRsp.m_nErrNo = iRet;
    memcpy(rstPkgRsp.m_szGuildBossAwardStateList, rstGuildInfo.m_szGuildBossAwardStateList,
        sizeof(rstGuildInfo.m_szGuildBossAwardStateList));
    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
    return;
}


bool Guild::IsExpeditionOpen()
{
	return m_iExpeditionServerOpenDay <= SvrTime::Instance().GetOpenSvrDay();
}
