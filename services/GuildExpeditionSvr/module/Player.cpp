
#include "log/LogMacros.h"
#include "PKGMETA_metalib.h"
#include "Player.h"
#include "Guild.h"
#include "DataMgr.h"

using namespace PKGMETA;
bool Player::InitFromDB(DT_GUILD_EXPEDITION_PLAYER_DATA& rstData)
{
	this->Reset();

	size_t ulUseSize = 0;
	uint32_t dwVersion = rstData.m_dwVersion;
	int iRet = m_stPlayerInfo.unpack((char*)rstData.m_stPlayerBlob.m_szData, sizeof(rstData.m_stPlayerBlob.m_szData), &ulUseSize, dwVersion);
	if (iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR("Uin<%lu> unpack m_stPlayerBlob failed, Ret<%d>", rstData.m_ullUin, iRet);
		return false;
	}
	LOGWARN("Uin<%lu> InfiFromDB OK ", m_stPlayerInfo.m_ullUin);
	return true;
}

bool Player::PackToData(PKGMETA::DT_GUILD_EXPEDITION_PLAYER_DATA& rstData)
{
	size_t ulUseSize = 0;
	rstData.m_ullUin = m_stPlayerInfo.m_ullUin;
	rstData.m_dwVersion = MetaLib::getVersion();
	int iRet = m_stPlayerInfo.pack((char*)rstData.m_stPlayerBlob.m_szData, sizeof(rstData.m_stPlayerBlob.m_szData), &ulUseSize);
	if (iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR("GuildId<%lu> pack m_stPlayerBlob failed! Ret<%d> ", m_stPlayerInfo.m_ullUin, iRet);
		return false;
	}
	rstData.m_stPlayerBlob.m_iLen = (int)ulUseSize;
	return true;
}

void Player::UptInfo(uint32_t dwAddr, uint64_t ullGuildId, PKGMETA::DT_GUILD_EXPEDITION_FIGHT_INFO& rstFightInfo, PKGMETA::DT_GUILD_EXPEDITION_PLAYER_SHOW_INFO& rstShowInfo)
{
	uint32_t DeltaLi = rstShowInfo.m_dwHigheastLi - m_stPlayerInfo.m_stShowInfo.m_dwHigheastLi;
	m_stPlayerInfo.m_dwAddr = dwAddr;
	memcpy(&m_stPlayerInfo.m_stFightInfo, &rstFightInfo, sizeof(m_stPlayerInfo.m_stFightInfo));
	memcpy(&m_stPlayerInfo.m_stShowInfo, &rstShowInfo, sizeof(m_stPlayerInfo.m_stShowInfo));
	DataMgr::Instance().DelFromTimeList(this);
	DataMgr::Instance().AddToDirtyList(this);
	Guild* pstGuild = DataMgr::Instance().GetGuild(ullGuildId);
	if (pstGuild == NULL)
	{
		LOGERR("Uin<%lu> update fight info error, cant find the guild<%lu>", m_stPlayerInfo.m_ullUin, ullGuildId);
		return;
	}
	pstGuild->AddDefendMemberNum(m_stPlayerInfo.m_ullUin, DeltaLi);

}

void Player::CreateInit(uint64_t ullUin, uint32_t dwAddr, uint64_t ullGuildId, PKGMETA::DT_GUILD_EXPEDITION_FIGHT_INFO& rstFightInfo, PKGMETA::DT_GUILD_EXPEDITION_PLAYER_SHOW_INFO& rstShowInfo)
{
	this->Reset();
	m_stPlayerInfo.m_ullUin = ullUin;
	this->UptInfo(dwAddr, ullGuildId, rstFightInfo, rstShowInfo);

}

void Player::Reset()
{
	bzero(&m_stPlayerInfo, sizeof(m_stPlayerInfo));
	bzero(&m_stDirtyListNode, sizeof(m_stDirtyListNode));
	bzero(&m_stTimeListNode, sizeof(m_stTimeListNode));
}

void Player::GetShowInfo(OUT PKGMETA::DT_GUILD_EXPEDITION_PLAYER_SHOW_INFO& rstInfo)
{
	memcpy(&rstInfo, &m_stPlayerInfo.m_stShowInfo, sizeof(rstInfo));
}

void Player::GetFightInfo(OUT PKGMETA::DT_GUILD_EXPEDITION_FIGHT_INFO& rstInfo)
{
	memcpy(&rstInfo, &m_stPlayerInfo.m_stFightInfo, sizeof(rstInfo));
}

