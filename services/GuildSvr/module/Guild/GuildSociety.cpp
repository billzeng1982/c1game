#include "GuildSociety.h"
#include "LogMacros.h"
#include "../../gamedata/GameDataMgr.h"
#include "GuildMgr.h"
#include "Guild.h"

bool GuildSociety::InitFromDB(DT_GUILD_SOCIETY_BLOB& rstBlob, uint64_t ullGuildId, uint16_t wVersion)
{
	//bzero(&m_oSocietyInfo, sizeof(m_oSocietyInfo));
	size_t ulUseSize = 0;
	int iRet = m_oSocietyInfo.unpack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
	if (iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("Guild(%lu) init Apply failed, unpack DT_GUILD_SOCIETY_BLOB failed, Ret=%d", ullGuildId, iRet);
		return false;
	}
	m_ullGuildId = ullGuildId;
	return true;
}

bool GuildSociety::PackGuildSocietyInfo(PKGMETA::DT_GUILD_SOCIETY_BLOB& rstBlob, uint16_t wVersion)
{
	size_t ulUseSize = 0;
	int iRet = m_oSocietyInfo.pack((char*)rstBlob.m_szData, MAX_LEN_GUILD_SOCIETY, &ulUseSize, wVersion);
	if (iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("Guild(%lu) pack DT_GUILD_SOCIETY_BLOB failed, iRet=%d, UsedSize=%lu", m_ullGuildId, iRet, ulUseSize);
		return false;
	}
	rstBlob.m_iLen = (int)ulUseSize;
	return true;
}

void GuildSociety::Clear()
{
	m_ullGuildId = 0;
	//bzero(&m_oSocietyInfo, sizeof(m_oSocietyInfo));
}

bool GuildSociety::Init()
{
	for (int i=0; i<MAX_GUILD_SOCIETY_TYPE; i++)
	{
		m_oSocietyInfo.m_astBranches[i].m_bLv = 1;
	}
	return true;
}

int GuildSociety::AddExp(uint8_t bType, uint32_t dwValueExp)
{
	uint8_t& rbBranchLv = m_oSocietyInfo.m_astBranches[bType-1].m_bLv;
	uint32_t& rdwExp = m_oSocietyInfo.m_astBranches[bType-1].m_dwExp;

	//uint8_t bTmpLv = rbBranchLv;

	ResGuildSocietyInfoMgr_t& rResGuildSocietyInfoMgr = CGameDataMgr::Instance().GetResGuildSocietyInfoMgr();
	ResGuildSocietyMgr_t& rResGuildSocietyMgr = CGameDataMgr::Instance().GetResGuildSocietyMgr();

	rdwExp += dwValueExp;

	RESGUILDSOCIETYINFO* pstGuildSocietyInfo = rResGuildSocietyInfoMgr.Find(bType);
	if (!pstGuildSocietyInfo)
	{
		LOGERR("pstGuildSocietyInfo is null, line<:%d>", bType);
		return ERR_SYS;
	}

	uint32_t dwIndexGuildSociety = pstGuildSocietyInfo->m_dwStartIndexId + rbBranchLv;
	RESGUILDSOCIETY* pstGuildSociety = rResGuildSocietyMgr.Find(dwIndexGuildSociety);
	if (!pstGuildSociety)
	{
		LOGERR("pstGuildSociety is null, line<:%d>", dwIndexGuildSociety);
		return ERR_SYS;
	}

	while (rdwExp >= pstGuildSociety->m_ullLvUpReqExp)
	{
		if (rbBranchLv == pstGuildSocietyInfo->m_bMaxLv)
		{
			break;
		}
		rbBranchLv++;

		dwIndexGuildSociety = pstGuildSocietyInfo->m_dwStartIndexId + rbBranchLv;
		pstGuildSociety = rResGuildSocietyMgr.Find(dwIndexGuildSociety);
		if (!pstGuildSociety)
		{
			LOGERR("pstGuildSociety is null, line<:%d>", dwIndexGuildSociety);
			return ERR_SYS;
		}

		if (rbBranchLv == pstGuildSocietyInfo->m_bMaxLv)
		{
			break;
		}
	}

	return ERR_NONE;
}

int GuildSociety::GetValue(uint8_t bType)
{
	uint8_t bBranchLv = m_oSocietyInfo.m_astBranches[bType-1].m_bLv;

	ResGuildSocietyInfoMgr_t& rResGuildSocietyInfoMgr = CGameDataMgr::Instance().GetResGuildSocietyInfoMgr();
	ResGuildSocietyMgr_t& rResGuildSocietyMgr = CGameDataMgr::Instance().GetResGuildSocietyMgr();

	RESGUILDSOCIETYINFO* pstGuildSocietyInfo = rResGuildSocietyInfoMgr.Find(bType);
	if (!pstGuildSocietyInfo)
	{
		LOGERR("pstGuildSocietyInfo is null, line<:%d>", bType);
		return 0;
	}

	uint32_t dwIndexGuildSociety = pstGuildSocietyInfo->m_dwStartIndexId + bBranchLv;
	RESGUILDSOCIETY* pstGuildSociety = rResGuildSocietyMgr.Find(dwIndexGuildSociety);
	if (!pstGuildSociety)
	{
		LOGERR("pstGuildSociety is null, line<:%d>", dwIndexGuildSociety);
		return 0;
	}

	return pstGuildSociety->m_wDataUp;
}

bool GuildSociety::IsLvMax(uint8_t bType)
{
	uint8_t bBranchLv = m_oSocietyInfo.m_astBranches[bType-1].m_bLv;

	ResGuildSocietyInfoMgr_t& rResGuildSocietyInfoMgr = CGameDataMgr::Instance().GetResGuildSocietyInfoMgr();
	ResGuildSocietyMgr_t& rResGuildSocietyMgr = CGameDataMgr::Instance().GetResGuildSocietyMgr();

	RESGUILDSOCIETYINFO* pstGuildSocietyInfo = rResGuildSocietyInfoMgr.Find(bType);
	if (!pstGuildSocietyInfo)
	{
		LOGERR("pstGuildSocietyInfo is null, line<:%d>", bType);
		return true;
	}

	uint32_t dwIndexGuildSociety = pstGuildSocietyInfo->m_dwStartIndexId + bBranchLv;
	RESGUILDSOCIETY* pstGuildSociety = rResGuildSocietyMgr.Find(dwIndexGuildSociety);
	if (!pstGuildSociety)
	{
		LOGERR("pstGuildSociety is null, line<:%d>", dwIndexGuildSociety);
		return true;
	}

	if (pstGuildSocietyInfo->m_bMaxLv == bBranchLv)
	{
		return true;
	}

	Guild* poGuild = GuildMgr::Instance().GetGuild(m_ullGuildId);
	if (!poGuild)
	{
		LOGERR("poGuild not found. guildid is <%lu>", m_ullGuildId);
		return true;
	}

	if (poGuild->GetGuildLevel() < pstGuildSociety->m_wLvUpReqGuildLv)
	{
		return true;
	}
	else
	{
		return false;
	}

}



