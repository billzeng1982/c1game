

#include "sys/GameTime.h"
#include "LogicMgr.h"
#include "DataMgr.h"
#include "Guild.h"
#include "Player.h"
#include "../framework/GameDataMgr.h"


using namespace PKGMETA;

bool LogicMgr::Init()
{
	ResBasicMgr_t& rBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
	RESBASIC* poResBasic = rBasicMgr.Find(COMMON_UPDATE_TIME); assert(poResBasic);

	m_iDailyUpdateHour = poResBasic->m_para[0];
	m_ullLastMatchTime = CGameTime::Instance().GetSecOfCycleHourInCurrday(m_iDailyUpdateHour);

	poResBasic = rBasicMgr.Find(9501); assert(poResBasic);
	m_iLeastDefendNum = poResBasic->m_para[4];
	//m_iLeastDefendNum = 1;
	poResBasic = rBasicMgr.Find(9502); assert(poResBasic);
	m_iDefeatDamageHp = poResBasic->m_para[2];
	m_bWinDamageHp = poResBasic->m_para[3];
	m_bLoseDamageHp = poResBasic->m_para[4];
	poResBasic = rBasicMgr.Find(9505); assert(poResBasic);
	m_iPeriodDefeatRate = poResBasic->m_para[0];
	return true;
}


void LogicMgr::Update()
{
	//uint64_t ullNow = CGameTime::Instance().GetCurrSecond();
	uint64_t ullNewMatchTime = CGameTime::Instance().GetSecOfCycleHourInCurrday(m_iDailyUpdateHour);
	if (ullNewMatchTime > m_ullLastMatchTime)
	{
		m_ullLastMatchTime = ullNewMatchTime;
	}
}


void LogicMgr::Fini()
{

}

int LogicMgr::GetGuildAllInfo(uint64_t ullGuildId, OUT DT_GUILD_EXPEDITION_ALL_INFO& rstInfo)
{
	Guild* pstGuild = DataMgr::Instance().GetGuild(ullGuildId);
	if (pstGuild == NULL)
	{
		LOGERR(" cant find the Guild<%lu>", ullGuildId);
		return ERR_DB_ERROR;
	}
	//先执行匹配
	//pstGuild->MatchGuild();

	pstGuild->DailyUpdate();
	pstGuild->GetAllInfo(rstInfo);
	return ERR_NONE;
}

void LogicMgr::UploadGuildInfo(DT_GUILD_EXPEDITION_GUILD_UPLOAD_INFO& rstInfo)
{
	if (rstInfo.m_ullGuildId == 0 || rstInfo.m_szName[0] == '\0')
	{
		LOGERR("Upload guild info error,GuildId<%lu> Name<%s>", rstInfo.m_ullGuildId, rstInfo.m_szName);
		return;
	}
	Guild* pstGuild = DataMgr::Instance().GetGuild(rstInfo.m_ullGuildId);
	if (pstGuild != NULL)
	{

		pstGuild->UptInfo(rstInfo);
		LOGWARN("GuildID<%lu>  update info", rstInfo.m_ullGuildId);
		return;
	}
	//新建公会信息
	pstGuild = DataMgr::Instance().GetNewGuild();
	if (pstGuild == NULL)
	{
		LOGERR("cant get the GuildNode ,GuildId<%lu> Name<%s>", rstInfo.m_ullGuildId, rstInfo.m_szName);
		return;
	}
	pstGuild->CreateInit(rstInfo.m_ullGuildId, rstInfo);
	DataMgr::Instance().AddGuildToMap(pstGuild);
	DataMgr::Instance().SaveGuild(pstGuild);
	LOGWARN("GuildID<%lu>  add to map list", rstInfo.m_ullGuildId);
}

void LogicMgr::SetFightInfo(uint64_t ullUin, uint64_t ullGuildId, uint32_t dwZoneAddr, 
	PKGMETA::DT_GUILD_EXPEDITION_FIGHT_INFO& rstFightInfo, PKGMETA::DT_GUILD_EXPEDITION_PLAYER_SHOW_INFO& rstShowInfo)
{
	if (ullUin == 0)
	{
		LOGERR("Uin is 0");
		return;
	}
	Player* pstPlayer = DataMgr::Instance().GetPlayer(ullUin);
	if (pstPlayer != NULL)
	{
		pstPlayer->UptInfo(dwZoneAddr, ullGuildId, rstFightInfo, rstShowInfo);

		LOGWARN("Uin<%lu> set fight info", ullUin);
		return;
	}
	//新建玩家信息
	pstPlayer = DataMgr::Instance().GetNewPlayer();
	if (pstPlayer == NULL)
	{
		LOGERR("cant get the pstPlayerNode ,pstPlayer<%lu> GuildId<%lu>", ullUin, ullGuildId);
		return;
	}
	pstPlayer->CreateInit(ullUin, dwZoneAddr, ullGuildId, rstFightInfo, rstShowInfo);
	DataMgr::Instance().AddPlayerToMap(pstPlayer);
	DataMgr::Instance().SavePlayer(pstPlayer);
	LOGWARN("Uin<%lu> create player set fight info GuildId<%lu>", ullUin, ullGuildId);
}


int LogicMgr::FightRequest(uint64_t ullUin, uint64_t ullGuildId, uint64_t ullFoeUin, uint64_t ullFoeGuildId,
	OUT PKGMETA::DT_GUILD_EXPEDITION_FIGHT_INFO& rstFightInfo)
{
	Guild* pstGuild = DataMgr::Instance().GetGuild(ullGuildId);
	if (pstGuild == NULL)
	{
		LOGERR("Uin<%lu> fight request, not find the guild<%lu>", ullUin, ullGuildId);
		return ERR_NOT_EXIST_BY_GUILDID;
	}
	int iRet = pstGuild->CanFight(ullFoeUin, ullFoeGuildId);
	if ( iRet!= ERR_NONE)
	{
		LOGERR("ullUin<%lu> GuildId<%lu> cant fight  foeplayer<%lu> in foeguild<%lu> Ret<%d>", 
			ullUin, ullGuildId, ullFoeUin, ullFoeGuildId, iRet);
		return iRet;
	}
	Player* pstPlayer = DataMgr::Instance().GetPlayer(ullFoeUin);
	if (pstPlayer == NULL)
	{
		LOGERR("Uin<%lu> fight request, not find the foeplayer<%lu>", ullUin, ullFoeUin);
		return ERR_SYS;
	}
	pstPlayer->GetFightInfo(rstFightInfo);
	return ERR_NONE;
}


int LogicMgr::FightResult(uint64_t ullUin, uint64_t ullGuildId, uint8_t bIsWin, uint64_t ullFoeUin, uint64_t ullFoeGuildId)
{
	Guild* pstGuild = DataMgr::Instance().GetGuild(ullGuildId);
	if (pstGuild == NULL)
	{
		LOGERR("Uin<%lu> set fight ready error, not find the guild<%lu>", ullUin, ullGuildId);
		return ERR_NOT_EXIST_BY_GUILDID;
	}
	return pstGuild->HandleFightResult(ullUin, bIsWin, ullFoeGuildId, ullFoeUin);
}

void LogicMgr::GuildDissovle(uint64_t ullGuildId)
{
	Guild* pstGuild = DataMgr::Instance().GetGuild(ullGuildId);
	if (pstGuild == NULL)
	{
		LOGERR(" dissolve guild error, not find the guild<%lu>",  ullGuildId);
		return;
	}
	pstGuild->Dissolve();
	//DataMgr::Instance().DelGuild(pstGuild);
	//pstGuild = NULL;

}

void LogicMgr::GuildMemberQuit(uint64_t ullGuildId, uint64_t ullUin)
{
	Guild* pstGuild = DataMgr::Instance().GetGuild(ullGuildId);
	if (pstGuild == NULL)
	{
		LOGERR("Uin<%lu> set fight ready error, not find the guild<%lu>", ullUin, ullGuildId);
		return;
	}
	pstGuild->MemberQuit(ullUin);


}



