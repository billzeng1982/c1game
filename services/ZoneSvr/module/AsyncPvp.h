#pragma once

#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include "GameDataMgr.h"

using namespace PKGMETA;

class AsyncPvp : public TSingleton<AsyncPvp>
{
public:
    AsyncPvp();
    ~AsyncPvp();

    bool Init();

    void UpdateServer();

    void UpdatePlayerData(PlayerData* pstData);

    int GetShowData(PlayerData* pstData, DT_ASYNC_PVP_PLAYER_SHOW_DATA& rstShowData);

    int GetTeamData(PlayerData* pstData, DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData);

    void UptToAsyncSvr(PlayerData* pstData);

    int CheckFight(PlayerData* pstData);

    int SkipFight(PlayerData* pstData, uint8_t bFightCount, DT_SYNC_ITEM_INFO& rstSyncItemInfo);

    int CheckRefresh(PlayerData* pstData);

    int AfterRefresh(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo);

    int AfterStartFight(PlayerData* pstData);

    int FightTimesBuy(PlayerData* pstData, uint8_t bBuyCount, DT_SYNC_ITEM_INFO& rstSyncItemInfo);

    int GetTopList(DT_ASYNC_PVP_PLAYER_SHOW_DATA   astTopList[], uint64_t& ullTimeStamp);

    void RefreshTopList(DT_ASYNC_PVP_PLAYER_SHOW_DATA   astTopList[], uint64_t ullTimeStamp);

	int CheckColdTime(PlayerData* pstData);

	int CheckResetColdTime(PlayerData* pstData);

	void SettleResetDia(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo);

	void UpdateCdTime(PlayerData* pstData, uint64_t& rullCdTimeStamp);

	int ResetCdTime(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo);

	int Worship(PlayerData* pstData, CS_PKG_ASYNC_PVP_WORSHIP_REQ& rstCsReq, SC_PKG_ASYNC_PVP_WORSHIP_RSP& rstScRsp); //膜拜

	void GetWorshipInfo(PlayerData* pstData, DT_ASYNC_PVP_PLAYER_SHOW_DATA* pastOpponentList, uint8_t bCount);

	int AddWorshipGold(PlayerData* pstData, int32_t iGoldNum, SC_PKG_ASYNC_PVP_GET_WORSHIP_GOLD_RSP& rstScRsp);

private:
    void _ResetPlayerData(PlayerData* pstData);

	void _FreshWorshipList(PlayerData* pstData, DT_ASYNC_PVP_PLAYER_SHOW_DATA* pastOpponentList); //刷新膜拜列表

private:
    SSPKG m_stSsPkg;

    //每日免费刷新次数
    int m_iFreeRefreshTimes;

    //每日免费战斗次数
    int m_iFreeFightTimes;

    //战斗次数购买消耗
    RESCONSUME* m_pResFightBuyConsume;

    //刷新消耗
    RESCONSUME* m_pResRefreshConsume;

    //刷新相关参数
    int m_iUptTimeHour;
    bool m_bUptFlag;
    uint64_t m_ullLastUptTime;

	//冷却相关参数
	uint64_t m_ullColdTimeList[MAX_VIP_LEVEL+1];
	int32_t m_iColdResetCostDia;

	//膜拜次数冷却相关参数
	int m_iWorshipTimeHour1;
	int m_iWorshipTimeHour2;
	bool m_bWorshipUptFlag;
	uint64_t m_ullLastWorshipUptTime;

	//被膜拜相关参数
	uint64_t m_ullWorshippedLastUptTime;

	//膜拜奖励
	int32_t m_iWorshipperGoldOnce;
	int32_t m_iWorshipperGoldMax;
	int32_t m_iWorshippedGoldOnce;
	int32_t m_iWorshippedGoldMax;


	//排行榜
    uint64_t m_ullTopListTimeStamp;
    DT_ASYNC_PVP_PLAYER_SHOW_DATA m_astTopList[MAX_RANK_TOP_NUM];
};