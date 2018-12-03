#include "MsgLogicGuild.h"
#include "LogMacros.h"
#include "ss_proto.h"
#include "cs_proto.h"
#include "GameTime.h"
#include "ov_res_public.h"
#include "dwlog_svr.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../module/player/Player.h"
#include "../module/player/PlayerMgr.h"
#include "../module/player/PlayerData.h"
#include "../module/Guild.h"
#include "../module/Task.h"
#include "../module/Match.h"
#include "../module/Majesty.h"
#include "../module/Marquee.h"
#include "../module/ZoneLog.h"
#include "../module/GloryItemsMgr.h"
#include "../module/GuildGeneralHang.h"
#include "../module/Item.h"
#include "PKGMETA_metalib.h"

using namespace PKGMETA;
using namespace DWLOG;

//CS协议
int JoinGuildReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    int iErrNo = Majesty::Instance().IsArriveLevel(&poPlayer->GetPlayerData(), LEVEL_LIMIT_GUILD);
    if (ERR_NONE != iErrNo)
    {
        //沒有到特定的等級
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_JOIN_GUILD_RSP;
        SC_PKG_JOIN_GUILD_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stJoinGuildRsp;
        rstScPkgBodyRsp.m_nErrNo = iErrNo;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return iErrNo;
    }

    CS_PKG_JOIN_GUILD_REQ & rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stJoinGuildReq;

    //检查能否加入公会
    int iRet = Guild::Instance().CheckJoinGuild(&poPlayer->GetPlayerData());
    if (iRet != ERR_NONE)
    {
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_JOIN_GUILD_RSP;
        SC_PKG_JOIN_GUILD_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stJoinGuildRsp;
        rstScPkgBodyRsp.m_nErrNo = iRet;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return -1;
    }

    //发给GuildSvr处理
    SS_PKG_JOIN_GUILD_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stJoinGuildReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_JOIN_GUILD_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    rstSsPkgBodyReq.m_ullGuildId = rstCsPkgBodyReq.m_ullGuildId;
    rstSsPkgBodyReq.m_bIsApply = rstCsPkgBodyReq.m_bIsApply;

    DT_ONE_GUILD_MEMBER & rstGuildMember = rstSsPkgBodyReq.m_stPlayerInfo;
    //初始化公会成员信息
    Guild::Instance().InitMemberInfo(&poPlayer->GetPlayerData(),rstGuildMember);
    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);

    return 0;
}

int QuitGuildReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);

    if (!poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    //直接发给GuildSvr处理
    CS_PKG_QUIT_GUILD_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stQuitGuildReq;
    SS_PKG_QUIT_GUILD_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stQuitGuildReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_QUIT_GUILD_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    rstSsPkgBodyReq.m_ullGuildId = rstCsPkgBodyReq.m_ullGuildId;

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

int GuildSetNeedApplyReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);

    if (!poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    CS_PKG_GUILD_SET_NEED_APPLY_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stGuildSetNeedApplyReq;
    SS_PKG_GUILD_SET_NEED_APPLY_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stGuildSetNeedApplyReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_SET_NEED_APPLY_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    rstSsPkgBodyReq.m_ullGuildId = rstCsPkgBodyReq.m_ullGuildId;
    rstSsPkgBodyReq.m_bNeedApply = rstCsPkgBodyReq.m_bNeedApply;

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);

    return 0;
}

int GuildSetLevelLimitReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGRUN("player not exist.");
		return -1;
	}

	CS_PKG_GUILD_SET_LEVEL_LIMIT_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stGuildSetLevelLimitReq;
	SS_PKG_GUILD_SET_LEVEL_LIMIT_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stGuildSetLevelLimitReq;
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_SET_LEVEL_LIMIT_REQ;
	m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
	rstSsPkgBodyReq.m_ullGuildId = rstCsPkgBodyReq.m_ullGuildId;
	rstSsPkgBodyReq.m_bLevelLimit = rstCsPkgBodyReq.m_bLevelLimit;

	ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);

	return 0;

}

int CreateGuildReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    int iErrNo = Majesty::Instance().IsArriveLevel(&poPlayer->GetPlayerData(), LEVEL_LIMIT_GUILD);

    if (ERR_NONE != iErrNo)
    {
        //沒有到特定的等級
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CREATE_GUILD_RSP;
        SC_PKG_CREATE_GUILD_RSP& rstSsPkgRsp = m_stScPkg.m_stBody.m_stCreateGuildRsp;
        rstSsPkgRsp.m_nErrNo = iErrNo;
		rstSsPkgRsp.m_stGuildWholeData.m_stBossBlob.m_iLen = 0;
		rstSsPkgRsp.m_stGuildWholeData.m_stApplyBlob.m_iLen = 0;
		rstSsPkgRsp.m_stGuildWholeData.m_stGlobalBlob.m_iLen = 0;
		rstSsPkgRsp.m_stGuildWholeData.m_stMemberBlob.m_iLen = 0;
		rstSsPkgRsp.m_stGuildWholeData.m_stReplayBlob.m_iLen = 0;
		rstSsPkgRsp.m_stGuildWholeData.m_stSocietyBlob.m_iLen = 0;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return iErrNo;
    }
    //检查能否创建公会
    int iRet = Guild::Instance().CheckCreateGuild(&poPlayer->GetPlayerData());
    if (iRet != ERR_NONE)
    {
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CREATE_GUILD_RSP;
        SC_PKG_CREATE_GUILD_RSP& rstSsPkgRsp = m_stScPkg.m_stBody.m_stCreateGuildRsp;
        rstSsPkgRsp.m_nErrNo = iRet;
		rstSsPkgRsp.m_stGuildWholeData.m_stBossBlob.m_iLen = 0;
		rstSsPkgRsp.m_stGuildWholeData.m_stApplyBlob.m_iLen = 0;
		rstSsPkgRsp.m_stGuildWholeData.m_stGlobalBlob.m_iLen = 0;
		rstSsPkgRsp.m_stGuildWholeData.m_stMemberBlob.m_iLen = 0;
		rstSsPkgRsp.m_stGuildWholeData.m_stReplayBlob.m_iLen = 0;
		rstSsPkgRsp.m_stGuildWholeData.m_stSocietyBlob.m_iLen = 0;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return -1;
    }

    //发给GuildSvr处理
    CS_PKG_CREATE_GUILD_REQ & rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stCreateGuildReq;
    SS_PKG_CREATE_GUILD_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stCreateGuildReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_CREATE_GUILD_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    m_stSsPkg.m_stHead.m_ullReservId = (uint64_t)poPlayer->GetProtocolVersion();
    memcpy(&rstSsPkgBodyReq.m_szName, &rstCsPkgBodyReq.m_szName, DWLOG::MAX_NAME_LENGTH);
    DT_ONE_GUILD_MEMBER & rstGuildMember = rstSsPkgBodyReq.m_stLeaderInfo;

    //初始化公会成员信息
    Guild::Instance().InitMemberInfo(&poPlayer->GetPlayerData(),rstGuildMember);
    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

int DissolveGuildReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    //直接发给GuildSvr处理
    CS_PKG_DISSOLVE_GUILD_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stDissolveGuildReq;
    SS_PKG_DISSOLVE_GUILD_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stDissolveGuildReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_DISSOLVE_GUILD_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    rstSsPkgBodyReq.m_ullGuildId = rstCsPkgBodyReq.m_ullGuildId;

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

int RefreshGuildListReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    //直接发给GuildSvr处理
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_REFRESH_GUILDLIST_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

int GetGuildDetailReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_DETAILS_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    m_stSsPkg.m_stHead.m_ullReservId = (uint64_t)poPlayer->GetProtocolVersion();

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

int GuildDealApplyReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);

    if (!poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    //直接发给GuildSvr处理
    SS_PKG_GUILD_DEAL_APPLICATION_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stGuildDealApplyReq;
    CS_PKG_GUILD_DEAL_APPLICATION_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stGuildDealApplicationReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_DEAL_APPLICATION_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    rstSsPkgBodyReq.m_ullGuildId = rstCsPkgBodyReq.m_ullGuildId;
    rstSsPkgBodyReq.m_ullApplicantUin = rstCsPkgBodyReq.m_ullApplicantUin;
    rstSsPkgBodyReq.m_bIsAgreed = rstCsPkgBodyReq.m_bIsAgreed;
    if (1 == rstCsPkgBodyReq.m_bIsAgreed)
    {
        Player* poTmpPlayer = PlayerMgr::Instance().GetPlayerByUin(rstCsPkgBodyReq.m_ullApplicantUin);
        //记录军团日志
        ZoneLog::Instance().WriteGuildLog(&poPlayer->GetPlayerData(), METHOD_GUILD_OP_DEAL_APPLY, rstCsPkgBodyReq.m_ullGuildId, poTmpPlayer == NULL ? NULL : poTmpPlayer->GetRoleName()
                                            , rstCsPkgBodyReq.m_ullApplicantUin, 0);
    }
    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);

    return 0;
}

//  修改公告请求
int UpdateGulidNotice_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    //直接发给GuildSvr处理
    SS_PKG_UPDATE_GUILD_NOTICE_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stUpdateGuildNoticeReq;
    CS_PKG_UPDATE_GUILD_NOTICE_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stUpdateGuildNoticeReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_UPDATE_GUILD_NOTICE_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    rstSsPkgBodyReq.m_ullGuildId = rstCsPkgBodyReq.m_ullGuildId;
    StrCpy(rstSsPkgBodyReq.m_szGulidNotice, rstCsPkgBodyReq.m_szGulidNotice, PKGMETA::MAX_MSG_LENGTH);

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

//设置公会成员职位请求
int SetGuildMemJob_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    //直接发给GuildSvr处理
    SS_PKG_SET_GUILD_MEM_JOB_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stSetGuildMemJobReq;
    CS_PKG_SET_GUILD_MEM_JOB_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stSetGuildMemJobReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_SET_GUILD_MEM_JOB_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    rstSsPkgBodyReq.m_ullGuildId = rstCsPkgBodyReq.m_ullGuildId;
    rstSsPkgBodyReq.m_bMemJob = rstCsPkgBodyReq.m_bMemJob;
    rstSsPkgBodyReq.m_ullMemUin = rstCsPkgBodyReq.m_ullMemUin;

    Player* poTmpPlayer = PlayerMgr::Instance().GetPlayerByUin(rstCsPkgBodyReq.m_ullMemUin);
    ZoneLog::Instance().WriteGuildLog(&poPlayer->GetPlayerData(), METHOD_GUILD_OP_SET_MEM_JOB, rstCsPkgBodyReq.m_ullGuildId, poTmpPlayer == NULL ? NULL : poTmpPlayer->GetRoleName(),
                    rstCsPkgBodyReq.m_ullMemUin, rstCsPkgBodyReq.m_bMemJob);

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

//踢人请求
int GuildKickPlayer_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    //直接发给GuildSvr处理
    CS_PKG_GUILD_KICK_PLAYER_REQ& rstCsKickPlayerReq = rstCsPkg.m_stBody.m_stGuildKickPlayerReq;
    SS_PKG_GUILD_KICK_PLAYER_REQ& rstSsKickPlayerReq = m_stSsPkg.m_stBody.m_stGuildKickPlayerReq;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_KICK_PLAYER_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    rstSsKickPlayerReq.m_ullGuildId= rstCsKickPlayerReq.m_ullGuildId;
    rstSsKickPlayerReq.m_ullPlayerId= rstCsKickPlayerReq.m_ullPlayerId;

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);

    return 0;
}

//获取公会任务请求
int GuildGetTask_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);

    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    SC_PKG_GUILD_GET_GUILDTASK_RSP& rstScGetGuildTaskRsp = m_stScPkg.m_stBody.m_stGetGuildTaskRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_GET_GUILDTASK_RSP;

    DT_ROLE_GUILD_INFO& rstGuildInfo = poPlayer->GetPlayerData().GetGuildInfo();
    rstScGetGuildTaskRsp.m_bTaskCount = rstGuildInfo.m_bGuildTaskCount;
    for (int i=0; i<rstScGetGuildTaskRsp.m_bTaskCount; i++)
    {
        rstScGetGuildTaskRsp.m_astTaskList[i] = rstGuildInfo.m_astGuildTaskList[i];
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

//搜索公会请求
int SearchGuildReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    //直接发给GuildSvr处理
    CS_PKG_SEARCH_GUILD_REQ& rstCsSearchReq = rstCsPkg.m_stBody.m_stSearchGuildReq;
    SS_PKG_SEARCH_GUILD_REQ& rstSsSearchReq = m_stSsPkg.m_stBody.m_stSearchGuildReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_SEARCH_GUILD_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();

    StrCpy(rstSsSearchReq.m_szGuildName, rstCsSearchReq.m_szGuildName, DWLOG::MAX_NAME_LENGTH);

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

////公会商店购买请求
//int GuildPurchaseReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
//{
//    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
//    if (!poPlayer)
//    {
//        LOGERR("player not exist.");
//        return -1;
//    }
//
//    CS_PKG_GUILD_MARKET_PURCHASE_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildMarketPurchaseReq;
//    SC_PKG_GUILD_MARKET_PURCHASE_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildMarketPurchaseRsp;
//    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_MARKET_PURCHASE_RSP;
//
//    int iRet = Guild::Instance().PurChase(&poPlayer->GetPlayerData(), rstCsPkgReq, rstScPkgRsp);
//    rstScPkgRsp.m_nErrNo = iRet;
//
//    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
//
//    if (iRet == ERR_NONE)
//    {
//        // 任务记数修改
//        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_GUILD_OTHER, 1/*value*/, 3);
//
//        //公会日志
//        ZoneLog::Instance().WriteGuildLog(&poPlayer->GetPlayerData(), METHOD_GUILD_OP_PURCHASE, poPlayer->GetPlayerData().GetGuildInfo().m_ullGuildId, NULL, 0, rstCsPkgReq.m_bItemIndex);
//    }
//
//    return 0;
//}

//公会升级请求
int GuildLvUpReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    //直接发给GuildSvr处理
    CS_PKG_GUILD_LEVELUP_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildLevelupReq;
    SS_PKG_GUILD_LEVELUP_REQ& rstSsPkgReq = m_stSsPkg.m_stBody.m_stGuildLvUpReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_LEVELUP_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();

    rstSsPkgReq.m_ullGuildId = rstCsPkgReq.m_ullGuildId;

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

//公会捐赠
int GuildDonateReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_GUILD_DONATE_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildDonateReq;
    SC_PKG_GUILD_DONATE_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildDonateRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_DONATE_RSP;

    int iRet = Guild::Instance().Donate(&poPlayer->GetPlayerData(), rstCsPkgReq, rstScPkgRsp);
    rstScPkgRsp.m_nErrNo = iRet;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    if (iRet == ERR_NONE)
    {
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_GUILD_OTHER, 1, 4);

        if (rstCsPkgReq.m_bDonateType == 1)
        {
            //捐赠金币
            ZoneLog::Instance().WriteGuildLog(&poPlayer->GetPlayerData(), METHOD_GUILD_OP_DONATE_GOLD, rstCsPkgReq.m_ullGuildId, NULL, 0, poPlayer->GetPlayerData().GetGuildInfo().m_bGuildDonateTimes);
        }
        else if (rstCsPkgReq.m_bDonateType == 2)
        {
            //捐赠钻石
            ZoneLog::Instance().WriteGuildLog(&poPlayer->GetPlayerData(), METHOD_GUILD_OP_DONATE_DIAMAND, rstCsPkgReq.m_ullGuildId, NULL, 0, poPlayer->GetPlayerData().GetGuildInfo().m_bGuildDonateTimes);
        }
		else if (rstCsPkgReq.m_bDonateType == 3)
		{
			//VIP捐赠钻石
			ZoneLog::Instance().WriteGuildLog(&poPlayer->GetPlayerData(), METHOD_GUILD_OP_DONATE_DIAMAND, rstCsPkgReq.m_ullGuildId, NULL, 0, poPlayer->GetPlayerData().GetGuildInfo().m_bGuildDonateTimes);
		}
    }

    return 0;
}

//获取军团排行榜
int GuildGetGuildRankReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_GUILD_GET_GUILD_RANK_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildGetGuildRankReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_GET_GUILD_RANK_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
	m_stSsPkg.m_stBody.m_stGuildGetGuildRankReq.m_ullVersion = rstCsPkgReq.m_ullVersion;
    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

//获取军团积分排行榜
int GuildGetGFightRankReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    //CS_PKG_GUILD_GET_GFIGHT_RANK_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildGetGFightRankReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_GET_GFIGHT_RANK_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

//刷新军团商店或任务
int GuildResetMarketOrTaskReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_GUILD_RESET_MARKET_OR_TASK_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildResetMarketOrTaskReq;
    SC_PKG_GUILD_RESET_MARKET_OR_TASK_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildResetMarketOrTaskRsp;
    rstScPkgRsp.m_nErrNo = Guild::Instance().ResetMarketOrTask(&poPlayer->GetPlayerData(), rstCsPkgReq, rstScPkgRsp);

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_RESET_MARKET_OR_TASK_RSP;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int GuildActivityReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);

    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_ACTIVITY_REWARD_RSP;
    CS_PKG_GUILD_ACTIVITY_REWARD_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildActivityRewardReq;
    SC_PKG_GUILD_ACTIVITY_REWARD_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildActivityRewardRsp;
    rstScPkgRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;


    rstScPkgRsp.m_nErrNo = Guild::Instance().ActivityRecvReward(&poPlayer->GetPlayerData(), rstCsPkgReq.m_dwId, rstScPkgRsp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

//军团战报名
int GuildFightApplyReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_GUILD_FIGHT_APPLY_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildFightApplyReq;
    SS_PKG_GUILD_FIGHT_APPLY_REQ& rstSsPkgReq = m_stSsPkg.m_stBody.m_stGuildFightApplyReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_APPLY_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();

    rstSsPkgReq.m_ullGuildId = rstCsPkgReq.m_ullGuildId;
    rstSsPkgReq.m_dwApplyFund = rstCsPkgReq.m_dwApplyFund;

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

//请求报名列表
int GuildFightGetApplyListReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_GUILD_FIGHT_GET_APPLY_LIST_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildFightGetApplyListReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_FIGHT_GET_APPLY_LIST_RSP;
    SC_PKG_GUILD_FIGHT_GET_APPLY_LIST_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildFightGetApplyListRsp;
    Guild::Instance().GetGuildFightApplyList(rstCsPkgReq.m_ullGuildId, rstScPkgRsp);
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

//请求对阵表
int GuildFightGetAgainstInfoReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    SC_PKG_GUILD_FIGHT_GET_AGAINST_INFO_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildFightGetAgainstInfoRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_FIGHT_GET_AGAINST_INFO_RSP;

    Guild::Instance().GetGuildFightAgainstInfo(rstScPkgRsp.m_stAgainstInfo);
    rstScPkgRsp.m_nErrNo = ERR_NONE;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

// 进入战场
int GuildFightArenaJoinReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_GUILD_FIGHT_ARENA_JOIN_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildFightArenaJoinReq;
    SS_PKG_GUILD_FIGHT_ARENA_JOIN_REQ& rstSsPkgReq = m_stSsPkg.m_stBody.m_stGuildFightArenaJoinReq;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_ARENA_JOIN_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();

    rstSsPkgReq.m_stJoinerInfo = rstCsPkgReq.m_stJoinerInfo;
    rstSsPkgReq.m_bType = rstCsPkgReq.m_bType;

    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = poPlayer->GetPlayerData().GetMajestyInfo();

	rstSsPkgReq.m_wHeadIcon = rstMajestyInfo.m_wIconId;
	rstSsPkgReq.m_wHeadFrame = rstMajestyInfo.m_wFrameId;
	rstSsPkgReq.m_wHeadTitile = rstMajestyInfo.m_wTitleId;

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);

    return 0;
}

// 退出战场
int GuildFightArenaQuitReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_GUILD_FIGHT_ARENA_QUIT_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildFightArenaQuitReq;
    SS_PKG_GUILD_FIGHT_ARENA_QUIT_REQ& rstSsPkgReq = m_stSsPkg.m_stBody.m_stGuildFightArenaQuitReq;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_ARENA_QUIT_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();

    rstSsPkgReq.m_stQuiterInfo = rstCsPkgReq.m_stQuiterInfo;

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

//战场移动
int GuildFightArenaMoveReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_GUILD_FIGHT_ARENA_MOVE_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildFightArenaMoveReq;
    SS_PKG_GUILD_FIGHT_ARENA_MOVE_REQ& rstSsPkgReq = m_stSsPkg.m_stBody.m_stGuildFightArenaMoveReq;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_ARENA_MOVE_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();

    rstSsPkgReq.m_stPlayerInfo = rstCsPkgReq.m_stPlayerInfo;
    rstSsPkgReq.m_wCampId = rstCsPkgReq.m_wCampId; //移动的目的地

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 1;
}

//退出匹配
int GuildFightQuitMatchReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_GUILD_FIGHT_QUIT_MATCH_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildFightQuitMatchReq;
    SS_PKG_GUILD_FIGHT_QUIT_MATCH_REQ& rstSsPkgReq = m_stSsPkg.m_stBody.m_stGuildFightQuitMatchReq;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_QUIT_MATCH_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();

    rstSsPkgReq.m_stQuiterInfo = rstCsPkgReq.m_stQuiterInfo;

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 1;
}

//退出无敌模式
int GuildFightCancleGodReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_GUILD_FIGHT_QUIT_MATCH_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildFightQuitMatchReq;
    SS_PKG_GUILD_FIGHT_QUIT_MATCH_REQ& rstSsPkgReq = m_stSsPkg.m_stBody.m_stGuildFightQuitMatchReq;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_CANCLE_GOD_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();

    rstSsPkgReq.m_stQuiterInfo = rstCsPkgReq.m_stQuiterInfo;

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 1;
}

int GuildBossResetReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    Guild::Instance().GuildBossReset(&poPlayer->GetPlayerData());
    return 0;
}

int GuildBossGetKilledAwardReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    Guild::Instance().GuildBossGetKilledAward(&poPlayer->GetPlayerData());
    return 0;
}

int GuildBossCompetitorInfoReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (poPlayer == NULL)
    {
        LOGERR("player not exist.");
        return -1;
    }
    PKGMETA::SSPKG rstSsPkg;
    rstSsPkg.m_stHead.m_wMsgId = SS_MSG_RANK_GUILD_COMPETITOR_REQ;
    SS_PKG_RANK_GUILD_COMPETITOR_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stRankGuildGetCompetitorReq;
    uint64_t ullMyGuildId = poPlayer->GetPlayerData().GetGuildInfo().m_ullGuildId;
    rstSsPkgReq.m_ullMyGuildID = ullMyGuildId;
    rstSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    ZoneSvrMsgLayer::Instance().SendToGuildSvr(rstSsPkg);
    return 0;
}

int GuildBossSingleRewardReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);

    LOGWARN("pkg from client, BossID = :%d", rstCsPkg.m_stBody.m_stGuildBossGetSingleRewardReq.m_bBossID);
    if (poPlayer == NULL)
    {
        LOGERR("poPlayer is NULL");
        return ERR_DEFAULT;
    }

    Guild::Instance().GuildBossGetSingleReward(&poPlayer->GetPlayerData(), rstCsPkg.m_stBody.m_stGuildBossGetSingleRewardReq.m_bBossID);
    return ERR_NONE;
}

int GuildBossDamageRankInGuildReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_BOSS_DAMAGE_RANK_IN_GUILD_REQ;
    CS_PKG_GUILD_BOSS_DAMAGE_RANK_IN_GUILD_REQ& rstCsReq = rstCsPkg.m_stBody.m_stGuildBossDamageRankInGuildReq;
    SS_PKG_GUILD_BOSS_DAMAGE_RANK_IN_GUILD_REQ& rstSsPkg = m_stSsPkg.m_stBody.m_stGuildBossDamageRankInGuildReq;
    rstSsPkg.m_bBossId = rstCsReq.m_bBossId;
    rstSsPkg.m_ullGuildId = rstCsReq.m_ullGuildId;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

int GuildBossFightTimesReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
    CS_PKG_GUILD_BOSS_MEMBER_FIGHT_TIMES_REQ& rstCsReq = rstCsPkg.m_stBody.m_stGuildBossFightTimesReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_BOSS_GET_MEM_FIGHT_TIMES_REQ;
    SS_PKG_GUILD_BOSS_GET_MEM_FIGHT_TIMES_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stGuildBossGetMemFightTimesReq;
    rstSsReq.m_ullGuildId = rstCsReq.m_ullGuildId;
    if (rstCsReq.m_ullGuildId == 0)
    {
        LOGERR("GuildId = 0");
        return -1;
    }

    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (poPlayer == NULL)
    {
        LOGERR("player no exist");
        return ERR_DEFAULT;
    }
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

int GuildBossPassedGuildList_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_BOSS_PASSED_GUILD_LIST_REQ;
    SS_PKG_GUILD_BOSS_PASSED_GUILD_LIST_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stGuildBossPassedGuildListReq;
    rstSsReq.m_bBossId = rstCsPkg.m_stBody.m_stGuildBossGetPassedGuildListReq.m_bBossId;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (poPlayer == NULL)
    {
        LOGERR("poPlayer is null");
        return -1;
    }
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    uint64_t ullGuildId = poPlayer->GetPlayerData().GetGuildInfo().m_ullGuildId;
    rstSsReq.m_ullGuildId = ullGuildId;
    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

int GuildHangSettleReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (poPlayer == NULL)
	{
		LOGERR("player not exist.");
		return -1;
	}

	SS_PKG_GET_BE_SPEED_INFO_REQ& rstSsPkgReq = m_stSsPkg.m_stBody.m_stGuildGetBeSpeededInfoReq;
	rstSsPkgReq.m_ullGuildId = poPlayer->GetPlayerData().GetGuildInfo().m_ullGuildId;

	if (rstSsPkgReq.m_ullGuildId == 0)
	{
		LOGERR("GuildId is 0.");
		SC_PKG_GUILD_HANG_SETTLE_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildHangSettleRsp;
		rstScPkgRsp.m_nErrNo = ERR_SYS;
		ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
		return 0;
	}

	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GET_BE_SPEED_INFO_REQ;
	m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetPlayerData().GetRoleBaseInfo().m_ullUin;
	ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);

	return 0;
}

int GuildHangPurchaseSlotReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (poPlayer == NULL)
	{
		LOGERR("player not exist.");
		return -1;
	}

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_HANG_PURCHASE_SLOT_RSP;
	CS_PKG_GUILD_HANG_PURCHASE_SLOT_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildHangPurchaseSlotReq;
	SC_PKG_GUILD_HANG_PURCHASE_SLOT_RSP& rstScPkg = m_stScPkg.m_stBody.m_stGuildHangPurchaseSlotRsp;
	rstScPkg.m_stSyncItemInfo.m_bSyncItemCount = 0;

	if (rstCsPkgReq.m_bType == 1)
	{
		rstScPkg.m_nErrNo = GuildGeneralHang::Instance().BuyLevelSlot(&poPlayer->GetPlayerData(), rstScPkg.m_stSyncItemInfo, rstCsPkgReq.m_bIndex);
	}
	else if (rstCsPkgReq.m_bType == 2)
	{
		rstScPkg.m_nErrNo = GuildGeneralHang::Instance().BuyVipSlot(&poPlayer->GetPlayerData(), rstScPkg.m_stSyncItemInfo, rstCsPkgReq.m_bIndex);
	}
	else
	{
		rstScPkg.m_nErrNo = ERR_WRONG_PARA;
	}

	rstScPkg.m_bIndex = rstCsPkgReq.m_bIndex;
	rstScPkg.m_bType = rstCsPkgReq.m_bType;

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 0;
}

int GuildHangLayGeneralReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (poPlayer == NULL)
	{
		LOGERR("player not exist.");
		return -1;
	}

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_HANG_LAY_GENERAL_RSP;
	CS_PKG_GUILD_HANG_LAY_GENERAL_REQ& rstCspkgReq = rstCsPkg.m_stBody.m_stGuildHangLayGeneralReq;
	SC_PKG_GUILD_HANG_LAY_GENERAL_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildHangLayGeneralRsp;

	rstScPkgRsp.m_nErrNo = GuildGeneralHang::Instance().LayGCard(&poPlayer->GetPlayerData(), rstScPkgRsp, rstCspkgReq);

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 0;
}

int GuildHangSpeedPartnerReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (poPlayer == NULL)
	{
		LOGERR("player not exist.");
		return -1;
	}

	CS_PKG_GUILD_HANG_SPEED_PARTNER_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildHangSpeedPartnerReq;
	SS_PKG_SPEED_PARTNER_REQ& rstSsPkgReq = m_stSsPkg.m_stBody.m_stGuildSpeedPartnerReq;

	rstSsPkgReq.m_ullGuildId = poPlayer->GetPlayerData().GetGuildInfo().m_ullGuildId;

	if (rstSsPkgReq.m_ullGuildId == 0)
	{
		LOGERR("GuildId is 0.");
		SC_PKG_GUILD_HANG_SPEED_PARTNER_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildHangSpeedPartnerRsp;
		rstScPkgRsp.m_nErrNo = ERR_SYS;
		ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
		return 0;
	}

	rstSsPkgReq.m_ullTargetUin = rstCsPkgReq.m_ullTargetUin;
	m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetPlayerData().GetRoleBaseInfo().m_ullUin;
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_SPEED_PARTNER_REQ;
	ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);

	return 0;
}

int GuildHangStarReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (poPlayer == NULL)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_GUILD_HANG_STAR_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildHangStarReq;
    SS_PKG_GUILD_HANG_STAR_REQ& rstSsPkgReq = m_stSsPkg.m_stBody.m_stGuildHangStarReq;

    rstSsPkgReq.m_bType = rstCsPkgReq.m_bType;
    rstSsPkgReq.m_ullTargetUin = rstCsPkgReq.m_ullTargetUin;
    rstSsPkgReq.m_ullGuildId = poPlayer->GetPlayerData().GetGuildInfo().m_ullGuildId;

    if (rstSsPkgReq.m_ullGuildId == 0)
    {
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_HANG_STAR_RSP;
        m_stScPkg.m_stBody.m_stGuildHangStarRsp.m_nErrNo = ERR_GUILD_PLAYER_NOT_IN_GUILD;
        m_stScPkg.m_stBody.m_stGuildHangStarRsp.m_stHangStarInfo.m_bCount = 0;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return 0;
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_HANG_STAR_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetPlayerData().GetRoleBaseInfo().m_ullUin;

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);

    return 0;
}

int GuildBossGetSubRwdMemListReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_BOSS_GET_MEM_WHO_GET_SUB_RWD_REQ;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (poPlayer == NULL)
    {
        LOGERR("player not exist.");
        return -1;
    }
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    SS_PKG_GUILD_BOSS_GET_MEM_WHO_GET_SUB_RWD_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stGuildBossGetMemWhoGetSubRwdReq;
    rstSsReq.m_ullGuildId = rstCsPkg.m_stBody.m_stGuildBossGetSubRwdMemListReq.m_ullGuildId;

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

//SS协议
int CreateGuildRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_CREATE_GUILD_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stCreateGuildRsp;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    //将GuildSvr的结果发给Clinet
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CREATE_GUILD_RSP;
    SC_PKG_CREATE_GUILD_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stCreateGuildRsp;
    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;

    //记录GuildId
    if (rstScPkgBodyRsp.m_nErrNo == ERR_NONE)
    {
        DT_ROLE_GUILD_INFO& rstGuildInfo = poPlayer->GetPlayerData().GetGuildInfo();
        rstGuildInfo.m_ullGuildId = rstSsPkgBodyRsp.m_stGuildWholeData.m_stBaseInfo.m_ullGuildId;
        rstGuildInfo.m_bGuildLevel = 1;
        rstGuildInfo.m_bGuildIdentify = GUILD_IDENTITY_LEADER;
        Guild::Instance().AfterCreateGuild(&poPlayer->GetPlayerData(), rstScPkgBodyRsp.m_stSyncItem);
        memcpy(&rstScPkgBodyRsp.m_stGuildWholeData, &rstSsPkgBodyRsp.m_stGuildWholeData, sizeof(DT_GUILD_WHOLE_DATA));
        memcpy(&rstScPkgBodyRsp.m_stFightStateInfo, &rstSsPkgBodyRsp.m_stFightStateInfo, sizeof(DT_GUILD_FIGHT_STATE_INFO));
        memcpy(&rstScPkgBodyRsp.m_stGuildBossInfo, &rstSsPkgBodyRsp.m_stGuildBossInfo, sizeof(DT_GUILD_BOSS_MODULE_STATE));

        // 任务
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_GUILD, 1/*value*/);

        // 激活
        poPlayer->GetPlayerData().m_bIsJoinGuild = true;
        Task::Instance().TaskCondTrigger(&poPlayer->GetPlayerData(), TASK_COND_TYPE_GUILD);

        ZoneLog::Instance().WriteGuildLog(&poPlayer->GetPlayerData(), METHOD_GUILD_OP_CREATE, rstSsPkgBodyRsp.m_stGuildWholeData.m_stBaseInfo.m_ullGuildId, rstScPkgBodyRsp.m_stGuildWholeData.m_stBaseInfo.m_szName, 0, 0);
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int JoinGuildRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_JOIN_GUILD_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stJoinGuildRsp;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    //将GuildSvr的结果发给Clinet
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_JOIN_GUILD_RSP;
    SC_PKG_JOIN_GUILD_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stJoinGuildRsp;
    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;

    if (rstSsPkgBodyRsp.m_nErrNo == ERR_NONE)
    {
        ZoneLog::Instance().WriteGuildLog(&poPlayer->GetPlayerData(), METHOD_GUILD_OP_JOIN, poPlayer->GetPlayerData().GetGuildInfo().m_ullGuildId, NULL, rstSsPkgBodyRsp.m_bNeedApply/*加入军团是否需要同意*/, 0);
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int DissolveGuildRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_DISSOLVE_GUILD_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stDissolveGuildRsp;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    //将GuildSvr的结果发给Clinet
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_DISSOLVE_GUILD_RSP;
    SC_PKG_DISSOLVE_GUILD_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stDissolveGuildRsp;
    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;

    //更新退出公会时间
    if (rstSsPkgBodyRsp.m_nErrNo == ERR_NONE)
    {
        uint64_t ullTimeStamp = CGameTime::Instance().GetCurrSecond();
        DT_ROLE_GUILD_INFO& rstGuildInfo = poPlayer->GetPlayerData().GetGuildInfo();
        rstGuildInfo.m_ullGuildLastQuitTime = ullTimeStamp;

        ZoneLog::Instance().WriteGuildLog(&poPlayer->GetPlayerData(), METHOD_GUILD_OP_DISSOLVE, rstGuildInfo.m_ullGuildId, NULL, 0, 0);
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int GuildSetNeedApplyRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_GUILD_SET_NEED_APPLY_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stGuildSetNeedApplyRsp;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    //将GuildSvr的结果发给Clinet
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_SET_NEED_APPLY_RSP;
    SC_PKG_GUILD_SET_NEED_APPLY_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGuildSetNeedApplyRsp;
    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;
	rstScPkgBodyRsp.m_bNeedApply = rstSsPkgBodyRsp.m_bNeedApply;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int GuildSetLevelLimitRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
	SS_PKG_GUILD_SET_LEVEL_LIMIT_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stGuildSetLevelLimitRsp;
	Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
	if (!poPlayer)
	{
		LOGRUN("poPlayer is null");
		return -1;
	}

	//将GuildSvr的结果发给Clinet
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_SET_LEVEL_LIMIT_RSP;
	SC_PKG_GUILD_SET_LEVEL_LIMIT_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGuildSetLevelLimit;
	rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;
	rstScPkgBodyRsp.m_bLevelLimit = rstSsPkgBodyRsp.m_bLevelLimit;

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 0;
}

int QuitGuildRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_QUIT_GUILD_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stQuitGuildRsp;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    //将GuildSvr的结果发给Clinet
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_QUIT_GUILD_RSP;
    SC_PKG_QUIT_GUILD_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stQuitGuildRsp;
    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;

    //更新退出公会时间
    if (rstSsPkgBodyRsp.m_nErrNo == ERR_NONE)
    {
        uint64_t ullTimeStamp = CGameTime::Instance().GetCurrSecond();
        DT_ROLE_GUILD_INFO& rstGuildInfo = poPlayer->GetPlayerData().GetGuildInfo();
        rstGuildInfo.m_ullGuildLastQuitTime = ullTimeStamp;
        rstGuildInfo.m_ullGuildId = 0;
        rstGuildInfo.m_bGuildIdentify = 0;
        //记军团日志
        ZoneLog::Instance().WriteGuildLog(&poPlayer->GetPlayerData(), METHOD_GUILD_OP_QUIT, rstGuildInfo.m_ullGuildId, NULL, 0, 0);
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int RefreshGuildListRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_REFRESH_GUILDLIST_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stRefreshGuildListRsp;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    //将GuildSvr的结果发给Clinet
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_REFRESH_GUILDLIST_RSP;
    SC_PKG_REFRESH_GUILDLIST_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stRefreshGuildListRsp;
    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;

    //记录GuildId
    if (rstScPkgBodyRsp.m_nErrNo == ERR_NONE)
    {
        rstScPkgBodyRsp.m_bGuildCount = rstSsPkgBodyRsp.m_bGuildCount;
        memcpy(&rstScPkgBodyRsp.m_astGuildList, &rstSsPkgBodyRsp.m_astGuildList, sizeof(rstScPkgBodyRsp.m_astGuildList));
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int GetGuildDetailRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_GUILD_DETAILS_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stGuildDetailsRsp;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    //将GuildSvr的结果发给Clinet
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_DETAILS_RSP;
    SC_PKG_GUILD_DETAILS_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGuildDetailsRsp;
    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;

    memcpy(&rstScPkgBodyRsp.m_stGuildWholeInfo, &rstSsPkgBodyRsp.m_stGuildWholeInfo, sizeof(DT_GUILD_WHOLE_DATA));
    memcpy(&rstScPkgBodyRsp.m_stGuildPlayerInfo, &rstSsPkgBodyRsp.m_stGuildPlayerInfo, sizeof(DT_GUILD_PLAYER_DATA));
    memcpy(&rstScPkgBodyRsp.m_stGuildRoomInfo, &rstSsPkgBodyRsp.m_stGuildRoomInfo, sizeof(DT_GUILD_ROOM_INFO));
	memcpy(&rstScPkgBodyRsp.m_stGuildBossInfo, &rstSsPkgBodyRsp.m_stGuildBossInfo, sizeof(DT_GUILD_BOSS_MODULE_STATE));
    rstScPkgBodyRsp.m_stFightStateInfo = rstSsPkgBodyRsp.m_stFightStateInfo;

    uint64_t ullGuildId = rstScPkgBodyRsp.m_stGuildPlayerInfo.m_stBaseInfo.m_ullGuildId;
    DT_ROLE_GUILD_INFO& rstGuildInfo = poPlayer->GetPlayerData().GetGuildInfo();
    if (rstGuildInfo.m_ullGuildId !=0 && ullGuildId==0)
    {
        Guild::Instance().UpdateGuidContribution(&poPlayer->GetPlayerData());
    }

    if (ullGuildId != 0 && rstGuildInfo.m_ullGuildId == 0)
    {
        // 任务
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_GUILD, 1/*value*/);
        // 激活
        poPlayer->GetPlayerData().m_bIsJoinGuild = true;
        Task::Instance().TaskCondTrigger(&poPlayer->GetPlayerData(), TASK_COND_TYPE_GUILD);
    }

    rstGuildInfo.m_ullGuildId = ullGuildId;
    rstGuildInfo.m_bGuildLevel = rstSsPkgBodyRsp.m_bGuildLevel;
    rstGuildInfo.m_bGuildIdentify = rstSsPkgBodyRsp.m_bGuildIdentify;
    rstGuildInfo.m_bGuildGoldTime = rstSsPkgBodyRsp.m_bGuildGoldTime;
    poPlayer->GetPlayerData().m_bBossResetNum = rstSsPkgBodyRsp.m_bGuildBossResetNum;

    DT_GUILD_BOSS_INFO stGuildBossInfo;
    DT_GUILD_BOSS_BLOB& rstGuildBossBlob = rstScPkgBodyRsp.m_stGuildWholeInfo.m_stBossBlob;
    if (rstGuildBossBlob.m_iLen != 0)
    {
        uint16_t wVersion = PKGMETA::MetaLib::getVersion();
        size_t ulUseSize = 0;
        int iRet = stGuildBossInfo.unpack((char*)rstGuildBossBlob.m_szData, sizeof(rstGuildBossBlob.m_szData), &ulUseSize, wVersion);
        if (iRet != TdrError::TDR_NO_ERROR)
        {
            LOGERR("Guild(%lu) init Apply failed, unpack DT_GUILD_BOSS_BLOB failed, Ret=%d", ullGuildId, iRet);
            return false;
        }
        for (size_t i = 0; i < stGuildBossInfo.m_dwCurBossId && stGuildBossInfo.m_dwCurBossId < MAX_GUILD_BOSS_NUM; ++i)
        {
            if (GUILD_BOSS_STATE_KILLED == stGuildBossInfo.m_astBossList[i].m_bState
                &&COMMON_AWARD_STATE_NONE == rstGuildInfo.m_szGuildBossAwardStateList[i])
            {
                rstGuildInfo.m_szGuildBossAwardStateList[i] = COMMON_AWARD_STATE_AVAILABLE;
            }
            LOGRUN("Current Boss<%d> and award state is <%d>", (int)i, (int)rstGuildInfo.m_szGuildBossAwardStateList[i]);
        }
    }

    //向GuildSvr发送登录消息
    Guild::Instance().RefreshMemberInfo(&poPlayer->GetPlayerData(), 1);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int GuildDealApplyRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    //先给审批者回复
    SS_PKG_GUILD_DEAL_APPLICATION_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stGuildDealApplyRsp;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }
    //将GuildSvr的结果发给审批者
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_DEAL_APPLICATION_RSP;
    SC_PKG_GUILD_DEAL_APPLICATION_RSP& rstScPkgDealApplyRsp = m_stScPkg.m_stBody.m_stGuildDealApplicationRsp;
    rstScPkgDealApplyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int GuildBroadCastNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_GUILD_BROADCAST_NTF& rstSsPkgBodyNtf = rstSsPkg.m_stBody.m_stGuildBroadcastNtf;

    //初始化ScPkg
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_MSG_NTF;
    SC_PKG_GUILD_MSG_NTF& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGuildMsgNtf;
    rstScPkgBodyRsp.m_wMsgId = rstSsPkgBodyNtf.m_wMsgId;
    memcpy(&rstScPkgBodyRsp.m_stNtfMsg, &rstSsPkgBodyNtf.m_stNtfMsg, sizeof(DT_GUILD_NTF_MSG));

    for (int i=0; i<rstSsPkgBodyNtf.m_bPlayerCount; i++)
    {
        uint64_t ullPlayerId = rstSsPkgBodyNtf.m_PlayerList[i];
        Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullPlayerId);
        if (!poPlayer)
        {
            //LOGRUN("poPlayer is null");
            continue;
        }

        DT_ROLE_GUILD_INFO& rstGuildInfo = poPlayer->GetPlayerData().GetGuildInfo();

        switch (rstScPkgBodyRsp.m_wMsgId)
        {
        case DT_GUILD_MSG_UPDATE_GUILD_GLOBAL:
            rstGuildInfo.m_bGuildLevel = rstScPkgBodyRsp.m_stNtfMsg.m_stUptGuildGlobal.m_stGlobalInfo.m_bGuildLevel;
            break;
        case DT_GUILD_MSG_UPDATE_GUILD_MEMBER:
            {
                if (ullPlayerId == rstScPkgBodyRsp.m_stNtfMsg.m_stUptGuildMember.m_stMemberInfo.m_ullUin)
                {
                    rstGuildInfo.m_bGuildIdentify = rstScPkgBodyRsp.m_stNtfMsg.m_stUptGuildMember.m_stMemberInfo.m_bIdentity;
                }
				if (ullPlayerId == rstScPkgBodyRsp.m_stNtfMsg.m_stUptGuildMember.m_stOfficerInfo.m_ullUin)
				{
					rstGuildInfo.m_bGuildIdentify = rstScPkgBodyRsp.m_stNtfMsg.m_stUptGuildMember.m_stOfficerInfo.m_bIdentity;
				}
            }
            break;
        case DT_GUILD_MSG_DISSOLVE_GUILD:
            rstGuildInfo.m_ullGuildId = 0;
            rstGuildInfo.m_bGuildIdentify = 0;
            break;
        case DT_GUILD_MSG_UPDATE_GUILD_BOSS:
            Guild::Instance().GuildBossSettleUpdate(&poPlayer->GetPlayerData(), rstScPkgBodyRsp.m_stNtfMsg.m_stUptGuildBoss);
            continue;   //注意:这里直接continue,在逻辑里回包
		case DT_GUILD_MSG_UPDATE_GUILD_SOCIETY:
			Guild::Instance().UpdateSocietyBuffs(rstGuildInfo, rstScPkgBodyRsp.m_stNtfMsg.m_stUptGuildSociety);
			break;
        default :
            break;
        }

        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }
    return 0;
}

int UpdateGulidNotice_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_UPDATE_GUILD_NOTICE_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stUpdateGuildNoticeRsp;
    LOGRUN("recevie GuildSvr update notice!");
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    //将GuildSvr的结果发给Clinet
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_UPDATE_GUILD_NOTICE_RSP;
    SC_PKG_UPDATE_GUILD_NOTICE_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stUpdateGuildNoticeRsp;
    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

//设置公会成员职位 收到回复
int SetGuildMemJob_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_SET_GUILD_MEM_JOB_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stSetGuildMemJobRsp;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    //将GuildSvr的结果发给Clinet
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_SET_GUILD_MEM_JOB_RSP;
    SC_PKG_SET_GUILD_MEM_JOB_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stSetGuildMemJobRsp;
    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

//踢人收到GuildSvr回复
int GuildKickPlayerRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }
    //将GuildSvr的结果发给Clinet
    SS_PKG_GUILD_KICK_PLAYER_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stGuildKickPlayerRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_KICK_PLAYER_RSP;
    SC_PKG_GUILD_KICK_PLAYER_RSP& rstScKickPlayerRsp = m_stScPkg.m_stBody.m_stGuildKickPlayerRsp;
    rstScKickPlayerRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    //如果被踢的人在线
    poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkgBodyRsp.m_ullPlayerId);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }
    DT_ROLE_GUILD_INFO &rstGuildInfo = poPlayer->GetPlayerData().GetGuildInfo();
    rstGuildInfo.m_ullGuildId = 0;
    rstGuildInfo.m_bGuildIdentify = 0;
    return 0;
}

//搜索公会 收到回复
int SearchGuildRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_SEARCH_GUILD_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stSearchGuildRsp;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    //将GuildSvr的结果发给Clinet
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_SEARCH_GUILD_RSP;
    SC_PKG_SEARCH_GUILD_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stSearchGuildRsp;

    rstScPkgBodyRsp.m_bIsExist = rstSsPkgBodyRsp.m_bIsExist;
    rstScPkgBodyRsp.m_stGuildBriefInfo = rstSsPkgBodyRsp.m_stGuildBriefInfo;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

//升级公会回复
int GuildLvUpRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_LEVELUP_RSP;

    SS_PKG_GUILD_LEVELUP_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stGuildLvUpRsp;
    SC_PKG_GUILD_LEVELUP_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGuildLevelupRsp;

    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;

    if (rstSsPkgBodyRsp.m_nErrNo == ERR_NONE)
    {
        //记公会日志
        ZoneLog::Instance().WriteGuildLog(&poPlayer->GetPlayerData(), METHOD_GUILD_OP_LVUP, poPlayer->GetPlayerData().GetGuildInfo().m_ullGuildId, NULL, 0, poPlayer->GetPlayerData().GetGuildInfo().m_bGuildLevel);
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int GuildDrawSalaryRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
	if (!poPlayer)
	{
		LOGRUN("poPlayer is null");
		return -1;
	}

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_DRAW_SALARY_RSP;
	SC_PKG_GUILD_DRAW_SALARY_RSP& rstScPkg = m_stScPkg.m_stBody.m_stGuildDrawSalaryRsp;
	SS_PKG_GUILD_DRAW_SALARY_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stGuildDrawSalaryRsp;

	rstScPkg.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;

	if (rstSsPkgBodyRsp.m_nErrNo == ERR_NONE)
	{
		rstScPkg.m_stSyncItemInfo.m_bSyncItemCount = 0;
		rstScPkg.m_nErrNo = Guild::Instance().DrawSalary(&poPlayer->GetPlayerData(), rstScPkg.m_stSyncItemInfo, rstSsPkgBodyRsp.m_bSalaryIdentityToday);
	}

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 0;
}

//获取军团排行榜回复
int GuildGetGuildRankRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }
    SS_PKG_GUILD_GET_GUILD_RANK_RSP& rstSsPkgBody = rstSsPkg.m_stBody.m_stGuildGetGuildRankRsp;
    SC_PKG_GUILD_GET_GUILD_RANK_RSP& rstScPkgBody = m_stScPkg.m_stBody.m_stGuildGetGuildRankRsp;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_GET_GUILD_RANK_RSP;
	rstScPkgBody.m_ullVersion = rstScPkgBody.m_ullVersion;
	rstScPkgBody.m_dwSelfRank = rstScPkgBody.m_dwSelfRank;
    memcpy(rstScPkgBody.m_astRankList, rstSsPkgBody.m_astRankList, sizeof(rstScPkgBody.m_astRankList));
    rstScPkgBody.m_nCount = rstSsPkgBody.m_nCount;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

//获取军团积分排行榜回复
int GuildGetGFightRankRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }
    SS_PKG_GUILD_GET_GFIGHT_RANK_RSP& rstSsPkgBody = rstSsPkg.m_stBody.m_stGuildGetGFightRankRsp;
    SC_PKG_GUILD_GET_GFIGHT_RANK_RSP& rstScPkgBody = m_stScPkg.m_stBody.m_stGuildGetGFightRankRsp;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_GET_GFIGHT_RANK_RSP;
    memcpy(rstScPkgBody.m_astRankList, rstSsPkgBody.m_astRankList, sizeof(rstScPkgBody.m_astRankList));
    rstScPkgBody.m_nCount = rstSsPkgBody.m_nCount;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

//报名回复
int GuildFightApplyRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_FIGHT_APPLY_RSP;

    SS_PKG_GUILD_FIGHT_APPLY_RSP& rstSsPkgRsp = rstSsPkg.m_stBody.m_stGuildFightApplyRsp;
    SC_PKG_GUILD_FIGHT_APPLY_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildFightApplyRsp;

    rstScPkgRsp.m_nErrNo = rstSsPkgRsp.m_nErrNo;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

//报名列表同步
int GuildFightApplyListNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_GUILD_FIGHT_APPLY_LIST_NTF& rstSsPkgNtf = rstSsPkg.m_stBody.m_stGuildFightApplyListNtf;
    Guild::Instance().SetGuildFightApplyList(rstSsPkgNtf.m_stApplyListInfo);
    return 0;
}

//对阵表同步
int GuildFightAgainstNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_GUILD_FIGHT_AGAINST_INFO_NTF& rstSsPkgNtf = rstSsPkg.m_stBody.m_stGuildFightAgainstInfoNtf;
    Guild::Instance().SetGuildFightAgainstInfo(rstSsPkgNtf.m_stAgainstInfo);
    return 0;
}

//加入
int GuildFightArenaJoinRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    SS_PKG_GUILD_FIGHT_ARENA_JOIN_RSP& rstSsPkgRsp = rstSsPkg.m_stBody.m_stGuildFightArenaJoinRsp;
    SC_PKG_GUILD_FIGHT_ARENA_JOIN_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildFightArenaJoinRsp;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_FIGHT_ARENA_JOIN_RSP;
    rstScPkgRsp.m_nErrNo = rstSsPkgRsp.m_nErrNo;
    rstScPkgRsp.m_ullTimeStamp = rstSsPkgRsp.m_ullTimeStamp;
    memcpy(&rstScPkgRsp.m_stArenaInfo, &rstSsPkgRsp.m_stArenaInfo, sizeof(rstScPkgRsp.m_stArenaInfo));

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

//退出
int GuildFightArenaQuitRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    SS_PKG_GUILD_FIGHT_ARENA_QUIT_RSP& rstSsPkgRsp = rstSsPkg.m_stBody.m_stGuildFightArenaQuitRsp;
    SC_PKG_GUILD_FIGHT_ARENA_QUIT_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildFightArenaQuitRsp;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_FIGHT_ARENA_QUIT_RSP;
    rstScPkgRsp.m_nErrNo = rstSsPkgRsp.m_nErrNo;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

//移动
int GuildFightArenaMoveRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    SS_PKG_GUILD_FIGHT_ARENA_MOVE_RSP& rstSsPkgRsp = rstSsPkg.m_stBody.m_stGuildFightArenaMoveRsp;
    SC_PKG_GUILD_FIGHT_ARENA_MOVE_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildFightArenaMoveRsp;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_FIGHT_ARENA_MOVE_RSP;
    rstScPkgRsp.m_nErrNo = rstSsPkgRsp.m_nErrNo;
    rstScPkgRsp.m_wTime = rstSsPkgRsp.m_wTime;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

//打完匹配后退出
int GuildFightQuitMatchRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    SS_PKG_GUILD_FIGHT_QUIT_MATCH_RSP& rstSsPkgRsp = rstSsPkg.m_stBody.m_stGuildFightQuitMatchRsp;
    SC_PKG_GUILD_FIGHT_QUIT_MATCH_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildFightQuitMatchRsp;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_FIGHT_QUIT_MATCH_RSP;
    rstScPkgRsp.m_nErrNo = rstSsPkgRsp.m_nErrNo;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int GuildFightCancleGodRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    SS_PKG_GUILD_FIGHT_QUIT_MATCH_RSP& rstSsPkgRsp = rstSsPkg.m_stBody.m_stGuildFightQuitMatchRsp;
    SC_PKG_GUILD_FIGHT_QUIT_MATCH_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildFightQuitMatchRsp;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_FIGHT_CANCLE_GOD_RSP;
    rstScPkgRsp.m_nErrNo = rstSsPkgRsp.m_nErrNo;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

// 军团战PVP匹配成功
int GuildFightPvpMatchReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    SS_PKG_GUILD_FIGHT_PVP_MATCH_REQ& rstSsPkgBody = rstSsPkg.m_stBody.m_stGuildFightPvpMatchReq;

    Player* poPlayer1 = PlayerMgr::Instance().GetPlayerByUin(rstSsPkgBody.m_stPlayerInfo1.m_ullUin);

    Player* poPlayer2 = PlayerMgr::Instance().GetPlayerByUin(rstSsPkgBody.m_stPlayerInfo2.m_ullUin);

    int16_t nErrNo = ERR_NONE;
    do
    {
        if (poPlayer1 == NULL || poPlayer2 == NULL)
        {
            nErrNo = ERR_NOT_SATISFY_COND;
            break;
        }

        // 初始化匹配玩家数据
        nErrNo = Match::Instance().InitFightPlayerInfo(&poPlayer1->GetPlayerData(), MATCH_TYPE_GUILD_FIGHT);
        if (nErrNo != ERR_NONE) break;

        nErrNo = Match::Instance().InitFightPlayerInfo(&poPlayer2->GetPlayerData(), MATCH_TYPE_GUILD_FIGHT);
        if (nErrNo != ERR_NONE) break;

        // 创建副本
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_DUNGEON_CREATE_REQ;
        m_stSsPkg.m_stHead.m_ullReservId = 0;	// 目标 FightSvrId 由 ClusterGate 决定，暂时不关心该字段
        SS_PKG_DUNGEON_CREATE_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stDungeonCreateReq;

        bzero(&rstSsPkgBodyReq.m_stDungeonInfo, sizeof(rstSsPkgBodyReq.m_stDungeonInfo));
		rstSsPkgBodyReq.m_stDungeonInfo.m_bMatchType = MATCH_TYPE_GUILD_FIGHT;
        rstSsPkgBodyReq.m_stDungeonInfo.m_bFakeType = MATCH_FAKE_NONE;

        rstSsPkgBodyReq.m_stDungeonInfo.m_bFightPlayerNum = 2;
        rstSsPkgBodyReq.m_stDungeonInfo.m_astFightPlayerList[0] = poPlayer1->GetPlayerData().m_oSelfInfo;
        rstSsPkgBodyReq.m_stDungeonInfo.m_astFightPlayerList[0].m_chGroup = PLAYER_GROUP_DOWN;

        rstSsPkgBodyReq.m_stDungeonInfo.m_astFightPlayerList[1] = poPlayer2->GetPlayerData().m_oSelfInfo;
        rstSsPkgBodyReq.m_stDungeonInfo.m_astFightPlayerList[1].m_chGroup = PLAYER_GROUP_UP;

        ZoneSvrMsgLayer::Instance().SendToFightSvr(m_stSsPkg);

    } while (false);

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_PVP_MATCH_RSP;
    m_stSsPkg.m_stHead.m_ullUin = 0;

    m_stSsPkg.m_stBody.m_stGuildFightPvpMatchRsp.m_nErrNo = nErrNo;
    m_stSsPkg.m_stBody.m_stGuildFightPvpMatchRsp.m_stPlayerInfo1 = rstSsPkgBody.m_stPlayerInfo1;

    m_stSsPkg.m_stBody.m_stGuildFightPvpMatchRsp.m_stPlayerInfo2 = rstSsPkgBody.m_stPlayerInfo2;
    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return 0;
}

//将GuildSvr的广播消息发给客户端
int GuildFightMsgBroad_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    SS_PKG_GUILD_FIGHT_MSG_BROADCAST& rstSsPkgBody = rstSsPkg.m_stBody.m_stGuildFightMsgBroadCast;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_FIGHT_MSG_SYN;
    SC_PKG_GUILD_FIGHT_MSG_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stGuildFightMsgSyn;
    memcpy(&rstScPkgBody.m_stSynMsg, &rstSsPkgBody.m_stSynMsg, sizeof(DT_GUILD_FIGHT_SYN_MSG));

    for (int i=0; i<rstSsPkgBody.m_bReceiverCnt; i++)
    {
       uint64_t ullPlayerId = rstSsPkgBody.m_ReceiverList[i];
       Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullPlayerId);
       if (!poPlayer)
       {
           LOGRUN("poPlayer is null");
           continue;
       }

       ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }
    return 0;
}

int GuildFightSettleNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    SS_PKG_GUILD_FIGHT_SETTLE_NTF& rstSsPkgBody = rstSsPkg.m_stBody.m_stGuildFightSettleNtf;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_FIGHT_SETTLE_NTF;
    SC_PKG_GUILD_FIGHT_SETTLE_NTF& rstScPkgBody = m_stScPkg.m_stBody.m_stGuildFightSettleNtf;
    memcpy(&rstScPkgBody.m_stSettleInfo, &rstSsPkgBody.m_stSettleInfo, sizeof(DT_GUILD_FIGHT_SETTLE_INFO));

    for (int i=0; i<rstSsPkgBody.m_bReceiverCnt; i++)
    {
       uint64_t ullPlayerId = rstSsPkgBody.m_ReceiverList[i];
       Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullPlayerId);
       if (!poPlayer)
       {
           LOGRUN("poPlayer is null");
           continue;
       }
       ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
       //主公称号获得
       GloryItemsMgr::Instance().AddMajestyItems(&poPlayer->GetPlayerData(), MAJESTY_ITEM_ACCESS_WIN_BATTLE, rstSsPkgBody.m_stSettleInfo.m_astStatisInfo[i].m_wKillNum);
    }
    return 0;
}

int GuildUploadReplayRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_FIGHT_QUIT_MATCH_RSP;
    SC_PKG_UPLOAD_GUILD_REPLAY_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildUploadReplayRsp;
    SS_PKG_UPLOAD_GUILD_REPLAY_RSP& rstSsPkgRsp = rstSsPkg.m_stBody.m_stGuildUploadReplayRsp;

    rstScPkgRsp.m_nErrNo = rstSsPkgRsp.m_nErrNo;
    memcpy(rstScPkgRsp.m_szURL, rstSsPkgRsp.m_szURL, MAX_LEN_URL);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int GuildBossEnterFightRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    int iRet = ERR_NONE;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (poPlayer == NULL)
    {
        LOGERR("Uin<%lu> poPlayer is null" , rstSsPkg.m_stHead.m_ullUin);
        return ERR_DEFAULT;
    }

    uint64_t ullStartTime = CGameTime::Instance().GetSecOfHourInCurrDay(9) + 30 * 60;
    uint64_t ullEndTime = CGameTime::Instance().GetSecOfHourInCurrDay(23) + 30 * 60;
    uint8_t bCurrWeekDay = CGameTime::Instance().GetCurDayOfWeek();

    SS_PKG_GUILD_BOSS_ENTER_FIGHT_RSP& rstSSPkgRsp = rstSsPkg.m_stBody.m_stGuildBossEnterFightRsp;
    SC_PKG_FIGHT_PVE_ENTER_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stFightPveEnterRsp;
    bzero( &rstScPkgBodyRsp, sizeof(rstScPkgBodyRsp) );

    do
    {
        if (bCurrWeekDay == 7 ||
            CGameTime::Instance().IsContainCur(ullStartTime, ullEndTime) == false )
        {
            iRet = ERR_WRONG_STATE;
            break;
        }

        if( ERR_NONE == rstSSPkgRsp.m_nErrNo )
        {
            iRet = Guild::Instance().CheckFightBoss(&poPlayer->GetPlayerData(), rstSSPkgRsp.m_dwFLevelId);
        }else
        {
            iRet = rstSSPkgRsp.m_nErrNo;
        }
    }while(0);

    rstScPkgBodyRsp.m_nErrNo = iRet;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_PVE_ENTER_RSP;

    if( ERR_NONE == rstScPkgBodyRsp.m_nErrNo )
    {
        rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
        rstScPkgBodyRsp.m_dwLevelId = rstSSPkgRsp.m_dwFLevelId;
        rstScPkgBodyRsp.m_dwBossHpLeft = rstSSPkgRsp.m_dwBossHpLeft;
        LOGRUN("LevelId<%u>, Boss left HP<%u>", rstScPkgBodyRsp.m_dwLevelId, rstScPkgBodyRsp.m_dwBossHpLeft);
        rstScPkgBodyRsp.m_ullTimeStamp = CGameTime::Instance().GetCurrSecond();
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int GuildBossCompetitorInfoRsp_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_BOSS_COMPETITOR_INFO_RSP;
    SC_PKG_GUILD_BOSS_COMPETITOR_INFO_RSP& rstScRsp = m_stScPkg.m_stBody.m_stGuildBossCompetitorInfoRsp;
    SS_PKG_GUILD_BOSS_COMPETITOR_INFO_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stGuildBossCompetitorInfoRsp;
    rstScRsp.m_bIsCompetitorExisted = rstSsRsp.m_bIsCompetitorExisted;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (rstSsRsp.m_bIsCompetitorExisted == 0)
    {
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return 0;
    }

    rstScRsp.m_stCompetitorInfo = rstSsRsp.m_stCompetitorGuildInfo;
    rstScRsp.m_nErrNo = rstSsRsp.m_nErrNo;
    if (poPlayer == NULL)
    {
        LOGERR("poPlayer is null!");
        return ERR_NOT_FOUND;
    }
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int GuildBossDamageRankInGuildRsp_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_BOSS_DAMAGE_RANK_IN_GUILD_RSP;
    SC_PKG_GUILD_BOSS_DAMAGE_RANK_IN_GUILD_RSP& rstScRsp = m_stScPkg.m_stBody.m_stGuildBossDamageRankInGuildRsp;
    SS_PKG_GUILD_BOSS_DAMAGE_RANK_IN_GUILD_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stGuildBossDamageRankInGuildRsp;
    memcpy(rstScRsp.m_astGuildMemberDamageRank, rstSsRsp.m_astGuildMemberDamageRank,
        sizeof(DT_GUILD_BOSS_DAMAGE_NODE) * rstSsRsp.m_wCount);
    rstScRsp.m_wAttackedMemNum = rstSsRsp.m_wCount;

    rstScRsp.m_nErrNo = ERR_NONE;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (poPlayer == NULL)
    {
        LOGERR("poPlayer<%lu> is NULL", rstSsPkg.m_stHead.m_ullUin);
        return ERR_SYS;
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int GuildGetMemberListRsp_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_BOSS_MEMBER_FIGHT_TIMES_RSP;
    SS_PKG_GUILD_GET_MEMBER_LIST_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stGuildGetGuildMemRsp;
    SC_PKG_GUILD_BOSS_MEMBER_FIGHT_TIMES_RSP& rstScRsp = m_stScPkg.m_stBody.m_stGuildBossFightTimesRsp;

    rstScRsp.m_wCount = rstSsRsp.m_wCount;
    for (int i = 0; i < rstScRsp.m_wCount; ++i)
    {
        Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsRsp.m_UinList[i]);
        if (poPlayer == NULL)
        {
            LOGERR("player is null");
            continue;
        }
        rstScRsp.m_astMemberFightTimes[i].m_bFightTimes = poPlayer->GetPlayerData().GetGuildInfo().m_bGuildBossFightNum;
        rstScRsp.m_astMemberFightTimes[i].m_ullUin = rstSsRsp.m_UinList[i];
        LOGRUN("Player <%lu> fight %d times", rstScRsp.m_astMemberFightTimes[i].m_ullUin, rstScRsp.m_astMemberFightTimes[i].m_bFightTimes);
    }
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (poPlayer == NULL)
    {
        LOGERR("player is null");
        return ERR_DEFAULT;
    }
    rstScRsp.m_nErrNo = ERR_NONE;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int GuildBossGetPassedListRsp_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_BOSS_PASSED_GUILD_LIST_RSP;
    SS_PKG_GUILD_BOSS_PASSED_GUILD_LIST_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stGuildBossPassedGuildListRsp;
    SC_PKG_GUILD_BOSS_PASSED_GUILD_LIST_RSP& rstScRsp = m_stScPkg.m_stBody.m_stGuildBossGetPassedGuildListRsp;
    rstScRsp.m_stPassedGuildList = rstSsRsp.m_stPassedGuildList;
    int iRet = rstSsRsp.m_nErrNo;
    rstScRsp.m_bBeInFlag = rstSsRsp.m_bBeInFlag;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (poPlayer == NULL)
    {
        LOGERR("player is null");
        return -1;
    }
    rstScRsp.m_nErrNo = iRet;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int GuildSpeedPartnerRsp_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
	if (!poPlayer)
	{
		LOGRUN("poPlayer is null");
		return -1;
	}

	SS_PKG_SPEED_PARTNER_RSP& rstSsPkgRsp = rstSsPkg.m_stBody.m_stGuildSpeedPartnerRsp;
	SC_PKG_GUILD_HANG_SPEED_PARTNER_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildHangSpeedPartnerRsp;
	rstScPkgRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

	int iRet = ERR_NONE;
	do
	{
		if (rstSsPkgRsp.m_nErrNo != ERR_NONE)
		{
			iRet = rstSsPkgRsp.m_nErrNo;
			break;
		}

		DT_ROLE_MAJESTY_INFO& rstMajestyInfo = poPlayer->GetPlayerData().GetMajestyInfo();
		RESGUILDGENERALHANG* pResHang = CGameDataMgr::Instance().GetResGuildGeneralHangMgr().Find(rstMajestyInfo.m_wLevel);
		if (!pResHang)
		{
			LOGERR("pResHang is null. index<%d>", rstMajestyInfo.m_wLevel);
			iRet = ERR_SYS;
			break;
		}

        RESBASIC* poBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(1721);
        int iVitality = (int)poBasic->m_para[1];
        Item::Instance().RewardItem(&poPlayer->GetPlayerData(), ITEM_TYPE_GUILD_VITALITY, 0, iVitality, rstScPkgRsp.m_stSyncItemInfo, DWLOG::METHOD_HANG_SPEED_PARTNER);

		for (int i = 0; i < pResHang->m_bRewardCount; i++)
		{
			Item::Instance().RewardItem(&poPlayer->GetPlayerData(), pResHang->m_szItemType[i], pResHang->m_itemId[i], pResHang->m_itemNumber[i], rstScPkgRsp.m_stSyncItemInfo, DWLOG::METHOD_HANG_SPEED_PARTNER);
		}
		//帮助团员练兵 次数
		Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_GUILD_OTHER, 1, 6);
	} while (false);

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_HANG_SPEED_PARTNER_RSP;
    rstScPkgRsp.m_nErrNo = iRet;

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 0;
}

int GuildGetBeSpeededInfoRsp_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
	if (poPlayer == NULL)
	{
		LOGERR("player not exist.");
		return -1;
	}

	int iRet = ERR_NONE;
	SC_PKG_GUILD_HANG_SETTLE_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildHangSettleRsp;

	do
	{
		SS_PKG_GET_BE_SPEED_INFO_RSP& rstSsPkgRsp = rstSsPkg.m_stBody.m_stGuildGetBeSpeededInfoRsp;

		if (rstSsPkgRsp.m_nErrNo != ERR_NONE)
		{
			iRet = rstSsPkgRsp.m_nErrNo;
			break;
		}

		GuildGeneralHang::Instance().AddSpeedExp(&poPlayer->GetPlayerData(), rstSsPkgRsp.m_bBeSpeededCount);

		iRet = GuildGeneralHang::Instance().SettleExpAllSlot(&poPlayer->GetPlayerData(), rstScPkgRsp);

	} while (false);

	rstScPkgRsp.m_nErrNo = iRet;
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_HANG_SETTLE_RSP;
	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 0;
}

int GuildHangStarRsp_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (poPlayer == NULL)
    {
        LOGERR("player not exist.");
        return -1;
    }

    SS_PKG_GUILD_HANG_STAR_RSP& rstSsPkgRsp = rstSsPkg.m_stBody.m_stGuildHangStarRsp;
    SC_PKG_GUILD_HANG_STAR_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stGuildHangStarRsp;

    do
    {
        rstScPkgRsp.m_nErrNo = rstSsPkgRsp.m_nErrNo;
        if (rstScPkgRsp.m_nErrNo != ERR_NONE)
        {
            rstScPkgRsp.m_stHangStarInfo.m_bCount = 0;
            break;
        }

        rstScPkgRsp.m_bType = rstSsPkgRsp.m_bType;
        memcpy(&rstScPkgRsp.m_stHangStarInfo, &rstSsPkgRsp.m_stHangStarInfo, sizeof(DT_GUILD_HANG_STAR_INFO));
    } while (false);

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_HANG_STAR_RSP;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}


int GuildBossGetMemFightTimesRsp_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    SC_PKG_GUILD_BOSS_MEMBER_FIGHT_TIMES_RSP& rstScRsp = m_stScPkg.m_stBody.m_stGuildBossFightTimesRsp;
    SS_PKG_GUILD_BOSS_GET_MEM_FIGHT_TIMES_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stGuildBossGetMemFightTimesRsp;
    rstScRsp.m_wCount = rstSsRsp.m_wMemNum;
    LOGRUN("There are <%d> players in this Guild.", rstSsRsp.m_wMemNum);
    memcpy(rstScRsp.m_astMemberFightTimes, rstSsRsp.m_astMemFightBossTimes, sizeof(DT_GUILD_BOSS_FIGHT_TIMES) * rstScRsp.m_wCount);
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_BOSS_MEMBER_FIGHT_TIMES_RSP;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (poPlayer == NULL)
    {
        LOGERR("poPlayer<%lu> is NULL", rstSsPkg.m_stHead.m_ullUin);
        return -1;
    }

    rstScRsp.m_nErrNo = ERR_NONE;
    LOGRUN("Player<%lu> fight <%d> times.", rstSsRsp.m_astMemFightBossTimes[0].m_ullUin, rstSsRsp.m_astMemFightBossTimes[0].m_bFightTimes);
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int GuildBossGetMemWhoGetSubRwdRsp_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_BOSS_GET_SUB_RWD_MEM_LIST_RSP;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (poPlayer == NULL)
    {
        LOGERR("poPlayer<%lu> is NULL", rstSsPkg.m_stHead.m_ullUin);
        return -1;
    }
    SC_PKG_GUILD_BOSS_GET_SUB_RWD_MEM_LIST_RSP& rstScRsp = m_stScPkg.m_stBody.m_stGuildBossGetSubRwdMemListRsp;
    SS_PKG_GUILD_BOSS_GET_MEM_WHO_GET_SUB_RWD_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stGuildBossGetMemWhoGetSubRwdRsp;
    memcpy(rstScRsp.m_astMemGetSubRwdList, rstSsRsp.m_astMemGetSubRwdList,
        sizeof(DT_GUILD_BOSS_GET_SUB_RWD_NODE) * rstSsRsp.m_wMemGetSubRwdNum);
    rstScRsp.m_wMemGetSubRwdNum = rstSsRsp.m_wMemGetSubRwdNum;
    rstScRsp.m_nErrNo = ERR_NONE;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}
