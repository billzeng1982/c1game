#pragma once

#include "TransactionFrame.h"
#include "../module/player/AsyncPvpPlayer.h"
#include "ss_proto.h"
#include "strutil.h"

using namespace PKGMETA;


//获取玩家信息
class GetPlayerAction : public IAction
{
public:
    GetPlayerAction() { this->Reset(); }
    virtual ~GetPlayerAction() {}

    void SetPlayerId(uint64_t ullPlayerId) { m_ullPlayerId = ullPlayerId; }

    uint64_t GetPlayerId() { return m_ullPlayerId; }

    AsyncPvpPlayer* GetPlayer() { return m_poPlayer; }

    virtual void Reset();

    virtual const char* GetActionName() { return "GetPlayerAction"; }

    virtual int Execute(Transaction* pObjTrans);

    virtual void OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen);

private:
    uint64_t m_ullPlayerId;
    AsyncPvpPlayer* m_poPlayer;
};


//创建玩家
class CreatePlayerAction : public IAction
{
public:
    CreatePlayerAction() { this->Reset(); }
    virtual ~CreatePlayerAction() {}

    void SetPlayerData(DT_ASYNC_PVP_PLAYER_SHOW_DATA& rstShowData)
    {
        m_stShowData = rstShowData;
    }

    AsyncPvpPlayer* GetPlayer() { return m_poPlayer; }

    uint64_t GetPlayerId() { return m_stShowData.m_stBaseInfo.m_ullUin; }

    virtual void Reset();

    virtual const char* GetActionName() { return "CreatePlayerAction"; }

    virtual int Execute(Transaction* pObjTrans);

    virtual void OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen);

private:
    DT_ASYNC_PVP_PLAYER_SHOW_DATA m_stShowData;
    AsyncPvpPlayer* m_poPlayer;
};


//获取阵容信息
class GetTeamAction : public IAction
{
public:
    GetTeamAction() { this->Reset(); }
    virtual ~GetTeamAction() {}

    void SetPlayerId(uint64_t ullPlayerId) { m_ullPlayerId = ullPlayerId; }

    uint64_t GetPlayerId() { return m_ullPlayerId; }

    DT_ASYNC_PVP_PLAYER_TEAM_DATA* GetTeam() { return m_pstTeam; }

    virtual void Reset();

    virtual const char* GetActionName() { return "GetTeamAction"; }

    virtual int Execute(Transaction* pObjTrans);

    virtual void OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen);

private:
    uint64_t m_ullPlayerId;
    DT_ASYNC_PVP_PLAYER_TEAM_DATA* m_pstTeam;
};


//创建阵容
class CreateTeamAction : public IAction
{
public:
    CreateTeamAction() { this->Reset(); }
    virtual ~CreateTeamAction() {}

    void SetTeam(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData)
    {
        m_stTeamData = rstTeamData;
    }

    DT_ASYNC_PVP_PLAYER_TEAM_DATA* GetTeam() { return m_pstTeam; }

    virtual void Reset();

    virtual const char* GetActionName() { return "CreateTeamAction"; }

    virtual int Execute(Transaction* pObjTrans);

    virtual void OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen);

private:
    DT_ASYNC_PVP_PLAYER_TEAM_DATA m_stTeamData;
    DT_ASYNC_PVP_PLAYER_TEAM_DATA* m_pstTeam;
};


//获取数据请求
class GetDataTransAction : public Transaction
{
public:
    GetDataTransAction() { this->Reset(); }
    virtual ~GetDataTransAction() {}

    void SaveReq(SS_PKG_ASYNC_PVP_GET_DATA_REQ& rstSsReq, uint64_t ullUin)
    {
        m_ullUin = ullUin;
        m_stSsReq = rstSsReq;
    }

    virtual void Reset();

    virtual const char* GetTransactionName() { return "GetDataTransAction"; }

    virtual void OnFinished(int iErrCode);

    virtual void OnActionFinished(TActionIndex iActionIdx);

private:
    int16_t m_nErrNo;
    uint64_t m_ullUin;
    SS_PKG_ASYNC_PVP_GET_DATA_REQ m_stSsReq;
};


//新增数据请求
class AddDataTransAction : public Transaction
{
public:
    AddDataTransAction() { this->Reset(); }
    virtual ~AddDataTransAction() {}

    void SaveReq(SS_PKG_ASYNC_PVP_GET_DATA_REQ& rstSsReq, uint64_t ullUin)
    {
        m_ullUin = ullUin;
        m_stSsReq = rstSsReq;
    }

    virtual void Reset();

    virtual const char* GetTransactionName() { return "AddDataTransAction"; }

    virtual void OnFinished(int iErrCode);

    virtual void OnActionFinished(TActionIndex iActionIdx);

private:
    int16_t m_nErrNo;
    uint64_t m_ullUin;
    SS_PKG_ASYNC_PVP_GET_DATA_REQ m_stSsReq;
};


//刷新对手请求
class RefreshOpponentTransAction : public Transaction
{
public:
    RefreshOpponentTransAction() { this->Reset(); }
    virtual ~RefreshOpponentTransAction() {}

    void SaveReq(SS_PKG_ASYNC_PVP_REFRESH_OPPONENT_REQ& rstSsReq, uint64_t ullUin)
    {
        m_ullUin = ullUin;
        m_stSsReq = rstSsReq;
    }

    virtual void Reset();

    virtual const char* GetTransactionName() { return "RefreshOpponentTransAction"; }

    virtual void OnFinished(int iErrCode);

    virtual void OnActionFinished(TActionIndex iActionIdx);

private:
    int16_t m_nErrNo;
    uint64_t m_ullUin;
    SS_PKG_ASYNC_PVP_REFRESH_OPPONENT_REQ m_stSsReq;
};


//获取前N名
class GetTopListTransAction : public Transaction
{
public:
    GetTopListTransAction() { this->Reset(); }
    virtual ~GetTopListTransAction() {}

    virtual void Reset();

    virtual const char* GetTransactionName() { return "GetTopListTransAction"; }

    virtual void OnFinished(int iErrCode);
};


//战斗开始请求
class FightStartTransAction : public Transaction
{
public:
    FightStartTransAction() { this->Reset(); }
    virtual ~FightStartTransAction() {}

    void SaveReq(SS_PKG_ASYNC_PVP_START_REQ& rstSsReq, uint64_t ullUin)
    {
        m_ullUin = ullUin;
        m_stSsReq = rstSsReq;
    }

    virtual void Reset();

    virtual const char* GetTransactionName() { return "FightStartTransAction"; }

    virtual void OnFinished(int iErrCode);

    virtual void OnActionFinished(TActionIndex iActionIdx);

private:
    int16_t m_nErrNo;
    uint64_t m_ullUin;
    SS_PKG_ASYNC_PVP_START_REQ m_stSsReq;
};


//战斗结算请求
class FightSettleTransAction : public Transaction
{
public:
    FightSettleTransAction() { this->Reset(); }
    virtual ~FightSettleTransAction() {}

    void SaveReq(SS_PKG_ASYNC_PVP_SETTLE_REQ& rstSsReq, uint64_t ullUin)
    {
        m_ullUin = ullUin;
        m_stSsReq = rstSsReq;
    }

    virtual void Reset();

    virtual const char* GetTransactionName() { return "FightSettleTransAction"; }

    virtual void OnFinished(int iErrCode);

    virtual void OnActionFinished(TActionIndex iActionIdx);

private:
    int16_t m_nErrNo;
    uint64_t m_ullUin;
    DT_ASYNC_PVP_FIGHT_RECORD m_stRecord;
    SS_PKG_ASYNC_PVP_SETTLE_REQ m_stSsReq;
};


class WorshipTransAction : public Transaction
{
public:
	WorshipTransAction() { this->Reset(); }
	virtual ~WorshipTransAction() {}

	void SaveReq(SS_PKG_ASYNC_PVP_WORSHIPPED_NTF& rstSsNtf, uint64_t ullUin)
	{
		m_ullUin = ullUin;
		m_stSsNtf = rstSsNtf;
	}
	virtual void Reset();

	virtual const char* GetTransactionName() { return "WorshipTransAction"; }

	virtual void OnFinished(int iErrCode);

	virtual void OnActionFinished(TActionIndex iActionIdx);

private:
	int16_t m_nErrNo;
	uint64_t m_ullUin;
	SS_PKG_ASYNC_PVP_WORSHIPPED_NTF m_stSsNtf;
};


class WorshipMailTransAction : public Transaction
{
private:
	static const int WORSHIPPED_CANDIDATE_MAX_NUM = 200;
	static const int DAILY_WORSHIPPED_SETTLE_MAIL_ID = 10008;

public:
	WorshipMailTransAction() { this->Reset(); }
	virtual ~WorshipMailTransAction() {}


	virtual void Reset();

	virtual const char* GetTransactionName() { return "WorshipMailTransAction"; }

	virtual void OnFinished(int iErrCode);

	virtual void OnActionFinished(TActionIndex iActionIdx);

private:
	int16_t m_nErrNo;
};

//获取玩家信息
class GetPlayerInfoTransAction : public Transaction
{
public:
    GetPlayerInfoTransAction() { this->Reset(); }
    virtual ~GetPlayerInfoTransAction() {}

    void SaveReq(SS_PKG_ASYNC_PVP_GET_PLAYER_INFO_REQ& rstSsReq, uint64_t ullUin)
    {
        m_ullUin = ullUin;
        m_stSsReq = rstSsReq;
    }

    virtual void Reset();

    virtual const char* GetTransactionName() { return "GetPlayerInfoTransAction"; }

    virtual void OnFinished(int iErrCode);

    virtual void OnActionFinished(TActionIndex iActionIdx);

private:
    int16_t m_nErrNo;
    uint64_t m_ullUin;
    SS_PKG_ASYNC_PVP_GET_PLAYER_INFO_REQ m_stSsReq;
};

