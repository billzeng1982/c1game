#include <string.h>
#include "strutil.h"
#include "ObjectUpdatorMgr.h"
#include "LogMacros.h"
#include "MsgLogicMatch.h"
#include "../gamedata/GameDataMgr.h"
#include "../framework/MatchSvrMsgLayer.h"
#include "../module/MatchMgr.h"


using namespace PKGMETA;

int MatchStartReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_MATCH_START_REQ& rstSsPkgBodyReq = rstSsPkg.m_stBody.m_stMatchStartReq;

    switch(rstSsPkgBodyReq.m_bMatchType)
    {
    case MATCH_TYPE_6V6:
        Match6v6Mgr::Instance().MatchStartHandle(rstSsPkgBodyReq.m_stInfo);
        break;
    case MATCH_TYPE_WEEKEND_LEAGUE:
        MatchWeekMgr::Instance().MatchStartHandle(rstSsPkgBodyReq.m_stInfo);
        break;
    case MATCH_TYPE_LEISURE:
        MatchLeisureMgr::Instance().MatchStartHandle(rstSsPkgBodyReq.m_stInfo);
        break;
    case MATCH_TYPE_DAILY_CHALLENGE:
        MatchDailyChallengeMgr::Instance().MatchStartHandle(rstSsPkgBodyReq.m_stInfo);
        break;
    case MATCH_TYPE_PEAK_ARENA:
        MatchPeakArenaMgr::Instance().MatchStartHandle(rstSsPkgBodyReq.m_stInfo);
        break;
    default:
        LOGERR("Uin<%lu> bMatchType<%hhu> not found", rstSsPkgBodyReq.m_stInfo.m_ullUin, rstSsPkgBodyReq.m_bMatchType);
        return ERR_WRONG_PARA;
    }
    LOGRUN("MatchStart_SS Uin<%lu> MatchType<%hhu> ", rstSsPkgBodyReq.m_stInfo.m_ullUin,
        rstSsPkgBodyReq.m_bMatchType);
    return 0;
}


int MatchCancelReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_MATCH_CANCEL_REQ& rstSsPkgBodyReq = rstSsPkg.m_stBody.m_stMatchCancelReq;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MATCH_CANCEL_RSP;
    SS_PKG_MATCH_CANCEL_RSP& rstSsPkgBodyRsp = m_stSsPkg.m_stBody.m_stMatchCancelRsp;

    int iRet = ERR_NONE;

    switch(rstSsPkgBodyReq.m_bMatchType)
    {
    case MATCH_TYPE_6V6:
        iRet = Match6v6Mgr::Instance().MatchCancelHandle(rstSsPkgBodyReq.m_ullUin, rstSsPkgBodyReq.m_dwScore);
        break;
    case MATCH_TYPE_WEEKEND_LEAGUE:
        iRet = MatchWeekMgr::Instance().MatchCancelHandle(rstSsPkgBodyReq.m_ullUin, rstSsPkgBodyReq.m_dwScore);
        break;
    case MATCH_TYPE_LEISURE:
        iRet = MatchLeisureMgr::Instance().MatchCancelHandle(rstSsPkgBodyReq.m_ullUin, rstSsPkgBodyReq.m_dwScore);
        break;
    case MATCH_TYPE_DAILY_CHALLENGE:
        iRet = MatchDailyChallengeMgr::Instance().MatchCancelHandle(rstSsPkgBodyReq.m_ullUin, rstSsPkgBodyReq.m_dwScore);
        break;
    case MATCH_TYPE_PEAK_ARENA:
        iRet = MatchPeakArenaMgr::Instance().MatchCancelHandle(rstSsPkgBodyReq.m_ullUin, rstSsPkgBodyReq.m_dwScore);
        break;
    default:
        LOGERR("Uin<%lu> bMatchType<%hhu> not found", rstSsPkgBodyReq.m_ullUin, rstSsPkgBodyReq.m_bMatchType);
        return ERR_WRONG_PARA;
    }
    m_stSsPkg.m_stHead.m_ullReservId = rstSsPkg.m_stHead.m_iSrcProcId;
    rstSsPkgBodyRsp.m_nErrNo = (int16_t)iRet;
    rstSsPkgBodyRsp.m_ullUin = rstSsPkgBodyReq.m_ullUin;
    rstSsPkgBodyRsp.m_bMatchType = rstSsPkgBodyReq.m_bMatchType;
    MatchSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    LOGRUN("MatchCancelReq_SS Uin<%lu> MatchType<%hhu> ret<%d>", rstSsPkg.m_stHead.m_ullUin, rstSsPkgBodyReq.m_bMatchType, iRet);
    return 0;
}


int MatchFightSvrNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    LOGWARN("MatchFightSvrNtf_SS ProcId<%d> ", rstSsPkg.m_stBody.m_stMatchFightServNtf.m_stServInfo.m_iProcId);
#if 0
    SS_PKG_MACTH_FIGHT_SERV_NTF& rstSsPkgBodyNtf = rstSsPkg.m_stBody.m_stMatchFightServNtf;
    if (rstSsPkgBodyNtf.m_stServInfo.m_iProcId != 0)
    {
        MatchWeekMgr::Instance().SetCurFightSvrInfo(rstSsPkgBodyNtf.m_stServInfo);
        Match6v6Mgr::Instance().SetCurFightSvrInfo(rstSsPkgBodyNtf.m_stServInfo);
        MatchLeisureMgr::Instance().SetCurFightSvrInfo(rstSsPkgBodyNtf.m_stServInfo);
        MatchDailyChallengeMgr::Instance().SetCurFightSvrInfo(rstSsPkgBodyNtf.m_stServInfo);
    }
#endif
    return 0;
}

