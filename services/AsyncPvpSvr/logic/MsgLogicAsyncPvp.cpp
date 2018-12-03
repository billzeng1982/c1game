#include "strutil.h"
#include "LogMacros.h"
#include "MsgLogicAsyncPvp.h"
#include "../framework/AsyncPvpSvrMsgLayer.h"
#include "../transaction/AsyncPvpTransaction.h"
#include "../transaction/AsyncPvpTransFrame.h"
#include "../transaction/GameObjectPool.h"
#include "../module/player/AsyncPvpPlayerMgr.h"
#include "../module/team/AsyncPvpTeamMgr.h"
#include "../module/fight/AsyncPvpFightMgr.h"
#include "../module/rank/AsyncPvpRank.h"

using namespace PKGMETA;

int AsyncPvpStartReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
	SS_PKG_ASYNC_PVP_START_REQ& rstReq = rstSsPkg.m_stBody.m_stAsyncpvpStartReq;

    FightStartTransAction* poTrans = GET_GAMEOBJECT(FightStartTransAction, GAMEOBJ_START_FIGHT_TRANS);
    assert(poTrans);
    poTrans->SaveReq(rstReq, rstSsPkg.m_stHead.m_ullUin);

    GetPlayerAction* poAction = GET_GAMEOBJECT(GetPlayerAction, GAMEOBJ_GET_PLAYER_ACTION);
    assert(poAction);
    poAction->SetPlayerId(rstReq.m_ullUin);

    poTrans->AddAction(poAction);

    AsyncPvpTransFrame::Instance().ScheduleTransaction(poTrans);

    return 0;
}


int AsyncPvpSettleReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    SS_PKG_ASYNC_PVP_SETTLE_REQ& rstReq = rstSsPkg.m_stBody.m_stAsyncpvpSettleReq;

    FightSettleTransAction* poTrans = GET_GAMEOBJECT(FightSettleTransAction, GAMEOBJ_SETTLE_FIGHT_TRANS);
    assert(poTrans);
    poTrans->SaveReq(rstReq, rstSsPkg.m_stHead.m_ullUin);

    GetPlayerAction* poAction = GET_GAMEOBJECT(GetPlayerAction, GAMEOBJ_GET_PLAYER_ACTION);
    assert(poAction);
    poAction->SetPlayerId(rstSsPkg.m_stHead.m_ullUin);

    poTrans->AddAction(poAction);

    AsyncPvpTransFrame::Instance().ScheduleTransaction(poTrans);

	return 0;
}


int AsyncPvpRefreshOpponentReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
	SS_PKG_ASYNC_PVP_REFRESH_OPPONENT_REQ& rstReq = rstSsPkg.m_stBody.m_stAsyncpvpRefreshOpponentReq;

    RefreshOpponentTransAction* poTrans = GET_GAMEOBJECT(RefreshOpponentTransAction, GAMEOBJ_REFRESH_OPPONENT_TRANS);
    assert(poTrans);
    poTrans->SaveReq(rstReq, rstSsPkg.m_stHead.m_ullUin);

    GetPlayerAction* poAction = GET_GAMEOBJECT(GetPlayerAction, GAMEOBJ_GET_PLAYER_ACTION);
    assert(poAction);
    poAction->SetPlayerId(rstReq.m_ullUin);

    poTrans->AddAction(poAction);

    AsyncPvpTransFrame::Instance().ScheduleTransaction(poTrans);

    return 0;
}


int AsyncPvpGetDataReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    SS_PKG_ASYNC_PVP_GET_DATA_REQ& rstReq = rstSsPkg.m_stBody.m_stAsyncpvpGetDataReq;

    if (rstReq.m_bIsFirst == 0)
    {
        GetDataTransAction* poTrans = GET_GAMEOBJECT(GetDataTransAction, GAMEOBJ_GETDATA_TRANS);
        assert(poTrans);
        poTrans->SaveReq(rstReq, rstSsPkg.m_stHead.m_ullUin);

        GetPlayerAction* poAction = GET_GAMEOBJECT(GetPlayerAction, GAMEOBJ_GET_PLAYER_ACTION);
        assert(poAction);
        poAction->SetPlayerId(rstReq.m_ullUin);

        poTrans->AddAction(poAction);

        AsyncPvpTransFrame::Instance().ScheduleTransaction(poTrans);
    }
    else
    {
        AddDataTransAction* poTrans = GET_GAMEOBJECT(AddDataTransAction, GAMEOBJ_ADDDATA_TRANS);
        assert(poTrans);
        poTrans->SaveReq(rstReq, rstSsPkg.m_stHead.m_ullUin);

        CreatePlayerAction* poAction = GET_GAMEOBJECT(CreatePlayerAction, GAMEOBJ_CREATE_PLAYER_ACTION);
        assert(poAction);
        poAction->SetPlayerData(rstReq.m_stShowData);

        poTrans->AddAction(poAction);

        AsyncPvpTransFrame::Instance().ScheduleTransaction(poTrans);
    }

    return 0;
}


int AsyncPvpUptPlayerNtf::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    SS_PKG_ASYNC_PVP_UPT_PLAYER_NTF& rstNtf = rstSsPkg.m_stBody.m_stAsyncpvpUptPlayerNtf;

    AsyncPvpPlayerMgr::Instance().UpdatePlayer(rstNtf.m_stShowData);

    AsyncPvpTeamMgr::Instance().UpdateTeamData(rstNtf.m_stTeamData);

	return 0;
}

int AsyncPvpWorshippedNtf::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
	SS_PKG_ASYNC_PVP_WORSHIPPED_NTF& rstNtf = rstSsPkg.m_stBody.m_stAsyncpvpWorshippedNtf;

	WorshipTransAction* poTrans = GET_GAMEOBJECT(WorshipTransAction, GAMEOBJ_WORSHIP_TRANS);
	assert(poTrans);
	poTrans->SaveReq(rstNtf, rstSsPkg.m_stHead.m_ullUin);

	GetPlayerAction* poAction = GET_GAMEOBJECT(GetPlayerAction, GAMEOBJ_GET_PLAYER_ACTION);
	assert(poAction);
	poAction->SetPlayerId(rstSsPkg.m_stHead.m_ullUin);

	poTrans->AddAction(poAction);

	AsyncPvpTransFrame::Instance().ScheduleTransaction(poTrans);

	return 0;
}

//该协议最后未被调用，暂时就不改了
int AsyncPvpGetWorshipGold::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
	SS_PKG_ASYNC_PVP_GET_WORSHIP_GOLD_REQ& rstSsReq = rstSsPkg.m_stBody.m_stAsyncpvpGetWorshipGoldReq;
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_GET_WORSHIP_GOLD_RSP;
	m_stSsPkg.m_stHead.m_ullUin = rstSsReq.m_ullUin;
	SS_PKG_ASYNC_PVP_GET_WORSHIP_GOLD_RSP& rstSsRsp = m_stSsPkg.m_stBody.m_stAsyncpvpGetWorshipGoldRsp;
	rstSsRsp.m_nErrNo = ERR_NONE;

	AsyncPvpSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);

	return 0;
}

int AsyncPvpGetPlayerInfo::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    SS_PKG_ASYNC_PVP_GET_PLAYER_INFO_REQ& rstReq = rstSsPkg.m_stBody.m_stAsyncpvpGetPlayerInfoReq;

	GetPlayerInfoTransAction* poTrans = GET_GAMEOBJECT(GetPlayerInfoTransAction, GAMEOBJ_GET_PLAYER_INFO_TRANS);
	assert(poTrans);
	poTrans->SaveReq(rstReq, rstSsPkg.m_stHead.m_ullUin);

	GetPlayerAction* poAction = GET_GAMEOBJECT(GetPlayerAction, GAMEOBJ_GET_PLAYER_ACTION);
	assert(poAction);
	poAction->SetPlayerId(AsyncPvpRank::Instance().GetPlayerByRank(rstReq.m_dwOpponent));

	poTrans->AddAction(poAction);

	AsyncPvpTransFrame::Instance().ScheduleTransaction(poTrans);

	return 0;
}

