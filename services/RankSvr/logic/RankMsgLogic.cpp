#include <vector>
#include "RankMsgLogic.h"
#include "LogMacros.h"
#include "../framework/RankSvrMsgLayer.h"
#include "../gamedata/GameDataMgr.h"
#include "../module/RankMgr.h"


using namespace PKGMETA;
using namespace std;

int Rank_6v6UpdateRankReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    int iRet = 0;
    SS_PKG_6V6_RANK_UPDATE_REQ & rstRankUpdateReq = rstSsPkg.m_stBody.m_stRank6V6UpdateReq;
    DT_RANK_INFO & rstRankInfo = rstRankUpdateReq.m_stReqInfo;
    Rank6V6Node stRankNode;
    stRankNode.SetRankInfo(rstRankInfo);
    iRet = RankMgr::Instance().GetRank6V6().UpdateRank(stRankNode);
    if (iRet < 0)
    {
        iRet = ERR_DEFAULT;
        LOGERR("update rank failed");
    }

    return 0;
}

int RankCommonUpdateNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    DT_RANK_ROLE_INFO& rstRankInfo = rstSsPkg.m_stBody.m_stRankCommonUpdateNtf.m_stReqInfo.m_stRankRoleInfo;
	DT_GUILD_RANK_INFO& rstGuildRankInfo = rstSsPkg.m_stBody.m_stRankCommonUpdateNtf.m_stReqInfo.m_stGuildRankInfo;
	DT_GFIGHT_RANK_INFO& rstGFightRankInfo = rstSsPkg.m_stBody.m_stRankCommonUpdateNtf.m_stReqInfo.m_stGFightRankInfo;
    DT_PEAK_ARENA_RANK_INFO& rstPeakArenaRankInfo = rstSsPkg.m_stBody.m_stRankCommonUpdateNtf.m_stReqInfo.m_stPeakArenaRankInfo;

	uint8_t bType;
	switch (rstSsPkg.m_stBody.m_stRankCommonUpdateNtf.m_bType)
	{
	case RANK_ROLE_INFO_TYPE:
		bType = rstRankInfo.m_bRankType;
		break;
	case GUILD_RANK_INFO_TYPE:
		bType = RANK_TYPE_GUILD;
		break;
	case GFIGHT_RANK_INFO_TYPE:
		bType = RANK_TYPE_GFTIGHT;
		break;
    case PEAK_ARENA_RANK_INFO_TYPE:
        bType = RANK_TYPE_PEAK_ARENA;
        break;
	default:
		return -1;
	}

    switch (bType)
    {
    case RANK_TYPE_LI:
        {
            RankRoleNode stRankNode;
            stRankNode.SetRankInfo(rstRankInfo);
            RankMgr::Instance().GetRankLi().UpdateRank(stRankNode);
        }
        break;
    case RANK_TYPE_PVE_STAR:
        {
            RankRoleNode stRankNode;
            stRankNode.SetRankInfo(rstRankInfo);
            RankMgr::Instance().GetRankPveStar().UpdateRank(stRankNode);
        }

        break;
    case RANK_TYPE_GCARD_CNT:
        {
            RankRoleNode stRankNode;
            stRankNode.SetRankInfo(rstRankInfo);
            RankMgr::Instance().GetRankGCardCnt().UpdateRank(stRankNode);
        }
        break;
    case RANK_TYPE_GCARD_LI:
        {
            RankGCardNode stGCardNode;
            stGCardNode.SetRankInfo(rstRankInfo);
            RankMgr::Instance().GetRankGCardLi().UpdateRank(stGCardNode);
        }
        break;
	case RANK_TYPE_GUILD:
		{
			GuildRankNode stGuildNode;
			stGuildNode.SetRankInfo(rstGuildRankInfo);
			RankMgr::Instance().GetRankGuild().UpdateRank(stGuildNode);
		}
		break;
	case RANK_TYPE_GFTIGHT:
		{
			GFightRankNode stGFightNode;
			stGFightNode.SetRankInfo(rstGFightRankInfo);
			RankMgr::Instance().GetRankGFight().UpdateRank(stGFightNode);
		}
		break;
    case RANK_TYPE_PEAK_ARENA:
        {
            PeakArenaRankNode stPeakArenaNode;
            stPeakArenaNode.SetRankInfo(rstPeakArenaRankInfo);
            RankMgr::Instance().GetRankPeakArena().UpdateRank(stPeakArenaNode);
        }
        break;
    default:
        //PVP 排行榜独立处理
        LOGERR("Uin<%lu> update rank type<%hhu> error", rstRankInfo.m_ullUin, rstRankInfo.m_bRankType);
        break;
    }
    return 0;
}

int RankCommonGetTopListReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
	SS_PKG_RANK_COMMON_GET_TOPLIST_REQ& rstSsBodyReqPkg = rstSsPkg.m_stBody.m_stRankCommonGetTopListReq;
    uint8_t bType = rstSsBodyReqPkg.m_bType;
	uint64_t ullVersion = rstSsBodyReqPkg.m_ullVersion;
	uint64_t ullUin = rstSsPkg.m_stHead.m_ullUin;
    uint64_t ullGuildId = rstSsBodyReqPkg.m_ullGuildId;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_RANK_COMMON_GET_TOPLIST_RSP;
    m_stSsPkg.m_stHead.m_ullUin = ullUin;
    SS_PKG_RANK_COMMON_GET_TOPLIST_RSP &rstRsp = m_stSsPkg.m_stBody.m_stRankCommonGetTopListRsp;
    int iRet = ERR_NONE;
    uint8_t bRankCount = 0;
	uint32_t dwSelfRank = 0;
	uint64_t ullRetVersion = 0;
    switch (bType)
    {
	case RANK_TYPE_PVP:
		{
			ullRetVersion = RankMgr::Instance().GetRank6V6().GetVersion();
			int iTmp = RankMgr::Instance().GetRank6V6().GetRank(ullUin);
			dwSelfRank = iTmp<0 ? 0 : iTmp;

			if (ullVersion >= ullRetVersion)
			{
				break;
			}

			DT_RANK_INFO oRankInfo[MAX_RANK_TOP_NUM];
			bRankCount = RankMgr::Instance().GetRank6V6().GetTopList(oRankInfo);

			for (int i=0; i<MAX_RANK_TOP_NUM; i++)
			{
				rstRsp.m_astTopList[i].m_stRankInfo = oRankInfo[i];
			}
			rstRsp.m_bRankInfoType = RANK_INFO_TYPE;
			break;
		}
    case RANK_TYPE_LI:
		{
			ullRetVersion = RankMgr::Instance().GetRankLi().GetVersion();
			int iTmp = RankMgr::Instance().GetRankLi().GetRank(ullUin);
			dwSelfRank = iTmp<0 ? 0 : iTmp;

			if (ullVersion >= ullRetVersion)
			{
				break;
			}

			DT_RANK_ROLE_INFO oRankRoleInfo[MAX_RANK_TOP_NUM];
			bRankCount = RankMgr::Instance().GetRankLi().GetTopList(oRankRoleInfo);
			for (int i=0; i<MAX_RANK_TOP_NUM; i++)
			{
				rstRsp.m_astTopList[i].m_stRankRoleInfo = oRankRoleInfo[i];
			}
			rstRsp.m_bRankInfoType = RANK_ROLE_INFO_TYPE;
			break;
		}
    case RANK_TYPE_PVE_STAR:
		{
			ullRetVersion = RankMgr::Instance().GetRankPveStar().GetVersion();
			int iTmp = RankMgr::Instance().GetRankPveStar().GetRank(ullUin);
			dwSelfRank = iTmp<0 ? 0 : iTmp;

			if (ullVersion >= ullRetVersion)
			{
				break;
			}

			DT_RANK_ROLE_INFO oRankRoleInfo[MAX_RANK_TOP_NUM];
			bRankCount = RankMgr::Instance().GetRankPveStar().GetTopList(oRankRoleInfo);
			for (int i=0; i<MAX_RANK_TOP_NUM; i++)
			{
				rstRsp.m_astTopList[i].m_stRankRoleInfo = oRankRoleInfo[i];
			}
			rstRsp.m_bRankInfoType = RANK_ROLE_INFO_TYPE;
			break;
		}
    case RANK_TYPE_GCARD_CNT:
		{
			ullRetVersion = RankMgr::Instance().GetRankGCardCnt().GetVersion();
			int iTmp = RankMgr::Instance().GetRankGCardCnt().GetRank(ullUin);
			dwSelfRank = iTmp<0 ? 0 : iTmp;

			if (ullVersion >= ullRetVersion)
			{
				break;
			}

			DT_RANK_ROLE_INFO oRankRoleInfo[MAX_RANK_TOP_NUM];
			bRankCount = RankMgr::Instance().GetRankGCardCnt().GetTopList(oRankRoleInfo);
			for (int i=0; i<MAX_RANK_TOP_NUM; i++)
			{
				rstRsp.m_astTopList[i].m_stRankRoleInfo = oRankRoleInfo[i];
			}
			rstRsp.m_bRankInfoType = RANK_ROLE_INFO_TYPE;
			break;
		}
    case RANK_TYPE_GCARD_LI:
		{
			ullRetVersion = RankMgr::Instance().GetRankGCardLi().GetVersion();

			RankGCardKey oRankGCardKey;
			oRankGCardKey.m_dwSeq = rstSsBodyReqPkg.m_dwGcardId;
			oRankGCardKey.m_ullUin = ullUin;
			int iTmp = RankMgr::Instance().GetRankGCardLi().GetRank(oRankGCardKey);
			dwSelfRank = iTmp<0 ? 0 : iTmp;

			if (ullVersion >= ullRetVersion)
			{
				break;
			}

			DT_RANK_ROLE_INFO oRankRoleInfo[MAX_RANK_TOP_NUM];
			bRankCount = RankMgr::Instance().GetRankGCardLi().GetTopList(oRankRoleInfo);
			for (int i=0; i<MAX_RANK_TOP_NUM; i++)
			{
				rstRsp.m_astTopList[i].m_stRankRoleInfo = oRankRoleInfo[i];
			}
			rstRsp.m_bRankInfoType = RANK_ROLE_INFO_TYPE;
			break;
		}
	case RANK_TYPE_DAILY_CHALLENGE:
		{
			ullRetVersion = RankMgr::Instance().GetRankDailyChallenge().GetVersion();
			int iTmp = RankMgr::Instance().GetRankDailyChallenge().GetRank(ullUin);
			dwSelfRank = iTmp<0 ? 0 : iTmp;

			if (ullVersion >= ullRetVersion)
			{
				break;
			}

			DT_DAILY_CHALLENGE_RANK_INFO oDailyChRankInfo[MAX_RANK_TOP_NUM];
			bRankCount = RankMgr::Instance().GetRankDailyChallenge().GetTopList(oDailyChRankInfo);
			for (int i=0; i<MAX_RANK_TOP_NUM; i++)
			{
				rstRsp.m_astTopList[i].m_stDailyChRankInfo = oDailyChRankInfo[i];
			}
			rstRsp.m_bRankInfoType = DAILY_CH_RANK_INFO_TYPE;
			break;
		}
	case RANK_TYPE_GUILD:
		{
			ullRetVersion = RankMgr::Instance().GetRankGuild().GetVersion();
			int iTmp = RankMgr::Instance().GetRankGuild().GetRank(ullGuildId);
			dwSelfRank = iTmp<0 ? 0 : iTmp;

			if (ullVersion >= ullRetVersion)
			{
				break;
			}

			DT_GUILD_RANK_INFO oGuildRankInfo[MAX_RANK_TOP_NUM];
			bRankCount = RankMgr::Instance().GetRankGuild().GetTopList(oGuildRankInfo);
			for (int i=0; i<MAX_RANK_TOP_NUM; i++)
			{
				rstRsp.m_astTopList[i].m_stGuildRankInfo = oGuildRankInfo[i];
			}
			rstRsp.m_bRankInfoType = GUILD_RANK_INFO_TYPE;
		}
		break;
	case RANK_TYPE_GFTIGHT:
		{
			ullRetVersion = RankMgr::Instance().GetRankGFight().GetVersion();
			int iTmp = RankMgr::Instance().GetRankGFight().GetRank(ullUin);
			dwSelfRank = iTmp<0 ? 0 : iTmp;

			if (ullVersion >= ullRetVersion)
			{
				break;
			}

			DT_GFIGHT_RANK_INFO oGFightRankInfo[MAX_RANK_TOP_NUM];
			bRankCount = RankMgr::Instance().GetRankGFight().GetTopList(oGFightRankInfo);
			for (int i=0; i<MAX_RANK_TOP_NUM; i++)
			{
				rstRsp.m_astTopList[i].m_stGFightRankInfo = oGFightRankInfo[i];
			}
			rstRsp.m_bRankInfoType = GFIGHT_RANK_INFO_TYPE;
		}
		break;
    case RANK_TYPE_PEAK_ARENA:
        {
            ullRetVersion = RankMgr::Instance().GetRankPeakArena().GetVersion();
			int iTmp = RankMgr::Instance().GetRankPeakArena().GetRank(ullUin);
			dwSelfRank = iTmp<0 ? 0 : iTmp;

			if (ullVersion >= ullRetVersion)
			{
				break;
			}

            DT_PEAK_ARENA_RANK_INFO astPeakArenaRankInfo[MAX_RANK_TOP_NUM];
			bRankCount = RankMgr::Instance().GetRankPeakArena().GetTopList(astPeakArenaRankInfo);
			for (int i=0; i<MAX_RANK_TOP_NUM; i++)
			{
				rstRsp.m_astTopList[i].m_stPeakArenaRankInfo = astPeakArenaRankInfo[i];
			}
			rstRsp.m_bRankInfoType = PEAK_ARENA_RANK_INFO_TYPE;
        }
        break;
    default:
        //PVP 排行榜独立处理
        LOGERR("Uin<%lu> get rank type<%hhu> error", rstSsPkg.m_stHead.m_ullUin, bType);
        iRet = ERR_SYS;
        break;
    }
    rstRsp.m_bCount = bRankCount;
    rstRsp.m_nErrNo = iRet;
	rstRsp.m_dwSelfRank = dwSelfRank;
	rstRsp.m_ullVersion = ullRetVersion;
	rstRsp.m_bType = bType;
    RankSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return 0;
}

int RankCommonGetRankReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    uint8_t bType = rstSsPkg.m_stBody.m_stRankCommonGetRankReq.m_bType;
    uint64_t ullUin = rstSsPkg.m_stBody.m_stRankCommonGetRankReq.m_ullUin;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_RANK_COMMON_GET_RANK_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    SS_PKG_RANK_COMMON_GET_RANK_RSP& rstRsp = m_stSsPkg.m_stBody.m_stRankCommonGetRankRsp;

    int iRank = 0;
    int iRet = ERR_NONE;
    switch (bType)
    {
    case RANK_TYPE_LI:
        iRank = RankMgr::Instance().GetRankLi().GetRank(ullUin);
        break;
    case RANK_TYPE_PVE_STAR:
        iRank = RankMgr::Instance().GetRankPveStar().GetRank(ullUin);
        break;
    case RANK_TYPE_GCARD_CNT:
        iRank = RankMgr::Instance().GetRankGCardCnt().GetRank(ullUin);
        break;
    default:
        //PVP 排行榜独立处理
        LOGERR("Uin<%lu> get rank type<%hhu> error", ullUin, bType);
        iRet = ERR_SYS;
        break;
    }

    rstRsp.m_nErrNo = iRet;
    rstRsp.m_dwRank = iRank <= 0 ? 0 : iRank;
    rstRsp.m_ullUin = ullUin;

    RankSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return 0;
}

int DailyChallengeUptRankNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_DAILY_CHALLENGE_UPDATE_RANK_NTF& rstNtf = rstSsPkg.m_stBody.m_stDailyChallengeUptRankNtf;
    RankDailyChallengeNode stRankNode;
    stRankNode.SetRankInfo(rstNtf.m_stSelfInfo);
    int iRet = RankMgr::Instance().GetRankDailyChallenge().UpdateRank(stRankNode);
    if (iRet < 0)
    {
        LOGERR("Player(%s) Uin(%lu) Update DailyChallenge rank failed", rstNtf.m_stSelfInfo.m_szRoleName, rstNtf.m_stSelfInfo.m_ullUin);
    }

    return ERR_NONE;
}

int DailyChallengeGetTopListReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_DAILY_CHALLENGE_GET_TOPLIST_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    SS_PKG_DAILY_CHALLENGE_GET_TOPLIST_RSP& rstRsp = m_stSsPkg.m_stBody.m_stDailyChallengeGetTopListRsp;
    rstRsp.m_nErrNo = ERR_NONE;

    rstRsp.m_dwSelfRank = RankMgr::Instance().GetRankDailyChallenge().GetRank(rstSsPkg.m_stHead.m_ullUin);
    rstRsp.m_bTopListCnt = RankMgr::Instance().GetRankDailyChallenge().GetTopList(rstRsp.m_astTopList);

    RankSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);

    return ERR_NONE;
}

int DailyChallengeSettleRankNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    RankMgr::Instance().GetRankDailyChallenge().SettleRank();
    return ERR_NONE;
}

int DailyChallengeClearRankNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    RankMgr::Instance().GetRankDailyChallenge().ClearRank();
    return ERR_NONE;
}

int LiRankSettleNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    RankMgr::Instance().GetRankLi().SettleRank();
    return ERR_NONE;
}

//int RankGuildCompetitorInfoReq_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
//{
//    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_RANK_GUILD_COMPETITOR_RSP;
//    uint64_t ullMyGuildId = rstSsPkg.m_stBody.m_stRankGuildGetCompetitorReq.m_ullMyGuildID;
//    uint16_t wMyGuildRank = RankMgr::Instance().GetRankGuild().GetRank(ullMyGuildId);
//    uint16_t wCompetitorGuildRank;
//    uint16_t wCurGuildRankNum = RankMgr::Instance().GetRankGuild().GetCurRankNum();
//    //防止排行榜为空
//    if (wCurGuildRankNum == 0)
//    {
//        m_stSsPkg.m_stBody.m_stRankGuildGetCompetitorRsp.m_nErrNo = ERR_NOT_EXIST_BY_GUILDID;
//        RankSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
//        return -1;
//    }
//    if ((wMyGuildRank & 1) == 0)
//    {
//        wCompetitorGuildRank = --wMyGuildRank;
//    }
//    else if (wMyGuildRank == wCurGuildRankNum)
//    {
//        wCompetitorGuildRank = wMyGuildRank;
//    }
//    else
//    {
//        wCompetitorGuildRank = ++wMyGuildRank;
//    }
//    SS_PKG_RANK_GUILD_COMPETITOR_RSP& rstSsRsp = m_stSsPkg.m_stBody.m_stRankGuildGetCompetitorRsp;
//    GuildRankNode* CompetitorGuild = RankMgr::Instance().GetRankGuild().GetRankNodeAtN(wCompetitorGuildRank);
//    if (CompetitorGuild == NULL)
//    {
//        return ERR_NOT_HAVE_GUILD;
//    }
//    rstSsRsp.m_ullCompetitorGuildID = CompetitorGuild->stScoreRankInfo.m_ullGuildId;
//    rstSsRsp.m_ullUin = rstSsPkg.m_stBody.m_stRankGuildGetCompetitorReq.m_ullUin;
//
//    RankSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
//
//    return 0;
//}

int NameChangeNtf_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
	uint64_t ullUin = rstSsPkg.m_stHead.m_ullUin;
	SS_PKG_NAME_CHANGE_NTF& rstSsBodyNtf = rstSsPkg.m_stBody.m_stChangeNameNtf;
	uint64_t ullGuildUin = rstSsBodyNtf.m_ullGuildId;
	int iTmp = 0;

	iTmp = RankMgr::Instance().GetRank6V6().GetRank(ullUin);
	do
	{
		if (iTmp<=0)
		{
			LOGRUN("Not in rank. iTmp<%d>", iTmp);
			break;
		}

		Rank6V6Node* poRankNode = RankMgr::Instance().GetRank6V6().GetRankNodeAtN(iTmp);
		if (poRankNode == NULL)
		{
			LOGRUN("poRankNod is not found. iTmp<%d>", iTmp);
			break;
		}

		DT_RANK_INFO& rstRankInfo = poRankNode->GetRankInfo();
		memcpy(rstRankInfo.m_szAccountName, rstSsBodyNtf.m_szName, MAX_NAME_LENGTH);

		if (iTmp <= MAX_RANK_TOP_NUM)
		{
			RankMgr::Instance().GetRank6V6().UpdateVersion();
		}

	} while (false);

	iTmp = RankMgr::Instance().GetRankLi().GetRank(ullUin);
	do
	{
		if (iTmp<=0)
		{
			LOGRUN("Not in rank. iTmp<%d>", iTmp);
			break;
		}

		RankRoleNode* poRankNode = RankMgr::Instance().GetRankLi().GetRankNodeAtN(iTmp);
		if (poRankNode == NULL)
		{
			LOGRUN("poRankNod is not found. iTmp<%d>", iTmp);
			break;
		}

		DT_RANK_ROLE_INFO& rstRankInfo = poRankNode->GetRankInfo();
		memcpy(rstRankInfo.m_szRoleName, rstSsBodyNtf.m_szName, MAX_NAME_LENGTH);

		if (iTmp <= MAX_RANK_TOP_NUM)
		{
			RankMgr::Instance().GetRankLi().UpdateVersion();
		}

	} while (false);

	iTmp = RankMgr::Instance().GetRankPveStar().GetRank(ullUin);
	do
	{
		if (iTmp<=0)
		{
			LOGRUN("Not in rank. iTmp<%d>", iTmp);
			break;
		}

		RankRoleNode* poRankNode = RankMgr::Instance().GetRankPveStar().GetRankNodeAtN(iTmp);
		if (poRankNode == NULL)
		{
			LOGRUN("poRankNod is not found. iTmp<%d>", iTmp);
			break;
		}

		DT_RANK_ROLE_INFO& rstRankInfo = poRankNode->GetRankInfo();
		memcpy(rstRankInfo.m_szRoleName, rstSsBodyNtf.m_szName, MAX_NAME_LENGTH);

		if (iTmp <= MAX_RANK_TOP_NUM)
		{
			RankMgr::Instance().GetRankPveStar().UpdateVersion();
		}
	} while (false);

	iTmp = RankMgr::Instance().GetRankGCardCnt().GetRank(ullUin);
	do
	{
		if (iTmp<=0)
		{
			LOGRUN("Not in rank. iTmp<%d>", iTmp);
			break;
		}

		RankRoleNode* poRankNode = RankMgr::Instance().GetRankGCardCnt().GetRankNodeAtN(iTmp);
		if (poRankNode == NULL)
		{
			LOGRUN("poRankNod is not found. iTmp<%d>", iTmp);
			break;
		}

		DT_RANK_ROLE_INFO& rstRankInfo = poRankNode->GetRankInfo();
		memcpy(rstRankInfo.m_szRoleName, rstSsBodyNtf.m_szName, MAX_NAME_LENGTH);

		if (iTmp <= MAX_RANK_TOP_NUM)
		{
			RankMgr::Instance().GetRankGCardCnt().UpdateVersion();
		}
	} while (false);

	iTmp = RankMgr::Instance().GetRankDailyChallenge().GetRank(ullUin);
	do
	{
		if (iTmp<=0)
		{
			LOGRUN("Not in rank. iTmp<%d>", iTmp);
			break;
		}

		RankDailyChallengeNode* poRankNode = RankMgr::Instance().GetRankDailyChallenge().GetRankNodeAtN(iTmp);
		if (poRankNode == NULL)
		{
			LOGRUN("poRankNod is not found. iTmp<%d>", iTmp);
			break;
		}

		DT_DAILY_CHALLENGE_RANK_INFO& rstRankInfo = poRankNode->GetRankInfo();
		memcpy(rstRankInfo.m_szRoleName, rstSsBodyNtf.m_szName, MAX_NAME_LENGTH);

		if (iTmp <= MAX_RANK_TOP_NUM)
		{
			RankMgr::Instance().GetRankDailyChallenge().UpdateVersion();
		}
	} while (false);

	//公会Uin
	do
	{
		if (ullGuildUin==0)
		{
			LOGRUN("GuildId is 0.");
			break;
		}

		iTmp = RankMgr::Instance().GetRankGuild().GetRank(ullGuildUin);

		if (iTmp<=0)
		{
			LOGRUN("Not in rank. iTmp<%d>", iTmp);
			break;
		}

		GuildRankNode* poRankNode = RankMgr::Instance().GetRankGuild().GetRankNodeAtN(iTmp);
		if (poRankNode == NULL)
		{
			LOGRUN("poRankNod is not found. iTmp<%d>", iTmp);
			break;
		}

		DT_GUILD_RANK_INFO& rstRankInfo = poRankNode->GetRankInfo();
		memcpy(rstRankInfo.m_szLeaderName, rstSsBodyNtf.m_szName, MAX_NAME_LENGTH);

		if (iTmp <= MAX_RANK_TOP_NUM)
		{
			RankMgr::Instance().GetRankGuild().UpdateVersion();
		}
	} while (false);

	//公会Uin
	do
	{
		if (ullGuildUin==0)
		{
			LOGRUN("GuildId is 0.");
			break;
		}

		iTmp = RankMgr::Instance().GetRankGFight().GetRank(ullGuildUin);

		if (iTmp<=0)
		{
			LOGRUN("Not in rank. iTmp<%d>", iTmp);
			break;
		}

		GFightRankNode* poRankNode = RankMgr::Instance().GetRankGFight().GetRankNodeAtN(iTmp);
		if (poRankNode == NULL)
		{
			LOGRUN("poRankNod is not found. iTmp<%d>", iTmp);
			break;
		}

		DT_GFIGHT_RANK_INFO& rstRankInfo = poRankNode->GetRankInfo();
		memcpy(rstRankInfo.m_szLeaderName, rstSsBodyNtf.m_szName, MAX_NAME_LENGTH);

		if (iTmp <= MAX_RANK_TOP_NUM)
		{
			RankMgr::Instance().GetRankGFight().UpdateVersion();
		}
	} while (false);

	iTmp = RankMgr::Instance().GetRankPeakArena().GetRank(ullUin);
	do
	{
		if (iTmp<=0)
		{
			LOGRUN("Not in rank. iTmp<%d>", iTmp);
			break;
		}

		PeakArenaRankNode* poRankNode = RankMgr::Instance().GetRankPeakArena().GetRankNodeAtN(iTmp);
		if (poRankNode == NULL)
		{
			LOGRUN("poRankNod is not found. iTmp<%d>", iTmp);
			break;
		}

		DT_PEAK_ARENA_RANK_INFO& rstRankInfo = poRankNode->GetRankInfo();
		memcpy(rstRankInfo.m_szRoleName, rstSsBodyNtf.m_szName, MAX_NAME_LENGTH);

		if (iTmp <= MAX_RANK_TOP_NUM)
		{
			RankMgr::Instance().GetRankPeakArena().UpdateVersion();
		}
	} while (false);

	for (int i = 0; i < rstSsBodyNtf.m_bGeneralCount; i++)
	{
		RankGCardKey oKey;
		oKey.m_ullUin = ullUin;
		oKey.m_dwSeq = rstSsBodyNtf.m_GeneralList[i];
		iTmp = RankMgr::Instance().GetRankGCardLi().GetRank(oKey);
		do
		{
			if (iTmp<=0)
			{
				LOGRUN("Not in rank. iTmp<%d>, uin<%lu>, generalId<%d>", iTmp, oKey.m_ullUin, oKey.m_dwSeq);
				break;
			}

			RankGCardNode* poRankNode = RankMgr::Instance().GetRankGCardLi().GetRankNodeAtN(iTmp);
			if (poRankNode == NULL)
			{
				LOGRUN("Not in rank. iTmp<%d>, uin<%lu>, generalId<%d>", iTmp, oKey.m_ullUin, oKey.m_dwSeq);
				break;
			}

			DT_RANK_ROLE_INFO& rstRankInfo = poRankNode->GetRankInfo();
			memcpy(rstRankInfo.m_szRoleName, rstSsBodyNtf.m_szName, MAX_NAME_LENGTH);

			if (iTmp <= MAX_RANK_TOP_NUM)
			{
				RankMgr::Instance().GetRankGCardLi().UpdateVersion();
			}
		} while (false);
	}

}

int RankGuildGetRankListReq_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_BOSS_GET_RANK_LIST_RSP;
    SS_PKG_GUILD_BOSS_GET_RANK_LIST_RSP& rstRsp = m_stSsPkg.m_stBody.m_stGuildBossGetRankListRsp;

    DT_GUILD_RANK_INFO oGuildRankInfo[MAX_LEN_ROLE_GUILD];
    rstRsp.m_wCount = RankMgr::Instance().GetRankGuild().GetCurRankNum();
    for (int i = 0; i != rstRsp.m_wCount; ++i)
    {
        rstRsp.m_astRankList[i] = oGuildRankInfo[i];
    }

    RankSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return ERR_NONE;
}

int PeakArenaSettleNtf_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    RankMgr::Instance().GetRankPeakArena().SettleRank();
    RankMgr::Instance().GetRankPeakArena().ClearRank();
    return ERR_NONE;
}

