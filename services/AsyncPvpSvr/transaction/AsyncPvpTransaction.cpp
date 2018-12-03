#include "AsyncPvpTransaction.h"
#include "LogMacros.h"
#include "../module/rank/AsyncPvpRank.h"
#include "../module/player/AsyncPvpPlayerMgr.h"
#include "GameTime.h"
#include "GameObjectPool.h"
#include"../framework/AsyncPvpSvrMsgLayer.h"
#include "../module/team/AsyncPvpTeamMgr.h"
#include "../module/fight/AsyncPvpFightMgr.h"
#include "../gamedata/GameDataMgr.h"

using namespace PKGMETA;

/*----------------------------------------------------------------------------------------------------*/

void GetPlayerAction::Reset()
{
    IAction::Reset();
    m_ullPlayerId = 0;
    m_poPlayer = NULL;
}


int GetPlayerAction::Execute(Transaction* pObjTrans)
{
    TActionToken ullToken = this->GetToken();

    if (m_ullPlayerId == 0)
    {
        LOGERR_r("GetPlayerAction execute failed, PlayerId is 0");
        return -1;
    }

    m_poPlayer = AsyncPvpPlayerMgr::Instance().GetPlayer(m_ullPlayerId, ullToken);
    if (!m_poPlayer)
    {
        return 0;
    }
    else
    {
        this->SetFiniFlag(1);
        return 1;
    }
}


void GetPlayerAction::OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen)
{
    assert(iActionIdx == m_iIndex);

    m_poPlayer = (AsyncPvpPlayer*)pResult;
    if (!m_poPlayer)
    {
        LOGERR_r("Player(%lu) is not found", this->GetPlayerId());
    }

    this->SetFiniFlag(1); // 表示动作执行完成
}



/*----------------------------------------------------------------------------------------------------*/


void CreatePlayerAction::Reset()
{
    IAction::Reset();
    m_poPlayer = NULL;
}


int CreatePlayerAction::Execute(Transaction* pObjTrans)
{
    TActionToken ullToken = this->GetToken();

    int iRet = AsyncPvpPlayerMgr::Instance().CreatePlayer(m_stShowData, ullToken);
    if (iRet == ERR_NONE)
    {
        return 0;
    }
    else if (iRet == ERR_ALREADY_EXISTED)
    {
        LOGRUN_r("Player(%lu) send create player req, but already exist, so update player", m_stShowData.m_stBaseInfo.m_ullUin);
        m_poPlayer = AsyncPvpPlayerMgr::Instance().GetPlayer(m_stShowData.m_stBaseInfo.m_ullUin, ullToken);
        m_poPlayer->UptShowData(m_stShowData);
        this->SetFiniFlag(1);
        return 1;
    }
    else
    {
        return iRet;
    }
}


void CreatePlayerAction::OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen)
{
    assert(iActionIdx == m_iIndex);

    m_poPlayer = (AsyncPvpPlayer*)pResult;

    this->SetFiniFlag(1); // 表示动作执行完成
}



/*----------------------------------------------------------------------------------------------------*/


void GetTeamAction::Reset()
{
    IAction::Reset();
    m_ullPlayerId = 0;
    m_pstTeam = NULL;
}


int GetTeamAction::Execute(Transaction* pObjTrans)
{
    TActionToken ullToken = this->GetToken();

    m_pstTeam = AsyncPvpTeamMgr::Instance().GetTeamData(m_ullPlayerId, ullToken);
    if (!m_pstTeam)
    {
        return 0;
    }
    else
    {
        this->SetFiniFlag(1);
        return 1;
    }
}


void GetTeamAction::OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen)
{
    assert(iActionIdx == m_iIndex);

    m_pstTeam = (DT_ASYNC_PVP_PLAYER_TEAM_DATA*)pResult;
    if (!m_pstTeam)
    {
        LOGERR_r("Player(%lu) team is not found", this->GetPlayerId());
    }

    this->SetFiniFlag(1); // 表示动作执行完成
}



/*----------------------------------------------------------------------------------------------------*/


void CreateTeamAction::Reset()
{
    IAction::Reset();
    m_pstTeam = NULL;
}


int CreateTeamAction::Execute(Transaction* pObjTrans)
{
    TActionToken ullToken = this->GetToken();

    int iRet = AsyncPvpTeamMgr::Instance().CreateTeamData(m_stTeamData, ullToken);
    if (iRet == ERR_NONE)
    {
        return 0;
    }
    else if (iRet == ERR_ALREADY_EXISTED)
    {
        m_pstTeam = AsyncPvpTeamMgr::Instance().GetTeamData(m_stTeamData.m_ullUin, ullToken);
        *m_pstTeam = m_stTeamData;
        LOGRUN_r("Player(%lu) send create team req, but already exist, so update team", m_stTeamData.m_ullUin);
        this->SetFiniFlag(1);
        return 1;
    }
    else
    {
        this->SetFiniFlag(1);
        return iRet;
    }
}


void CreateTeamAction::OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen)
{
    assert(iActionIdx == m_iIndex);

    m_pstTeam = (DT_ASYNC_PVP_PLAYER_TEAM_DATA*)pResult;

    this->SetFiniFlag(1); // 表示动作执行完成
}



/*----------------------------------------------------------------------------------------------------*/



void GetDataTransAction::Reset()
{
    Transaction::Reset();
    m_ullUin = 0;
    m_nErrNo = ERR_NONE;
}


void GetDataTransAction::OnActionFinished(TActionIndex iActionIdx)
{
    //取得self数据的action完成后
    if (iActionIdx == 0)
    {
        GetPlayerAction* poGetSelfAction = (GetPlayerAction*)(this->GetAction(0));
        AsyncPvpPlayer* poSelf = poGetSelfAction->GetPlayer();
        if (!poSelf)
        {
            LOGERR_r("Player(%lu) GetDataTransAction failed", poGetSelfAction->GetPlayerId());
            m_nErrNo = ERR_SYS;
            return;
        }

        //获取对手的数据
        uint32_t astOpponentList[MAX_NUM_ASYNC_PVP_OPPONENT_MYSQL];
        uint8_t bCount = poSelf->GetOpponentList(astOpponentList);
        if (bCount == 0)
        {
            //对手个数为0则直接返回
            return;
        }

        //创建获取对手的组合action
        CompositeAction* poCompositeAction = GET_GAMEOBJECT(CompositeAction, GAMEOBJ_COMPOSITE_ACTION);
        assert(poCompositeAction);

        for (uint8_t i = 0; i < bCount; i++)
        {
            GetPlayerAction* poGetOpponentAction = GET_GAMEOBJECT(GetPlayerAction, GAMEOBJ_GET_PLAYER_ACTION);
            assert(poGetOpponentAction);
            poGetOpponentAction->SetPlayerId(AsyncPvpRank::Instance().GetPlayerByRank(astOpponentList[i]));

            poCompositeAction->AddAction(poGetOpponentAction);
        }

        this->AddAction(poCompositeAction);
    }
}


void GetDataTransAction::OnFinished(int iErrCode)
{
    SSPKG stSsPkg;
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_GET_DATA_RSP;
    stSsPkg.m_stHead.m_ullUin = m_ullUin;
    SS_PKG_ASYNC_PVP_GET_DATA_RSP& rstRsp = stSsPkg.m_stBody.m_stAsyncpvpGetDataRsp;
    rstRsp.m_stSelfInfo.m_bCount = 0;
    rstRsp.m_bRecordCount = 0;
    rstRsp.m_bOpponentCount = 0;
    rstRsp.m_nErrNo = ERR_NONE;

    do
    {
        if (iErrCode != TRANC_ERR_SUCCESS)
        {
            LOGERR_r("Player(%lu) GetDataTransAction excute failed, ErrNo(%d)", m_ullUin, iErrCode);
            rstRsp.m_nErrNo = ERR_SYS;
            break;
        }

        if (m_nErrNo != ERR_NONE)
        {
            rstRsp.m_nErrNo = m_nErrNo;
            break;
        }

        GetPlayerAction* poGetSelfAction = (GetPlayerAction*)this->GetAction(0);
        AsyncPvpPlayer* poSelf = poGetSelfAction->GetPlayer();
        assert(poSelf);

        poSelf->GetShowData(rstRsp.m_stSelfInfo);
        rstRsp.m_bRecordCount = poSelf->GetRecordList(rstRsp.m_astRecordList);
        rstRsp.m_bOpponentCount = AsyncPvpRank::Instance().GetTop10List(rstRsp.m_astOpponentList);

        CompositeAction* poCompositeAction = (CompositeAction*)this->GetAction(1);
        if (!poCompositeAction)
        {
            break;
        }

        uint8_t bCount = poCompositeAction->GetActionCount();
        for (uint8_t i=0; i<bCount; i++)
        {
            GetPlayerAction* poGetOpponentAction = (GetPlayerAction*)poCompositeAction->GetAction(i);
            AsyncPvpPlayer* poOpponent = poGetOpponentAction->GetPlayer();
            if (poOpponent)
            {
                poOpponent->GetShowData(rstRsp.m_astOpponentList[rstRsp.m_bOpponentCount++]);
            }
        }
    }while(false);

    AsyncPvpSvrMsgLayer::Instance().SendToZoneSvr(stSsPkg);
}


/*----------------------------------------------------------------------------------------------------*/


void AddDataTransAction::Reset()
{
    Transaction::Reset();
    m_ullUin = 0;
    m_nErrNo = ERR_NONE;
}

void AddDataTransAction::OnActionFinished(TActionIndex iActionIdx)
{
    //取得self数据的action完成后
    if (iActionIdx == 0)
    {
        CreatePlayerAction* poCreateSelfAction = (CreatePlayerAction*)(this->GetAction(0));
        AsyncPvpPlayer* poSelf = poCreateSelfAction->GetPlayer();
        if (!poSelf)
        {
            LOGERR_r("Player(%lu) CreatePlayerAction::[create player] failed", poCreateSelfAction->GetPlayerId());
            m_nErrNo = ERR_SYS;
            return;
        }

        CreateTeamAction* poCreateTeamAction = GET_GAMEOBJECT(CreateTeamAction, GAMEOBJ_CREATE_TEAM_ACTION);
        assert(poCreateTeamAction);
        poCreateTeamAction->SetTeam(m_stSsReq.m_stTeamData);
        this->AddAction(poCreateTeamAction);
    }
    else if (iActionIdx == 1)
    {
        CreatePlayerAction* poCreateSelfAction = (CreatePlayerAction*)(this->GetAction(0));
        AsyncPvpPlayer* poSelf = poCreateSelfAction->GetPlayer();

        CreateTeamAction* poCreateTeamAction = (CreateTeamAction*)(this->GetAction(1));
        DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam = poCreateTeamAction->GetTeam();
        if (!pstTeam)
        {
            LOGERR_r("Player(%lu) CreatePlayerAction::[create team] failed", poCreateSelfAction->GetPlayerId());
            AsyncPvpPlayerMgr::Instance().DelPlayer(poCreateSelfAction->GetPlayerId());
            m_nErrNo = ERR_SYS;
            return;
        }

        //获取对手数据
        uint32_t astOpponentList[MAX_NUM_ASYNC_PVP_OPPONENT_MYSQL];
        uint8_t bCount = poSelf->GetOpponentList(astOpponentList);
        if (bCount == 0)
        {
            //对手个数为0则直接返回
            return;
        }

        //创建获取对手的组合action
        CompositeAction* poCompositeAction = GET_GAMEOBJECT(CompositeAction, GAMEOBJ_COMPOSITE_ACTION);
        assert(poCompositeAction);

        for (uint8_t i = 0; i < bCount; i++)
        {
            GetPlayerAction* poGetOpponentAction = GET_GAMEOBJECT(GetPlayerAction, GAMEOBJ_GET_PLAYER_ACTION);
            assert(poGetOpponentAction);
            poGetOpponentAction->SetPlayerId(AsyncPvpRank::Instance().GetPlayerByRank(astOpponentList[i]));

            poCompositeAction->AddAction(poGetOpponentAction);
        }
        this->AddAction(poCompositeAction);
    }
}

void AddDataTransAction::OnFinished(int iErrCode)
{
    SSPKG stSsPkg;
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_GET_DATA_RSP;
    stSsPkg.m_stHead.m_ullUin = m_ullUin;
    SS_PKG_ASYNC_PVP_GET_DATA_RSP& rstRsp = stSsPkg.m_stBody.m_stAsyncpvpGetDataRsp;
    rstRsp.m_stSelfInfo.m_bCount = 0;
    rstRsp.m_bRecordCount = 0;
    rstRsp.m_bOpponentCount = 0;
    rstRsp.m_nErrNo = ERR_NONE;

    do
    {
        if (iErrCode != TRANC_ERR_SUCCESS)
        {
            LOGERR_r("Player(%lu) AddDataTransAction excute failed, ErrNo=(%d)", m_ullUin, iErrCode);
            rstRsp.m_nErrNo = ERR_SYS;
            break;
        }

        if (m_nErrNo != ERR_NONE)
        {
            rstRsp.m_nErrNo = m_nErrNo;
            break;
        }

        CreatePlayerAction* poCreateSelfAction = (CreatePlayerAction*)this->GetAction(0);
        AsyncPvpPlayer* poSelf = poCreateSelfAction->GetPlayer();
        assert(poSelf);

        poSelf->GetShowData(rstRsp.m_stSelfInfo);
        rstRsp.m_bRecordCount = poSelf->GetRecordList(rstRsp.m_astRecordList);
        rstRsp.m_bOpponentCount = AsyncPvpRank::Instance().GetTop10List(rstRsp.m_astOpponentList);

        CompositeAction* poCompositeAction = (CompositeAction*)this->GetAction(2);
        if (!poCompositeAction)
        {
            break;
        }

        uint8_t bCount = poCompositeAction->GetActionCount();
        for (uint8_t i=0; i<bCount; i++)
        {
            GetPlayerAction* poGetOpponentAction = (GetPlayerAction*)poCompositeAction->GetAction(i);
            AsyncPvpPlayer* poOpponent = poGetOpponentAction->GetPlayer();
            if (poOpponent)
            {
                poOpponent->GetShowData(rstRsp.m_astOpponentList[rstRsp.m_bOpponentCount++]);
            }
        }
    }while(false);

    AsyncPvpSvrMsgLayer::Instance().SendToZoneSvr(stSsPkg);
}


/*----------------------------------------------------------------------------------------------------*/


void RefreshOpponentTransAction::Reset()
{
    Transaction::Reset();
    m_ullUin = 0;
    m_nErrNo = ERR_NONE;
}


void RefreshOpponentTransAction::OnActionFinished(TActionIndex iActionIdx)
{
    //取得self数据的action完成后
    if (iActionIdx == 0)
    {
        GetPlayerAction* poGetSelfAction = (GetPlayerAction*)(this->GetAction(0));
        AsyncPvpPlayer* poPlayer = poGetSelfAction->GetPlayer();
        if (!poPlayer)
        {
            LOGERR_r("Player(%lu) RefreshOpponentTransAction failed", poGetSelfAction->GetPlayerId());
            m_nErrNo = ERR_SYS;
            return;
        }

        //刷新对手
        poPlayer->RefreshOpponentList();

        uint32_t astOpponentList[MAX_NUM_ASYNC_PVP_OPPONENT_MYSQL];
        uint8_t bCount = poPlayer->GetOpponentList(astOpponentList);
        if (bCount == 0)
        {
            return;
        }

        //创建获取对手的组合action
        CompositeAction* poCompositeAction = GET_GAMEOBJECT(CompositeAction, GAMEOBJ_COMPOSITE_ACTION);
        assert(poCompositeAction);

        for (uint8_t i = 0; i < bCount; i++)
        {
            GetPlayerAction* poGetOpponentAction = GET_GAMEOBJECT(GetPlayerAction, GAMEOBJ_GET_PLAYER_ACTION);
            assert(poGetOpponentAction);
            poGetOpponentAction->SetPlayerId(AsyncPvpRank::Instance().GetPlayerByRank(astOpponentList[i]));

            poCompositeAction->AddAction(poGetOpponentAction);
        }
        this->AddAction(poCompositeAction);
    }
}

void RefreshOpponentTransAction::OnFinished(int iErrCode)
{
    SSPKG stSsPkg;
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_REFRESH_OPPONENT_RSP;
    stSsPkg.m_stHead.m_ullUin = m_ullUin;
    SS_PKG_ASYNC_PVP_REFRESH_OPPONENT_RSP& rstRsp = stSsPkg.m_stBody.m_stAsyncpvpRefreshOpponentRsp;
    rstRsp.m_bOpponentCount = 0;
    rstRsp.m_nErrNo = ERR_NONE;
    rstRsp.m_bIsAuto = m_stSsReq.m_bIsAuto;

    do
    {
        if (iErrCode != TRANC_ERR_SUCCESS)
        {
            LOGERR_r("Player(%lu) RefreshOpponentTransAction excute failed, ErrNo=(%d)", m_ullUin, iErrCode);
            rstRsp.m_nErrNo = ERR_SYS;
            break;
        }

        if (m_nErrNo != ERR_NONE)
        {
            rstRsp.m_nErrNo = m_nErrNo;
            break;
        }

        //获取前10的数据
        rstRsp.m_bOpponentCount = AsyncPvpRank::Instance().GetTop10List(rstRsp.m_astOpponentList);

        GetPlayerAction* poGetSelfAction = (GetPlayerAction*)(this->GetAction(0));
        if (!poGetSelfAction)
        {
            break;
        }
        AsyncPvpPlayer* poSelf = poGetSelfAction->GetPlayer();
        if (poSelf)
        {
            rstRsp.m_dwMyRank = poSelf->GetRank();
        }

        //获取对手列表
        CompositeAction* poCompositeAction = (CompositeAction*)this->GetAction(1);
        if (!poCompositeAction)
        {
            break;
        }

        uint8_t bCount = poCompositeAction->GetActionCount();
        for (uint8_t i=0; i<bCount; i++)
        {
            GetPlayerAction* poGetOpponentAction = (GetPlayerAction*)poCompositeAction->GetAction(i);
            AsyncPvpPlayer* poOpponent = poGetOpponentAction->GetPlayer();
            if (poOpponent)
            {
                poOpponent->GetShowData(rstRsp.m_astOpponentList[rstRsp.m_bOpponentCount++]);
            }
        }
    }while(false);

    AsyncPvpSvrMsgLayer::Instance().SendToZoneSvr(stSsPkg);
}


/*----------------------------------------------------------------------------------------------------*/


void GetTopListTransAction::Reset()
{
    Transaction::Reset();
}

void GetTopListTransAction::OnFinished(int iErrCode)
{
    if (iErrCode != TRANC_ERR_SUCCESS)
    {
        LOGERR_r("GetTopListTransAction excute failed, ErrNo=(%d)", iErrCode);
        assert(false);
        return;
    }

    //初始化TopList
    AsyncPvpRank::Instance().InitTopList();
}


/*----------------------------------------------------------------------------------------------------*/


void FightStartTransAction::Reset()
{
    Transaction::Reset();
    m_nErrNo = ERR_NONE;
    m_ullUin = 0;
}


void FightStartTransAction::OnActionFinished(TActionIndex iActionIdx)
{
    if (iActionIdx == 0)
    {
        GetPlayerAction* poGetSelfAction = (GetPlayerAction*)(this->GetAction(0));
        AsyncPvpPlayer* poSelf = poGetSelfAction->GetPlayer();
        if (!poSelf)
        {
            LOGERR_r("Player(%lu) FightStartTransAction failed", poGetSelfAction->GetPlayerId());
            m_nErrNo = ERR_SYS;
            return;
        }

        //检查对手是否在对手列表中
        if (!poSelf->CheckOpponent(m_stSsReq.m_dwOpponent))
        {
            m_nErrNo = ERR_SYS;
            return;
        }

        uint64_t ullOpponentId = AsyncPvpRank::Instance().GetPlayerByRank(m_stSsReq.m_dwOpponent);

        //创建获取对手展示数据和阵容数据的组合action
        CompositeAction* poCompositeAction = GET_GAMEOBJECT(CompositeAction, GAMEOBJ_COMPOSITE_ACTION);
        assert(poCompositeAction);

        GetPlayerAction* poGetOpponentAction = GET_GAMEOBJECT(GetPlayerAction, GAMEOBJ_GET_PLAYER_ACTION);
        assert(poGetOpponentAction);
        poGetOpponentAction->SetPlayerId(ullOpponentId);
        poCompositeAction->AddAction(poGetOpponentAction);

        //真玩家需要获取TeamData,假玩家不需要获取TeamData
        if (ullOpponentId > MAX_NUM_ASYNC_PVP_FAKE_PLAYER_UIN)
        {
            GetTeamAction* poGetTeamAction = GET_GAMEOBJECT(GetTeamAction, GAMEOBJ_GET_TEAM_ACTION);
            assert(poGetTeamAction);
            poGetTeamAction->SetPlayerId(ullOpponentId);
            poCompositeAction->AddAction(poGetTeamAction);
        }

        this->AddAction(poCompositeAction);
    }
}


void FightStartTransAction::OnFinished(int iErrCode)
{
    SSPKG stSsPkg;
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_START_RSP;
    stSsPkg.m_stHead.m_ullUin = m_ullUin;
    SS_PKG_ASYNC_PVP_START_RSP& rstRsp = stSsPkg.m_stBody.m_stAsyncpvpStartRsp;
    rstRsp.m_stOpponentInfo.m_bIsFake = 1;
    rstRsp.m_stOpponentInfo.m_stPlayerShowData.m_bCount = 0;
    rstRsp.m_nErrNo = ERR_NONE;

    do
    {
        if (iErrCode != TRANC_ERR_SUCCESS)
        {
            LOGERR_r("Player(%lu) FightStartTransAction excute failed, ErrNo=(%d)", m_ullUin, iErrCode);
            rstRsp.m_nErrNo = ERR_SYS;
            break;
        }

        if (m_nErrNo != ERR_NONE)
        {
            rstRsp.m_nErrNo = m_nErrNo;
            break;
        }

        GetPlayerAction* poGetSelfAction = (GetPlayerAction*)this->GetAction(0);
        AsyncPvpPlayer* poSelf = poGetSelfAction->GetPlayer();

        CompositeAction* poCompositeAction = (CompositeAction*)this->GetAction(1);
        GetPlayerAction* poGetOpponentAction = (GetPlayerAction*)poCompositeAction->GetAction(0);
        AsyncPvpPlayer* poOpponent = poGetOpponentAction->GetPlayer();
        if (!poOpponent)
        {
            rstRsp.m_nErrNo = ERR_SYS;
            break;
        }
        poOpponent->GetShowData(rstRsp.m_stOpponentInfo.m_stPlayerShowData);

        if (poOpponent->GetPlayerId() > MAX_NUM_ASYNC_PVP_FAKE_PLAYER_UIN)
        {
            GetTeamAction* poGetTeamAction = (GetTeamAction*)poCompositeAction->GetAction(1);
            DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam = poGetTeamAction->GetTeam();
            if (!pstTeam)
            {
                rstRsp.m_nErrNo = ERR_SYS;
                break;
            }
            rstRsp.m_stOpponentInfo.m_stPlayerTeamData.m_stTeamData = *pstTeam;
            rstRsp.m_stOpponentInfo.m_bIsFake = 0;
        }
        else
        {
            rstRsp.m_stOpponentInfo.m_bIsFake = 1;
        }

        rstRsp.m_nErrNo = AsyncPvpFightMgr::Instance().Create(poSelf, poOpponent, &rstRsp.m_ullRaceNo);
    }while(false);

    AsyncPvpSvrMsgLayer::Instance().SendToZoneSvr(stSsPkg);
}


/*----------------------------------------------------------------------------------------------------*/


void FightSettleTransAction::Reset()
{
    Transaction::Reset();
    m_nErrNo = ERR_NONE;
    m_ullUin = 0;
}


void FightSettleTransAction::OnActionFinished(TActionIndex iActionIdx)
{
    if (iActionIdx == 0)
    {
        GetPlayerAction* poGetSelfAction = (GetPlayerAction*)(this->GetAction(0));
        AsyncPvpPlayer* poSelf = poGetSelfAction->GetPlayer();
        if (!poSelf)
        {
            LOGERR_r("Player(%lu) FightSettleTransAction failed", poGetSelfAction->GetPlayerId());
            m_nErrNo = ERR_SYS;
            return;
        }

        m_nErrNo = AsyncPvpFightMgr::Instance().Settle(m_stSsReq.m_ullRaceNo, m_stSsReq.m_bWinGroup, m_stRecord);
        if (m_nErrNo != ERR_NONE)
        {
            return;
        }

        //结算成功后刷新一次对手
        poSelf->RefreshOpponentList();

        uint32_t astOpponentList[MAX_NUM_ASYNC_PVP_OPPONENT_MYSQL];
        uint8_t bCount = poSelf->GetOpponentList(astOpponentList);
        if (bCount == 0)
        {
            return;
        }
        //创建获取对手的组合action
        CompositeAction* poCompositeAction = GET_GAMEOBJECT(CompositeAction, GAMEOBJ_COMPOSITE_ACTION);
        assert(poCompositeAction);

        for (uint8_t i = 0; i < bCount; i++)
        {
            GetPlayerAction* poGetOpponentAction = GET_GAMEOBJECT(GetPlayerAction, GAMEOBJ_GET_PLAYER_ACTION);
            assert(poGetOpponentAction);
            poGetOpponentAction->SetPlayerId(AsyncPvpRank::Instance().GetPlayerByRank(astOpponentList[i]));

            poCompositeAction->AddAction(poGetOpponentAction);
        }
        this->AddAction(poCompositeAction);
    }
}


void FightSettleTransAction::OnFinished(int iErrCode)
{
    SSPKG stSsPkg;
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_SETTLE_RSP;
    stSsPkg.m_stHead.m_ullUin = m_ullUin;
    SS_PKG_ASYNC_PVP_SETTLE_RSP& rstRsp = stSsPkg.m_stBody.m_stAsyncpvpSettleRsp;
    rstRsp.m_bOpponentCount = 0;
    rstRsp.m_nErrNo = ERR_NONE;

    do
    {
        if (iErrCode != TRANC_ERR_SUCCESS)
        {
            LOGERR_r("Player(%lu) FightSettleTransAction excute failed, ErrNo=(%d)", m_ullUin, iErrCode);
            rstRsp.m_nErrNo = ERR_SYS;
            break;
        }

        if (m_nErrNo != ERR_NONE)
        {
            rstRsp.m_nErrNo = m_nErrNo;
            break;
        }

        GetPlayerAction* poGetSelfAction = (GetPlayerAction*)this->GetAction(0);
        AsyncPvpPlayer* poSelf = poGetSelfAction->GetPlayer();

        //获取自己的基本信息
        poSelf ->GetBaseData(rstRsp.m_stSelfBaseInfo);

        //比赛记录
        rstRsp.m_stRecord = m_stRecord;

        //获取前10的数据
        rstRsp.m_bOpponentCount = AsyncPvpRank::Instance().GetTop10List(rstRsp.m_astOpponentList);

        //获取对手列表
        CompositeAction* poCompositeAction = (CompositeAction*)this->GetAction(1);
        if (!poCompositeAction)
        {
            break;
        }

        uint8_t bCount = poCompositeAction->GetActionCount();
        for (uint8_t i=0; i<bCount; i++)
        {
            GetPlayerAction* poGetOpponentAction = (GetPlayerAction*)poCompositeAction->GetAction(i);
            AsyncPvpPlayer* poOpponent = poGetOpponentAction->GetPlayer();
            if (poOpponent)
            {
                poOpponent->GetShowData(rstRsp.m_astOpponentList[rstRsp.m_bOpponentCount++]);
            }
        }
    }while(false);

    AsyncPvpSvrMsgLayer::Instance().SendToZoneSvr(stSsPkg);
}


/*----------------------------------------------------------------------------------------------------*/



void WorshipTransAction::Reset()
{
	Transaction::Reset();
	m_ullUin = 0;
	m_nErrNo = ERR_NONE;
}


void WorshipTransAction::OnActionFinished(TActionIndex iActionIdx)
{
	//取得self数据的action完成后
	if (iActionIdx == 0)
	{
		GetPlayerAction* poGetSelfAction = (GetPlayerAction*)(this->GetAction(0));
		AsyncPvpPlayer* poSelf = poGetSelfAction->GetPlayer();
		if (!poSelf)
		{
			LOGERR_r("Player(%lu) GetDataTransAction failed", poGetSelfAction->GetPlayerId());
			m_nErrNo = ERR_SYS;
			return;
		}

		//创建获取对手的组合action
		CompositeAction* poCompositeAction = GET_GAMEOBJECT(CompositeAction, GAMEOBJ_COMPOSITE_ACTION);
		assert(poCompositeAction);

		for (uint8_t i = 0; i < m_stSsNtf.m_bCount; i++)
		{
			GetPlayerAction* poGetWorshippedPlayerAction = GET_GAMEOBJECT(GetPlayerAction, GAMEOBJ_GET_PLAYER_ACTION);
			assert(poGetWorshippedPlayerAction);
			poGetWorshippedPlayerAction->SetPlayerId(m_stSsNtf.m_WorshippedList[i]);

			poCompositeAction->AddAction(poGetWorshippedPlayerAction);
		}

		this->AddAction(poCompositeAction);
	}
}


void WorshipTransAction::OnFinished(int iErrCode)
{
	do
	{
		if (iErrCode != TRANC_ERR_SUCCESS)
		{
			LOGERR_r("Player(%lu) GetDataTransAction excute failed, ErrNo(%d)", m_ullUin, iErrCode);
			break;
		}

		if (m_nErrNo != ERR_NONE)
		{
			break;
		}

		GetPlayerAction* poGetSelfAction = (GetPlayerAction*)this->GetAction(0);
		AsyncPvpPlayer* poSelf = poGetSelfAction->GetPlayer();
		assert(poSelf);

		CompositeAction* poCompositeAction = (CompositeAction*)this->GetAction(1);
		if (!poCompositeAction)
		{
			break;
		}

		uint8_t bCount = poCompositeAction->GetActionCount();
		for (uint8_t i=0; i<bCount; i++)
		{
			GetPlayerAction* poGetWorshippedPlayerAction = (GetPlayerAction*)poCompositeAction->GetAction(i);
			AsyncPvpPlayer* poWorshippedPlayers = poGetWorshippedPlayerAction->GetPlayer();
			if (poWorshippedPlayers)
			{
				poWorshippedPlayers->AddWorshipGold(m_stSsNtf.m_iWorshippedGoldOnce, m_stSsNtf.m_iWorshippedGoldMax);
			}
		}
	}while(false);


}


/*----------------------------------------------------------------------------------------------------*/

void WorshipMailTransAction::Reset()
{
	Transaction::Reset();
	m_nErrNo = ERR_NONE;
}

void WorshipMailTransAction::OnActionFinished(TActionIndex iActionIdx)
{
	return;
}

void WorshipMailTransAction::OnFinished(int iErrCode)
{
	RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(DAILY_WORSHIPPED_SETTLE_MAIL_ID);
	if (NULL == poResPriMail)
	{
		LOGERR("Can't find the mail<id %u>", DAILY_WORSHIPPED_SETTLE_MAIL_ID);
		return;
	}

	SSPKG stSsPkg;

	stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
	SS_PKG_MAIL_ADD_REQ& rstMailAddReq = stSsPkg.m_stBody.m_stMailAddReq;
	rstMailAddReq.m_nUinCount = 0;
	DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
	rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
	rstMailData.m_bState = MAIL_STATE_UNOPENED;
	StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
	StrCpy(rstMailData.m_szContent, poResPriMail->m_szContent, MAX_MAIL_CONTENT_LEN);
	rstMailData.m_ullFromUin = 0;
    rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();

	do
	{
		CompositeAction* poCompositeAction = (CompositeAction*)this->GetAction(0);
		if (!poCompositeAction)
		{
			break;
		}

		uint8_t bCount = poCompositeAction->GetActionCount();
		for (uint8_t i=0; i<bCount; i++)
		{
			GetPlayerAction* poGetWorshippedPlayerAction = (GetPlayerAction*)poCompositeAction->GetAction(i);
			AsyncPvpPlayer* poWorshippedPlayer = poGetWorshippedPlayerAction->GetPlayer();
			if (NULL == poWorshippedPlayer)
			{
				return;
			}

			uint64_t ullUin = poWorshippedPlayer->GetPlayerId();

			int iWorshippedGold = (int)poWorshippedPlayer->GetWorshipGold();

			if ( iWorshippedGold == 0 )
			{
				continue;
			}

			rstMailData.m_bAttachmentCount = 1;
			rstMailData.m_astAttachmentList[0].m_bItemType = ITEM_TYPE_GOLD;
			rstMailData.m_astAttachmentList[0].m_dwItemId = 0;
			rstMailData.m_astAttachmentList[0].m_iValueChg = iWorshippedGold;
			poWorshippedPlayer->ClearWorshipGold();
			rstMailAddReq.m_UinList[rstMailAddReq.m_nUinCount++] = ullUin;

			AsyncPvpSvrMsgLayer::Instance().SendToMailSvr(stSsPkg);
			rstMailAddReq.m_nUinCount = 0;
		}

	} while (false);
	//获取对手列表

};
/*----------------------------------------------------------------------------------------------------*/


void GetPlayerInfoTransAction::Reset()
{
	Transaction::Reset();
	m_nErrNo = ERR_NONE;
}

void GetPlayerInfoTransAction::OnActionFinished(TActionIndex iActionIdx)
{
	return;
}

void GetPlayerInfoTransAction::OnFinished(int iErrCode)
{
    SSPKG stSsPkg;
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_GET_PLAYER_INFO_RSP;
    stSsPkg.m_stHead.m_ullUin = m_ullUin;
    SS_PKG_ASYNC_PVP_GET_PLAYER_INFO_RSP& rstRsp = stSsPkg.m_stBody.m_stAsyncpvpGetPlayerInfoRsp;
    rstRsp.m_nErrNo = ERR_NONE;

    do
    {
        if (iErrCode != TRANC_ERR_SUCCESS)
        {
            LOGERR_r("Player(%lu) GetPlayerInfoTransAction excute failed, ErrNo=(%d)", m_ullUin, iErrCode);
            rstRsp.m_nErrNo = ERR_SYS;
            break;
        }

        if (m_nErrNo != ERR_NONE)
        {
            rstRsp.m_nErrNo = m_nErrNo;
            break;
        }

        GetPlayerAction* poGetPlayerAction = (GetPlayerAction*)this->GetAction(0);
        AsyncPvpPlayer* poPlayer = poGetPlayerAction->GetPlayer();
        if (poPlayer)
        {
            poPlayer->GetShowData(rstRsp.m_stOpponentInfo);
        }
    }while(false);

    AsyncPvpSvrMsgLayer::Instance().SendToZoneSvr(stSsPkg);
}

