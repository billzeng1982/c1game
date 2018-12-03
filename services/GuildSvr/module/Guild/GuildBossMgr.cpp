#include "GuildBossMgr.h"
#include "LogMacros.h"
#include "../../gamedata/GameDataMgr.h"
#include <stdlib.h>
#include "strutil.h"
#include <unistd.h>
#include "Guild.h"
#include "GuildMgr.h"
#include "dwlog_def.h"
#include "FakeRandom.h"
#include <algorithm>
#include "PKGMETA_metalib.h"
#include "../../GuildSvr.h"



bool GuildBossMgr::Init(const char* pszFileName)
{

    time_t tmp = 0;
    if (0 != CGameTime::Instance().ConvStrToTime(GuildSvr::Instance().GetConfig().m_szSvrOpenTime, tmp, "%Y/%m/%d:%H:%M:%S"))
    {
        LOGERR("Get the server open time error!");
        return false;
    }
    m_ullTimeSec = tmp;
    LOGRUN("### server open time is %s , UnixTime is %lu .", 
        GuildSvr::Instance().GetConfig().m_szSvrOpenTime, m_ullTimeSec);

	//存储进度的文件名
	StrCpy(m_szFileName, pszFileName, MAX_NAME_LENGTH);

	if (access(m_szFileName, F_OK))
	{
		if (!this->_FirstInit())
		{
			LOGERR_r("GuildFightMgr first init failed.");
			return false;
		}
	}
	else
	{
		if (!this->_InitFromFile())
		{
			LOGERR_r("GuildFightMgr init from file failed.");
			return false;
		}
	}

    this->_InitBossFixedInfo();

	return true;
}

void GuildBossMgr::Update()
{
	uint64_t ullCurrTime = CGameTime::Instance().GetCurrSecond();

	if (ullCurrTime >= m_oGuildBossFile.m_oData.m_ullChgStateTime)
	{
		bIsOpen = true;
	}
}

void GuildBossMgr::Fini()
{
    if (this->_SaveSchedule())
    {
        LOGRUN_r("GuildBossMgr Fini success");
    }
    else
    {
        LOGRUN_r("GuildBossMgr Fini failed");
    }
}

void GuildBossMgr::GetState(DT_GUILD_BOSS_MODULE_STATE& rstStateInfo)
{
	rstStateInfo.m_bState = this->IsOpen();
	rstStateInfo.m_ullTimeStamp = m_oGuildBossFile.m_oData.m_ullChgStateTime;
}

bool GuildBossMgr::IsOpen()
{
	return bIsOpen;
}

bool GuildBossMgr::_FirstInit()
{
    m_oGuildBossFile.Init(m_szFileName, &m_bIsFileExist);
	ResBasicMgr_t& rBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
	RESBASIC* poOpenDeltaTime = rBasicMgr.Find(GUILD_BOSS_OPEN_TIME_DELTA);
	if(NULL == poOpenDeltaTime)
	{
		LOGERR_r("not exist open delta time");
		return false;
	}

	//开服后首次开启军团BOSS的时间
    m_oGuildBossFile.m_oData.m_ullChgStateTime = poOpenDeltaTime->m_para[0] + m_ullTimeSec;
	bIsOpen = false;

    if (this->_SaveSchedule())
    {
        return true;
    }
    else
    {
        return false;
    }
}

DT_GUILD_BOSS_PASSED_GUILD* GuildBossMgr::GetGuildBossPassedGuildInfo(int iBossIndex)
{
    if (iBossIndex >= 0)
    {
        return &m_oGuildBossFile.m_oData.m_astPassGuildRank[iBossIndex];
    }
    return NULL;
}

void GuildBossMgr::SetPassedGuildInfo(uint64_t ullGuildId, int iBossIndex)
{
    if (m_oGuildBossFile.m_oData.m_astPassGuildRank[iBossIndex].m_bCount < GUILD_BOSS_PASSED_GUILD_RANK_SHOW_NUM && iBossIndex >= 0)
    {
        uint64_t ullCurrTime = CGameTime::Instance().GetCurrSecond();
        uint8_t bCount = m_oGuildBossFile.m_oData.m_astPassGuildRank[iBossIndex].m_bCount;
        m_oGuildBossFile.m_oData.m_astPassGuildRank[iBossIndex].m_GuildId[bCount] = ullGuildId;
        Guild* poGuild = GuildMgr::Instance().GetGuild(ullGuildId);
        if (poGuild == NULL)
        {
            LOGERR("poGuild is NULL");
            return;
        }
        StrCpy(m_oGuildBossFile.m_oData.m_astPassGuildRank[iBossIndex].m_aszGuildName[bCount], poGuild->GetGuildName(), DWLOG::MAX_NAME_LENGTH);
        m_oGuildBossFile.m_oData.m_astPassGuildRank[iBossIndex].m_PassedTime[bCount] = ullCurrTime;
        ++m_oGuildBossFile.m_oData.m_astPassGuildRank[iBossIndex].m_bCount;
        this->_SaveSchedule();
    }
}

bool GuildBossMgr::_InitFromFile()
{
    return m_oGuildBossFile.Init(m_szFileName, &m_bIsFileExist);
}

bool GuildBossMgr::_SaveSchedule()
{
    if (m_oGuildBossFile.SaveFile(PKGMETA::MetaLib::getVersion()) != 0)
    {
        return false;
    }
    return true;
}

uint32_t GuildBossMgr::FindBossID( uint32_t dwFLevelID )
{
    std::map<uint32_t, uint32_t>::iterator iter = m_FLevelToBossIdMap.find(dwFLevelID);
    if( iter == m_FLevelToBossIdMap.end() )
    {
        return 0;
    }
    else
    {
        return iter->second;
    }
}

uint32_t GuildBossMgr::GetBossMaxHp( uint32_t dwBossID )
{
    std::map<uint32_t, uint32_t>::iterator iter = m_BossId2MaxHpMap.find(dwBossID);
    if( iter == m_BossId2MaxHpMap.end() )
    {
        return 0;
    }
    else
    {
        return iter->second;
    }
}

void GuildBossMgr::GetSubsectionAward(DT_GUILD_ONE_BOSS_INFO stGuildOneBossInfo, OUT int &iCount, OUT uint64_t *ullRet)
{
    //多少玩家可以领取奖励
    uint16_t wMemCount = stGuildOneBossInfo.m_wAttackedMemNum;
    uint32_t dwBossId = stGuildOneBossInfo.m_dwBossId;
    uint32_t dwDamageHp = stGuildOneBossInfo.m_dwDamegeHp;
    uint16_t wSubsectionRwdNum = _GetSubsectionRwdTypeNum(dwDamageHp, dwBossId);
    //将记录玩家伤害的数组随机排序，然后按照领奖人数或者奖励数
    int iRandomIndex = 0;
    DT_GUILD_BOSS_DAMAGE_NODE stTemp = { 0 };
    int iTemp = wMemCount;
    while (iTemp--)
    {
        iRandomIndex = CFakeRandom::Instance().Random(iTemp + 1);
        stTemp = stGuildOneBossInfo.m_astDamageOfBossList[iTemp];
        stGuildOneBossInfo.m_astDamageOfBossList[iTemp] = stGuildOneBossInfo.m_astDamageOfBossList[iRandomIndex];
        stGuildOneBossInfo.m_astDamageOfBossList[iRandomIndex] = stTemp;
    }

    iCount = min(wMemCount, wSubsectionRwdNum);
    for (int i = 0; i < iCount; ++i)
    {
        ullRet[i] = stGuildOneBossInfo.m_astDamageOfBossList[i].m_ullUin;
    }
}

int GuildBossMgr::GetSubsectionRwdTypeNum(uint32_t dwDamageHp, uint32_t dwBossId)
{
    return _GetSubsectionRwdTypeNum(dwDamageHp, dwBossId);
}

void GuildBossMgr::ResetPassedGuildNum(uint8_t bBossNum)
{
    for (int i = 0; i < bBossNum; ++i)
    {
        m_oGuildBossFile.m_oData.m_PassedGuildNum[i] = 0;
    }
    this->_SaveSchedule();
}

void GuildBossMgr::_InitBossFixedInfo()
{
    RESGUILDBOSSINFO* poResGuildBossInfo = NULL;
    RESFIGHTLEVELGENERAL* poResFightLevelGeneralInfo = NULL;
   
    uint16_t wResNum = CGameDataMgr::Instance().GetResGuildBossInfoMgr().GetResNum();
    
    for (int i = 0; i < wResNum && i < PKGMETA::MAX_GUILD_BOSS_NUM; i++)
    {
        poResGuildBossInfo = CGameDataMgr::Instance().GetResGuildBossInfoMgr().GetResByPos(i);
        assert( poResGuildBossInfo );
       
        m_FLevelToBossIdMap[ poResGuildBossInfo->m_dwLevelId ] = poResGuildBossInfo->m_dwId;

        poResFightLevelGeneralInfo = CGameDataMgr::Instance().GetResFightLevelGeneralMgr().Find(
            poResGuildBossInfo->m_dwGeneralId);
        assert(poResFightLevelGeneralInfo);

        m_BossId2MaxHpMap[ poResGuildBossInfo->m_dwId ] = poResFightLevelGeneralInfo->m_dwInitHP ;
        LOGRUN_r("BossId<%u>, MaxHp<%lu>", poResGuildBossInfo->m_dwId, poResFightLevelGeneralInfo->m_dwInitHP);
    }
}

//返回一共可以领取几种分段奖励
int GuildBossMgr::_GetSubsectionRwdTypeNum(uint32_t dwDamageHp, uint32_t dwBossId)
{
    uint32_t dwBossMaxHp = this->GetBossMaxHp(dwBossId);
    float fDamagePercent = 0;
    if (dwDamageHp >= dwBossMaxHp)
    {
        fDamagePercent = 1;
    }
    else
    {
        fDamagePercent = dwDamageHp / dwBossMaxHp;
    }

    //分段奖励分段数量
    RESGUILDBOSSINFO* poGuildBossInfo = CGameDataMgr::Instance().GetResGuildBossInfoMgr().Find(dwBossId);
    if (poGuildBossInfo == NULL)
    {
        LOGERR_r("poGuildBossInfo is NULL");
        return 0;
    }
    uint16_t wSubsectionNum = poGuildBossInfo->m_bSubsectionRwdTypeNum;
    int iSubsectionRwdNum = 0;
    while (fDamagePercent >= poGuildBossInfo->m_subsection[iSubsectionRwdNum] && iSubsectionRwdNum < wSubsectionNum)
    {
        ++iSubsectionRwdNum;
    }
    return iSubsectionRwdNum;
}
