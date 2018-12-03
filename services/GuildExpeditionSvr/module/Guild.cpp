
#include "utils/strutil.h"
#include "log/LogMacros.h"
#include "Guild.h"
#include "DataMgr.h"
#include "LogicMgr.h"
#include "PKGMETA_metalib.h"

using namespace PKGMETA;

static int Cmp(const void *left, const void *right)
{
	if (((DT_GUILD_EXPEDITION_ONE_MEMBER_INFO*)left)->m_ullUin > ((DT_GUILD_EXPEDITION_ONE_MEMBER_INFO*)right)->m_ullUin)
	{
		return 1;
	}
	else if (((DT_GUILD_EXPEDITION_ONE_MEMBER_INFO*)left)->m_ullUin == ((DT_GUILD_EXPEDITION_ONE_MEMBER_INFO*)right)->m_ullUin)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

static int CmpMatchedMember(const void *left, const void *right)
{
	if (((DT_GUILD_EXPEDITION_MATCHED_ONE_MEMBER_INFO*)left)->m_ullUin > ((DT_GUILD_EXPEDITION_MATCHED_ONE_MEMBER_INFO*)right)->m_ullUin)
	{
		return 1;
	}
	else if (((DT_GUILD_EXPEDITION_MATCHED_ONE_MEMBER_INFO*)left)->m_ullUin == ((DT_GUILD_EXPEDITION_MATCHED_ONE_MEMBER_INFO*)right)->m_ullUin)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

uint64_t Guild::GetId()
{
	return m_stGuildInfo.m_ullGuildId;
}


bool Guild::InitFromDB(PKGMETA::DT_GUILD_EXPEDITION_GUILD_DATA& rstData)
{
	this->Reset();
	size_t ulUseSize = 0;
	uint32_t dwVersion = rstData.m_dwVersion;
	int iRet = m_stGuildInfo.unpack((char*)rstData.m_stGuildBlob.m_szData, sizeof(rstData.m_stGuildBlob.m_szData), &ulUseSize, dwVersion);
	if (iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR("GuildId<%lu> unpack m_stGuildBlob failed, Ret<%d>", rstData.m_ullGuildId, iRet);
		return false;
	}

	for (int i = 0; i< m_stGuildInfo.m_chMemberCount; i++)
	{
		if (m_stGuildInfo.m_astMemberList[i].m_bIsDefend)
		{
			m_iTotalDefendReadyNum++;
		}
	}

	for (int i = 0; i < m_stGuildInfo.m_chMatchedGuildCount; i++)
	{
		for (int j = 0; j < m_stGuildInfo.m_astMatchedGuildList[i].m_chMemberCount; j++)
		{
			if (m_stGuildInfo.m_astMatchedGuildList[i].m_astMemberList[j].m_chDamageHp >= LogicMgr::Instance().m_iDefeatDamageHp)
			{
				m_iTotalDefeatNum++;
				m_oGuildId2DefeatNumMap[m_stGuildInfo.m_astMatchedGuildList[i].m_ullGuildId]++;
			}
		}
		m_iTotalMatchedGuildMemberNum += m_stGuildInfo.m_astMatchedGuildList[i].m_chMemberCount;
	}
	//加入到被匹配列表中
	if (m_iTotalDefendReadyNum >= LogicMgr::Instance().m_iLeastDefendNum)
	{
		DataMgr::Instance().AddToMatchList(this);
	}
	//全部同步一遍
	m_iUpdateAllDefendInfoFreq = 0;
	SendDefendInfo(0);

	LOGWARN("GuildId<%lu> InfiFromDB OK , defend num<%d>, Li<%lu>", m_stGuildInfo.m_ullGuildId, m_iTotalDefendReadyNum, m_stGuildInfo.m_ullGuildLi);
	
	return true;
}


bool Guild::PackToData(PKGMETA::DT_GUILD_EXPEDITION_GUILD_DATA& rstData)
{
	size_t ulUseSize = 0;
	rstData.m_ullGuildId = m_stGuildInfo.m_ullGuildId;
	rstData.m_dwVersion = MetaLib::getVersion();
	int iRet = m_stGuildInfo.pack((char*)rstData.m_stGuildBlob.m_szData, sizeof(rstData.m_stGuildBlob.m_szData), &ulUseSize);
	if (iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR("GuildId<%lu> pack m_stGuildBlob failed! Ret<%d> ", m_stGuildInfo.m_ullGuildId, iRet);
		return false;
	}
	rstData.m_stGuildBlob.m_iLen = (int)ulUseSize;
	return true;
}


void Guild::Reset()
{
	m_oGuildId2DefeatNumMap.clear();
	m_iTotalDefendReadyNum = 0;
	m_iTotalDefeatNum = 0;
	m_iTotalMatchedGuildMemberNum = 0;
	m_iUpdateAllDefendInfoFreq = 0;
	bzero(&m_stGuildInfo, sizeof(m_stGuildInfo));
	bzero(&m_stDirtyListNode, sizeof(m_stDirtyListNode));
	bzero(&m_stTimeListNode, sizeof(m_stTimeListNode));
}

void Guild::GetInfo(OUT PKGMETA::DT_GUILD_EXPEDITION_GUILD_INFO& rstInfo)
{
	memcpy(&rstInfo, &m_stGuildInfo, sizeof(rstInfo));

}


void Guild::UptInfo(DT_GUILD_EXPEDITION_GUILD_UPLOAD_INFO& rstInfo)
{
	m_stGuildInfo.m_dwAddr = rstInfo.m_dwAddr;
	StrCpy(m_stGuildInfo.m_szName, rstInfo.m_szName, sizeof(m_stGuildInfo.m_szName));
	LOGWARN("GuildId<%lu> update guild info ok, defend num<%d>", m_stGuildInfo.m_ullGuildId, m_iTotalDefendReadyNum);
	DataMgr::Instance().DelFromTimeList(this);
	DataMgr::Instance().AddToDirtyList(this);
}


void Guild::CreateInit(uint64_t ullGuildId, PKGMETA::DT_GUILD_EXPEDITION_GUILD_UPLOAD_INFO& rstInfo)
{
	this->Reset();
	m_stGuildInfo.m_ullGuildId = ullGuildId;
	this->UptInfo(rstInfo);
}

void Guild::GetAllInfo(OUT DT_GUILD_EXPEDITION_ALL_INFO& rstOutInfo)
{
	//获取战斗记录
	memcpy(rstOutInfo.m_astFightLogList, m_stGuildInfo.m_astFightLogList, sizeof(rstOutInfo.m_astFightLogList[0]) *  m_stGuildInfo.m_chFightLogCount);
	rstOutInfo.m_chFightLogCount = m_stGuildInfo.m_chFightLogCount;

	Player* pstPlayer = NULL;
	int GuildCount = 0;
	for (int i = 0 ; i < m_stGuildInfo.m_chMatchedGuildCount && i < MAX_GUILD_EXPEDITION_MATCH_NUM; i++)
	{

		DT_GUILD_EXPEDITION_MATCHED_GUILD_INFO& rstMatchedGuildInfo = m_stGuildInfo.m_astMatchedGuildList[i];
		DT_GUILD_EXPEDITION_MATCHED_GUILD_CLIENT_INFO& rstMatchedGuildClientInfo = rstOutInfo.m_astMatchedGuildInfo[i];

		Guild* pstGuild = DataMgr::Instance().GetGuild(rstMatchedGuildInfo.m_ullGuildId);
		if (!pstGuild)
		{
			LOGERR("GuildId<%lu>  can't get the matched Guild<%lu>", m_stGuildInfo.m_ullGuildId, 
				rstMatchedGuildInfo.m_ullGuildId);
			continue;
		}
		//获取公会的简要信息
		pstGuild->GetMatchedBriefInfo(rstMatchedGuildClientInfo);
		int MemCount = 0;
		for (int j = 0; j < rstMatchedGuildInfo.m_chMemberCount; j++)
		{
			pstPlayer = DataMgr::Instance().GetPlayer(rstMatchedGuildInfo.m_astMemberList[j].m_ullUin);
			if (pstPlayer == NULL)
			{
				LOGERR("GuildId<%lu> get all info, cant find the matched guild player<%lu>", m_stGuildInfo.m_ullGuildId, rstMatchedGuildInfo.m_astMemberList[j].m_ullUin);
				continue;
			}

			rstMatchedGuildClientInfo.m_astMemberList[MemCount].m_ullUin = rstMatchedGuildInfo.m_astMemberList[j].m_ullUin;

			//获取血量
			rstMatchedGuildClientInfo.m_astMemberList[MemCount].m_chDamageHp = rstMatchedGuildInfo.m_astMemberList[j].m_chDamageHp;

			//获取展示信息
			pstPlayer->GetShowInfo(rstMatchedGuildClientInfo.m_astMemberList[MemCount].m_stShowInfo);
			MemCount++;
		}
		rstMatchedGuildClientInfo.m_chMemberCount = MemCount;
		GuildCount++;
	}
	rstOutInfo.m_chMatchedGuildCount = GuildCount;

}



void Guild::GetMatchedInfo(OUT PKGMETA::DT_GUILD_EXPEDITION_MATCHED_GUILD_INFO& rstInfo)
{
	rstInfo.m_ullGuildId = m_stGuildInfo.m_ullGuildId;
	rstInfo.m_chMemberCount = 0;
	for (int i = 0; i < m_stGuildInfo.m_chMemberCount; i++)
	{
		if (m_stGuildInfo.m_astMemberList[i].m_bIsDefend == 1)
		{
			rstInfo.m_astMemberList[rstInfo.m_chMemberCount].m_ullUin = m_stGuildInfo.m_astMemberList[i].m_ullUin;
			rstInfo.m_astMemberList[rstInfo.m_chMemberCount].m_chDamageHp = 0;
			rstInfo.m_chMemberCount++;
		}
	}
}

int Guild::MatchGuild()
{
	if (m_stGuildInfo.m_chMatchedGuildCount != 0)
	{
		LOGERR("GuildId<%lu> match error: has mathed guild ,count<%d>", GetId(), (int)m_stGuildInfo.m_chMatchedGuildCount);
		return ERR_GUILD_EXPEDITION_ALREADY_MATCHED;
	}

	if (m_iTotalDefendReadyNum < LogicMgr::Instance().m_iLeastDefendNum)
	{
		LOGERR("GuildId<%lu> match error: defend ready num<%d>", GetId(), m_iTotalDefendReadyNum);
		//同步一下 防守阵容信息
		m_iUpdateAllDefendInfoFreq = 0;
		SendDefendInfo(0);
		return ERR_GUILD_EXPEDITION_NO_ENOUGH_MEMBER_SET_DEFEND;
	}

	int iMatchedCount = DataMgr::Instance().MatchGuild(m_stGuildInfo.m_ullGuildId, m_stGuildInfo.m_ullGuildLi,
		m_stGuildInfo.m_astMatchedGuildList);
	if (iMatchedCount < 0)
	{
		return iMatchedCount;
	}
	else if (iMatchedCount  != MAX_GUILD_EXPEDITION_MATCH_NUM)
	{

		LOGERR(" Guild<%lu> MatchGuild  the count<%d> ie error", m_stGuildInfo.m_ullGuildId, iMatchedCount);
		return ERR_GUILD_EXPEDITION_NO_ENOUGH_GUILD_MATCH;
	}
	else
	{
		m_stGuildInfo.m_chMatchedGuildCount = iMatchedCount;
		m_stGuildInfo.m_ullLastMatchTime = LogicMgr::Instance().m_ullLastMatchTime;
		LOGWARN(" Guild<%lu> matched guild ok, list=<%lu> <%lu> <%lu>", m_stGuildInfo.m_ullGuildId, m_stGuildInfo.m_astMatchedGuildList[0].m_ullGuildId,
			m_stGuildInfo.m_astMatchedGuildList[1].m_ullGuildId, m_stGuildInfo.m_astMatchedGuildList[2].m_ullGuildId);

		for (int i = 0; i < m_stGuildInfo.m_chMatchedGuildCount; i++)
		{
			m_iTotalMatchedGuildMemberNum += m_stGuildInfo.m_astMatchedGuildList[i].m_chMemberCount;
		}

	}
	DataMgr::Instance().DelFromTimeList(this);
	DataMgr::Instance().AddToDirtyList(this);
	return ERR_NONE;
}


int Guild::HandleFightResult(uint64_t ullUin, uint8_t bIsWin, uint64_t ullFoeGuildId, uint64_t ullFoeUin)
{
	DT_GUILD_EXPEDITION_MATCHED_GUILD_INFO* pstMatchedGuildInfo = FindMatchedGuild(ullFoeGuildId);
	if (pstMatchedGuildInfo == NULL)
	{
		LOGERR("GuildId<%lu> Cant find the MatchedGuild<%lu>", m_stGuildInfo.m_ullGuildId, ullFoeGuildId);
		return ERR_GUILD_EXPEDITION_CANT_FIND_MATHED_GUILD;
	}
	DT_GUILD_EXPEDITION_MATCHED_ONE_MEMBER_INFO* pstMatchedGMem = FindMatchedGuildMember(pstMatchedGuildInfo, ullFoeUin);
	if (pstMatchedGMem  == NULL)
	{
		LOGERR("GuildId<%lu> fight result:the ForUin<%lu> not in MatchedGuild<%lu>", m_stGuildInfo.m_ullGuildId, ullFoeUin, ullFoeGuildId);
		return ERR_GUILD_EXPEDITION_FOE_NOT_IN_MATCHED_GUILD;
	}
	int8_t OldDamageHp = pstMatchedGMem->m_chDamageHp;
	int8_t AddDamageHp = bIsWin == 1 ? LogicMgr::Instance().m_bWinDamageHp : LogicMgr::Instance().m_bLoseDamageHp;
	pstMatchedGMem->m_chDamageHp = MIN(pstMatchedGMem->m_chDamageHp + AddDamageHp, LogicMgr::Instance().m_iDefeatDamageHp);
	if (pstMatchedGMem->m_chDamageHp >= LogicMgr::Instance().m_iDefeatDamageHp && OldDamageHp < LogicMgr::Instance().m_iDefeatDamageHp)
	{
		m_iTotalDefeatNum++;
		m_oGuildId2DefeatNumMap[ullFoeGuildId]++;

		//检查发送奖励
		CheckSendAward(pstMatchedGuildInfo);
	}
	DT_GUILD_EXPEDITION_FIGHT_LOG* pstLog = AddFightLog(ullUin, bIsWin, ullFoeUin);
	//广播给公会成员
	SSPKG* pstNtfPkg = GuildExpeditionSvrMsgLayer::Instance().GetSendPkg();
	if (pstNtfPkg)
	{
		pstNtfPkg->m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_COMMON_NTF;
		pstNtfPkg->m_stHead.m_ullReservId = m_stGuildInfo.m_dwAddr;
		pstNtfPkg->m_stBody.m_stGuildExpeditionCommonNtf.m_wMsgId = DT_GUILD_MSG_EXPEDITION_FIGHT_RESULT;
		pstNtfPkg->m_stBody.m_stGuildExpeditionCommonNtf.m_ullGuildId = m_stGuildInfo.m_ullGuildId;
		DT_GUILD_PKG_EXPEDITION_FIGHT_RESULT& rstFightResult = pstNtfPkg->m_stBody.m_stGuildExpeditionCommonNtf.m_stNtfInfo.m_stExpeditionFightResult;
		rstFightResult.m_ullFoeUin = ullFoeUin;
		rstFightResult.m_ullFoeGuildId = ullFoeGuildId;
		rstFightResult.m_bFoeDamageHp = pstMatchedGMem->m_chDamageHp;
		if (pstLog)
		{
			memcpy(&rstFightResult.m_stFightLog, pstLog, sizeof(rstFightResult.m_stFightLog));
		}
		else
		{
			bzero(&rstFightResult.m_stFightLog, sizeof(rstFightResult.m_stFightLog));
		}
		GuildExpeditionSvrMsgLayer::Instance().SendToOtherSvrByGate(pstNtfPkg);
	}
	DataMgr::Instance().DelFromTimeList(this);
	DataMgr::Instance().AddToDirtyList(this);
	return ERR_NONE;
}


void Guild::DailyUpdate()
{
	if (m_stGuildInfo.m_ullLastMatchTime >= LogicMgr::Instance().m_ullLastMatchTime)
	{
		return ;
	}
	m_stGuildInfo.m_ullLastMatchTime = LogicMgr::Instance().m_ullLastMatchTime;
	m_stGuildInfo.m_chFightLogCount = 0;
	bzero(&m_stGuildInfo.m_astFightLogList, sizeof(m_stGuildInfo.m_astFightLogList));
	m_stGuildInfo.m_chMatchedGuildCount = 0;
	bzero(&m_stGuildInfo.m_astMatchedGuildList, sizeof(m_stGuildInfo.m_astMatchedGuildList));
	m_stGuildInfo.m_bPeriodAwardState = 0;
	
	m_iTotalDefeatNum = 0;
	m_iTotalMatchedGuildMemberNum = 0;
	m_oGuildId2DefeatNumMap.clear();
	
	DataMgr::Instance().DelFromTimeList(this);
	DataMgr::Instance().AddToDirtyList(this);
	//全部同步一遍
	m_iUpdateAllDefendInfoFreq = 0;
	SendDefendInfo(0);
}

int Guild::AddDefendMemberNum(uint64_t ullUin, uint32_t iDeltali)
{
	DT_GUILD_EXPEDITION_ONE_MEMBER_INFO* pstMember = FindMember(ullUin);
	if (pstMember == NULL)
	{
        if (m_stGuildInfo.m_chMemberCount >= MAX_NUM_MEMBER)
        {

            LOGERR("Uin<%lu> add to guild<%lu> error, mem full", ullUin, m_stGuildInfo.m_ullGuildId);
            return ERR_SYS;
        }
		pstMember = AddMember(ullUin);
		if (pstMember == NULL)
		{
			LOGERR("Uin<%lu> add to guild<%lu> error", ullUin, m_stGuildInfo.m_ullGuildId);
			return ERR_NOT_FOUND;
		}
	}
	//从匹配列表中删除旧有的
	if (iDeltali != 0 && m_iTotalDefendReadyNum >= LogicMgr::Instance().m_iLeastDefendNum)
	{
		DataMgr::Instance().DelFromMatchList(this);
	}
	if (iDeltali < 0)
	{
		//强制保护
		m_stGuildInfo.m_ullGuildLi = m_stGuildInfo.m_ullGuildLi > -iDeltali ? m_stGuildInfo.m_ullGuildLi + iDeltali : 0;
	}
	else
	{
		m_stGuildInfo.m_ullGuildLi = m_stGuildInfo.m_ullGuildLi + iDeltali;
	}

	
	if (pstMember->m_bIsDefend == 0)	
	{
		//处理新增加的设置了防守阵容的玩家
		pstMember->m_bIsDefend = 1;
		m_iTotalDefendReadyNum++;
		SendDefendInfo(ullUin);
	}
	//加入到被匹配列表中
	if (iDeltali != 0 && m_iTotalDefendReadyNum >= LogicMgr::Instance().m_iLeastDefendNum)
	{
		DataMgr::Instance().AddToMatchList(this);
	}
	DataMgr::Instance().DelFromTimeList(this);
	DataMgr::Instance().AddToDirtyList(this);
	
	return ERR_NONE;

}




int Guild::CanFight(uint64_t ullFoeUin, uint64_t ullFoeGuildId)
{
	
	DT_GUILD_EXPEDITION_MATCHED_GUILD_INFO* pstInfo = FindMatchedGuild(ullFoeGuildId);
	if (pstInfo == NULL)
	{
		
		return ERR_GUILD_EXPEDITION_CANT_FIND_MATHED_GUILD;
	}
	DT_GUILD_EXPEDITION_MATCHED_ONE_MEMBER_INFO* pstMemberInfo = FindMatchedGuildMember(pstInfo, ullFoeUin);
	if (pstMemberInfo == NULL)
	{
		return ERR_GUILD_EXPEDITION_CANT_FIND_MATHED_MEMBER;
	}
	if (pstMemberInfo->m_chDamageHp >= LogicMgr::Instance().m_iDefeatDamageHp)
	{
		return ERR_GUILD_EXPEDITION_MATHED_MEMBER_DEFEAT_OK;
	}

	return  ERR_NONE;
}

PKGMETA::DT_GUILD_EXPEDITION_ONE_MEMBER_INFO* Guild::FindMember(uint64_t ullUin)
{
	DT_GUILD_EXPEDITION_ONE_MEMBER_INFO stOneMember = { 0 };
	stOneMember.m_ullUin = ullUin;
	int iEqual = 0;
	int iIndex = MyBSearch(&stOneMember, m_stGuildInfo.m_astMemberList, m_stGuildInfo.m_chMemberCount,
		sizeof(DT_GUILD_EXPEDITION_ONE_MEMBER_INFO), &iEqual, Cmp);
	if (iEqual != 0)
	{
		return &m_stGuildInfo.m_astMemberList[iIndex];
	}
	return NULL;
}

void Guild::DelMember(uint64_t ullUin)
{
	DT_GUILD_EXPEDITION_ONE_MEMBER_INFO stOneMember = { 0 };
	stOneMember.m_ullUin = ullUin;

	size_t nmemb = (size_t)m_stGuildInfo.m_chMemberCount;
	if (!MyBDelete(&stOneMember, m_stGuildInfo.m_astMemberList, &nmemb, 
		sizeof(DT_GUILD_EXPEDITION_ONE_MEMBER_INFO), Cmp))
	{
		//LOGERR("Delete rstAgreeInfo  err!");
		return;
	}
	m_stGuildInfo.m_chMemberCount = (int8_t)nmemb;
	LOGWARN("Uin<%lu> quit the Guild<%lu>", ullUin, GetId());

}

PKGMETA::DT_GUILD_EXPEDITION_ONE_MEMBER_INFO* Guild::AddMember(uint64_t ullUin)
{

	size_t nmemb = m_stGuildInfo.m_chMemberCount;
	DT_GUILD_EXPEDITION_ONE_MEMBER_INFO rstMemberInfo = { 0 };
	rstMemberInfo.m_ullUin = ullUin;
	rstMemberInfo.m_bIsDefend = 0;
	int iIndex = 0;
	if (MyBInsertIndex(&rstMemberInfo, m_stGuildInfo.m_astMemberList, &nmemb, sizeof(DT_GUILD_EXPEDITION_ONE_MEMBER_INFO), 1, &iIndex, Cmp))
	{
		m_stGuildInfo.m_chMemberCount = nmemb;
		return &m_stGuildInfo.m_astMemberList[iIndex];
	}
	return NULL;
}

void Guild::UptMember(PKGMETA::DT_GUILD_EXPEDITION_ONE_MEMBER_INFO& rstMemberInfo)
{
	DT_GUILD_EXPEDITION_ONE_MEMBER_INFO* pstMemberInfo = FindMember(rstMemberInfo.m_ullUin);
	if (pstMemberInfo == NULL)
	{
		return;
	}
	uint8_t tmp = rstMemberInfo.m_bIsDefend;

	memcpy(pstMemberInfo, &rstMemberInfo, sizeof(DT_GUILD_EXPEDITION_ONE_MEMBER_INFO));
	pstMemberInfo->m_bIsDefend = tmp;
}

void Guild::GetMatchedBriefInfo(PKGMETA::DT_GUILD_EXPEDITION_MATCHED_GUILD_CLIENT_INFO& rstInfo)
{
	rstInfo.m_ullLi = m_stGuildInfo.m_ullGuildLi;
	rstInfo.m_ullGuildId = m_stGuildInfo.m_ullGuildId;
	rstInfo.m_dwAddr = m_stGuildInfo.m_dwAddr;
	StrCpy(rstInfo.m_szGuildName, m_stGuildInfo.m_szName, sizeof(rstInfo.m_szGuildName));
}


PKGMETA::DT_GUILD_EXPEDITION_FIGHT_LOG* Guild::AddFightLog(uint64_t ullUin, uint8_t bIsWin, uint64_t ullFoeUin)
{
// 	Player* pstPlayer = DataMgr::Instance().GetPlayer(ullUin);
// 	Player* pstFoePlayer = DataMgr::Instance().GetPlayer(ullFoeUin);
// 	if (pstPlayer == NULL  || pstFoePlayer == NULL)
// 	{
// 		LOGERR("Uin<%lu> FoeUin<%lu> cant find player", ullUin, ullFoeUin);
// 		return NULL;
// 	}
	DT_GUILD_EXPEDITION_FIGHT_LOG* pstLog = NULL; 
	if (m_stGuildInfo.m_chFightLogCount >= MAX_GUILD_EXPEDITION_FIGHT_LOG_NUM)
	{
		memcpy(m_stGuildInfo.m_astFightLogList, &m_stGuildInfo.m_astFightLogList[1], 
			sizeof(DT_GUILD_EXPEDITION_FIGHT_LOG) * (MAX_GUILD_EXPEDITION_FIGHT_LOG_NUM - 1));
		pstLog = &m_stGuildInfo.m_astFightLogList[MAX_GUILD_EXPEDITION_FIGHT_LOG_NUM - 1];
	}
	else
	{
		pstLog = &m_stGuildInfo.m_astFightLogList[m_stGuildInfo.m_chFightLogCount++];
	}
	pstLog->m_chIsWin = bIsWin;
	pstLog->m_ullSelfUin = ullUin;
	pstLog->m_ullFoeUin = ullFoeUin;
	pstLog->m_ullTime = CGameTime::Instance().GetCurrSecond();
	return pstLog;
}


void Guild::SendDefendInfo(uint64_t ullUin)
{
	SSPKG* pstPkgNew = GuildExpeditionSvrMsgLayer::Instance().GetSendPkg();
	if (pstPkgNew == NULL)
	{
		LOGERR("get send pkg error");
		return;
	}
	pstPkgNew->m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_COMMON_NTF;
	pstPkgNew->m_stHead.m_ullReservId = m_stGuildInfo.m_dwAddr;
	pstPkgNew->m_stBody.m_stGuildExpeditionCommonNtf.m_wMsgId = DT_GUILD_MSG_EXPEDITION_UPDATE_DEFEND_INFO;
	pstPkgNew->m_stBody.m_stGuildExpeditionCommonNtf.m_ullGuildId = m_stGuildInfo.m_ullGuildId;

	DT_GUILD_PKG_EXPEDITION_UPDATE_DEFEND_INFO& rstUpdateInfo = pstPkgNew->m_stBody.m_stGuildExpeditionCommonNtf.m_stNtfInfo.m_stExpeditionUpdateDefendInfo;
	rstUpdateInfo.m_bMemberCount = 0;
	if (m_iUpdateAllDefendInfoFreq++ % 20 == 0)
	{
		//每设置20次,全部同步更新一下
		for (int i = 0; i < m_stGuildInfo.m_chMemberCount; i++)
		{
			if (m_stGuildInfo.m_astMemberList[i].m_bIsDefend == 1)
			{
				rstUpdateInfo.m_MemberList[rstUpdateInfo.m_bMemberCount++] = m_stGuildInfo.m_astMemberList[i].m_ullUin;
			}
		}
	}
	else
	{
		rstUpdateInfo.m_MemberList[rstUpdateInfo.m_bMemberCount++] = ullUin;
	}
	GuildExpeditionSvrMsgLayer::Instance().SendToOtherSvrByGate(pstPkgNew);
}

void Guild::CheckSendAward(PKGMETA::DT_GUILD_EXPEDITION_MATCHED_GUILD_INFO* pstMatchedGuildInfo)
{
	uint8_t bType = 0;
	if (m_stGuildInfo.m_bPeriodAwardState == COMMON_DO_SOME_STATE_NONE &&
		m_iTotalDefeatNum * 100 >= LogicMgr::Instance().m_iPeriodDefeatRate * m_iTotalMatchedGuildMemberNum) // > 70%
	{
		m_stGuildInfo.m_bPeriodAwardState = COMMON_DO_SOME_STATE_FINISHED;
		bType |= 1;
	}

	if (pstMatchedGuildInfo->m_bPerfectAwardState == COMMON_DO_SOME_STATE_NONE && 
		pstMatchedGuildInfo->m_chMemberCount == m_oGuildId2DefeatNumMap[pstMatchedGuildInfo->m_ullGuildId])
	{
		pstMatchedGuildInfo->m_bPerfectAwardState = COMMON_DO_SOME_STATE_FINISHED;
		bType |= 2;
	}
	if (bType == 0)
	{
		return;
	}
	SSPKG* pstPkg = GuildExpeditionSvrMsgLayer::Instance().GetSendPkg();
	if (pstPkg == NULL)
	{
		return;
	}
	pstPkg->m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_COMMON_NTF;
	pstPkg->m_stHead.m_ullReservId = m_stGuildInfo.m_dwAddr;
	pstPkg->m_stBody.m_stGuildExpeditionCommonNtf.m_wMsgId = DT_GUILD_MSG_EXPEDITION_AWARD;
	pstPkg->m_stBody.m_stGuildExpeditionCommonNtf.m_ullGuildId = m_stGuildInfo.m_ullGuildId;
	pstPkg->m_stBody.m_stGuildExpeditionCommonNtf.m_stNtfInfo.m_stExpeditionAward.m_bAwardType = bType;
	LOGWARN("GuildId<%lu> send award<%d>", m_stGuildInfo.m_ullGuildId, (int)bType);
	GuildExpeditionSvrMsgLayer::Instance().SendToOtherSvrByGate(pstPkg);
}

PKGMETA::DT_GUILD_EXPEDITION_MATCHED_GUILD_INFO* Guild::FindMatchedGuild(uint64_t ullGuildId)
{
	for (int i = 0; i < m_stGuildInfo.m_chMatchedGuildCount; i++)
	{
		if (m_stGuildInfo.m_astMatchedGuildList[i].m_ullGuildId == ullGuildId)
		{
			return &m_stGuildInfo.m_astMatchedGuildList[i];
		}
	}
	
	return NULL;
}

PKGMETA::DT_GUILD_EXPEDITION_MATCHED_ONE_MEMBER_INFO* Guild::FindMatchedGuildMember(PKGMETA::DT_GUILD_EXPEDITION_MATCHED_GUILD_INFO* pstMatchedGuildInfo, uint64_t ullMatchedUin)
{
	DT_GUILD_EXPEDITION_MATCHED_ONE_MEMBER_INFO stOneMember = { 0 };
	stOneMember.m_ullUin = ullMatchedUin;
	int iEqual = 0;
	int iIndex = MyBSearch(&stOneMember, pstMatchedGuildInfo->m_astMemberList, pstMatchedGuildInfo->m_chMemberCount,
		sizeof(DT_GUILD_EXPEDITION_MATCHED_ONE_MEMBER_INFO), &iEqual, CmpMatchedMember);
	if (iEqual != 0)
	{
		return &pstMatchedGuildInfo->m_astMemberList[iIndex];
	}
	return NULL;
}

void Guild::Dissolve()
{
	DataMgr::Instance().DelFromMatchList(this);
	DT_GUILD_EXPEDITION_GUILD_UPLOAD_INFO stUploadInfo = { 0 };
	stUploadInfo.m_dwAddr = m_stGuildInfo.m_dwAddr;
	stUploadInfo.m_ullGuildId = m_stGuildInfo.m_ullGuildId;
	StrCpy(stUploadInfo.m_szName, m_stGuildInfo.m_szName, sizeof(stUploadInfo.m_szName));
	Reset();
	m_stGuildInfo.m_ullGuildId = stUploadInfo.m_ullGuildId;
	UptInfo(stUploadInfo);
	DataMgr::Instance().DelFromTimeList(this);
	DataMgr::Instance().AddToDirtyList(this);
}


void Guild::MemberQuit(uint64_t ullUin)
{
	DT_GUILD_EXPEDITION_ONE_MEMBER_INFO* pstMember = FindMember(ullUin);
	if (pstMember == NULL)
	{
		return;
	}
	//未防守 不做处理
	if (pstMember->m_bIsDefend != 1)
	{
		DelMember(ullUin);
		return;
	}
	Player* pstPlayer = DataMgr::Instance().GetPlayer(ullUin);
	if (pstPlayer == NULL)
	{
		LOGERR("GuildId<%lu> member<%lu> quit, but not find the PlayerObj", GetId(), ullUin);
		return;
	}

	uint32_t dwSubLi = pstPlayer->GetHighestLi();
	if (dwSubLi != 0 && m_iTotalDefendReadyNum >= LogicMgr::Instance().m_iLeastDefendNum)
	{
		DataMgr::Instance().DelFromMatchList(this);
	}
	if (m_stGuildInfo.m_ullGuildLi <  dwSubLi)
	{
		m_stGuildInfo.m_ullGuildLi = 0;
	}
	else
	{
		m_stGuildInfo.m_ullGuildLi -= dwSubLi;
	}
	m_iTotalDefendReadyNum--;
	if (dwSubLi != 0 && m_iTotalDefendReadyNum >= LogicMgr::Instance().m_iLeastDefendNum)
	{
		DataMgr::Instance().AddToMatchList(this);
	}
	DelMember(ullUin);

	DataMgr::Instance().DelFromTimeList(this);
	DataMgr::Instance().AddToDirtyList(this);
}

