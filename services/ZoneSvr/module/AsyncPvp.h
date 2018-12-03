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

	int Worship(PlayerData* pstData, CS_PKG_ASYNC_PVP_WORSHIP_REQ& rstCsReq, SC_PKG_ASYNC_PVP_WORSHIP_RSP& rstScRsp); //Ĥ��

	void GetWorshipInfo(PlayerData* pstData, DT_ASYNC_PVP_PLAYER_SHOW_DATA* pastOpponentList, uint8_t bCount);

	int AddWorshipGold(PlayerData* pstData, int32_t iGoldNum, SC_PKG_ASYNC_PVP_GET_WORSHIP_GOLD_RSP& rstScRsp);

private:
    void _ResetPlayerData(PlayerData* pstData);

	void _FreshWorshipList(PlayerData* pstData, DT_ASYNC_PVP_PLAYER_SHOW_DATA* pastOpponentList); //ˢ��Ĥ���б�

private:
    SSPKG m_stSsPkg;

    //ÿ�����ˢ�´���
    int m_iFreeRefreshTimes;

    //ÿ�����ս������
    int m_iFreeFightTimes;

    //ս��������������
    RESCONSUME* m_pResFightBuyConsume;

    //ˢ������
    RESCONSUME* m_pResRefreshConsume;

    //ˢ����ز���
    int m_iUptTimeHour;
    bool m_bUptFlag;
    uint64_t m_ullLastUptTime;

	//��ȴ��ز���
	uint64_t m_ullColdTimeList[MAX_VIP_LEVEL+1];
	int32_t m_iColdResetCostDia;

	//Ĥ�ݴ�����ȴ��ز���
	int m_iWorshipTimeHour1;
	int m_iWorshipTimeHour2;
	bool m_bWorshipUptFlag;
	uint64_t m_ullLastWorshipUptTime;

	//��Ĥ����ز���
	uint64_t m_ullWorshippedLastUptTime;

	//Ĥ�ݽ���
	int32_t m_iWorshipperGoldOnce;
	int32_t m_iWorshipperGoldMax;
	int32_t m_iWorshippedGoldOnce;
	int32_t m_iWorshippedGoldMax;


	//���а�
    uint64_t m_ullTopListTimeStamp;
    DT_ASYNC_PVP_PLAYER_SHOW_DATA m_astTopList[MAX_RANK_TOP_NUM];
};