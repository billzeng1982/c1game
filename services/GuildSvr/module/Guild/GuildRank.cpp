#include "GuildRank.h"
#include "../../gamedata/GameDataMgr.h"
#include "PKGMETA_metalib.h"

using namespace PKGMETA;
using namespace std;

GuildRankMgr::GuildRankMgr()
{
}

bool GuildRankMgr::Init()
{
	if(!m_GuildRankSys.Init("GuildRankTable", PKGMETA::MetaLib::getVersion()))
	{
		LOGERR_r("GuildRankMgr Init Failed");
		return false;
	}
	m_iGuildRankMinScore = 0;
	m_iGuildTopMaxNum = MIN((int)CGameDataMgr::Instance().GetResBasicMgr().Find((int)RANK_TOP_MAX_NUM)->m_para[0], MAX_GUILD_RANK_TOP_NUM);
	return true;
}

int GuildRankMgr::UpdateRank(DT_GUILD_RANK_INFO& rstRankInfo)
{
	rstRankInfo.m_ullTimeStampMs = CGameTime::Instance().GetCurrTimeMs();
	memcpy(&m_stGuildRankNode.stScoreRankInfo, &rstRankInfo, sizeof(rstRankInfo));
	int iRet = m_GuildRankSys.Update(rstRankInfo.m_ullGuildId, m_stGuildRankNode);
	if (iRet<=MAX_RANK_TOP_NUM)
	{
		m_ullVersion = CGameTime::Instance().GetCurrTimeMs();
	}


	return 0;
}


int GuildRankMgr::GetTopList(DT_GUILD_RANK_INFO* pstTopList)
{
	m_GuildTopVector.clear();
	int count = m_GuildRankSys.GetTopObj(m_iGuildTopMaxNum, m_GuildTopVector);
	for (int i=0; i<count; i++)
	{
		pstTopList[i] = m_GuildTopVector[i].GetRankInfo();
		pstTopList[i].m_dwRank = i+1;
	}
	m_iGuildRankMinScore = pstTopList[count - 1].m_dwStarNum;
	m_iCurGuildRankNum = count;
	return count;
}

//从排行榜中删除
void GuildRankMgr::Delete(uint64_t ullGuildId)
{
	m_GuildRankSys.Delete(ullGuildId);
}

//奖励排行榜
void GuildRankMgr::RewardRank()
{
	LOGRUN("GuildRank reward ok");
}

//重置
void GuildRankMgr::Reset()
{
	m_iGuildRankMinScore = 0;	//排行榜上的最低分数
	m_iCurGuildRankNum = 0;		//当前排行榜人数
	m_GuildRankSys.ClearRank();
	LOGRUN("GuildRank clear ok");
}

//向ranksvr发送update数据，本地可以不再使用
void GuildRankMgr::UpdateGuildRankNtf(Guild* poGuild, DT_GUILD_RANK_INFO& rstGuildRankInfo)
{
	//m_stSsPkg.m_stHead.m_ullUin = poGuild->GetGuildId();
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_RANK_COMMON_UPDATE_NTF;
	SS_PKG_RANK_COMMON_UPDATE_NTF& rstRankUpdateNtf = m_stSsPkg.m_stBody.m_stRankCommonUpdateNtf;
	memcpy(&rstRankUpdateNtf.m_stReqInfo.m_stGuildRankInfo, &rstGuildRankInfo, sizeof(DT_GUILD_RANK_INFO));
	rstRankUpdateNtf.m_bType = GUILD_RANK_INFO_TYPE;

	GuildSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);
}




/******************************军团战积分排行榜***************************************************************/

bool GFightRankMgr::Init()
{

	m_iTopMaxNum = MIN((int)CGameDataMgr::Instance().GetResBasicMgr().Find((int)RANK_TOP_MAX_NUM)->m_para[0], MAX_GUILD_RANK_TOP_NUM);
    if (!m_RankSys.Init("GFightRankTable", PKGMETA::MetaLib::getVersion()))
    {
        LOGERR("GuildRankMgr Init Failed");
        return false;
    }
	return true;
}

int GFightRankMgr::UpdateRank(DT_GFIGHT_RANK_INFO& rstRankInfo)
{
	memcpy(&m_stRankNode.stScoreRankInfo, &rstRankInfo, sizeof(rstRankInfo));
	rstRankInfo.m_ullTimeStampMs = CGameTime::Instance().GetCurrTimeMs();
	m_RankSys.Update(rstRankInfo.m_ullGuildId, m_stRankNode);
	return 0;
}

int GFightRankMgr::GetTopList(DT_GFIGHT_RANK_INFO* pstTopList)
{
	m_TopVector.clear();
	int count = m_RankSys.GetTopObj(m_iTopMaxNum, m_TopVector);
	for (int i=0; i<count; i++)
	{
		pstTopList[i] = m_TopVector[i].stScoreRankInfo;
		pstTopList[i].m_dwRank = i+1;
	}
	return count;
}

//从排行榜中删除
void GFightRankMgr::Delete(uint64_t ullGuildId)
{
	m_RankSys.Delete(ullGuildId);
}

void GFightRankMgr::UpdateGFightRankNtf(Guild* poGuild, DT_GFIGHT_RANK_INFO& rstGFightRankInfo)
{
	//m_stSsPkg.m_stHead.m_ullUin = poGuild->GetGuildId();
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_RANK_COMMON_UPDATE_NTF;
	SS_PKG_RANK_COMMON_UPDATE_NTF& rstRankUpdateNtf = m_stSsPkg.m_stBody.m_stRankCommonUpdateNtf;
	memcpy(&rstRankUpdateNtf.m_stReqInfo.m_stGFightRankInfo, &rstGFightRankInfo, sizeof(DT_GFIGHT_RANK_INFO));
	rstRankUpdateNtf.m_bType = GFIGHT_RANK_INFO_TYPE;

	GuildSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);
}

