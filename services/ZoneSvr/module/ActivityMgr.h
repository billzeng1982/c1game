#pragma once
#include "define.h"
#include "common_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include "cs_proto.h"
#include "../gamedata/GameDataMgr.h"

using namespace std;
using namespace PKGMETA;

class ActivityMgr : public TSingleton<ActivityMgr>
{
public:
	ActivityMgr() { m_ullUptTimeStamp = 0; }
	virtual ~ActivityMgr() {}
	bool Init();
	// ����������
	int UpdateServer();
	int UpdatePlayerData(PlayerData* poPlayerData);

	int ActivityPveSettle(PlayerData* poPlayerData, CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsPkgBodyReq, SC_PKG_FIGHT_PVE_SETTLE_RSP& rstScPkgBodyRsp);

	int ActivitySkipFight(PlayerData* poPlayerData, CS_PKG_PVE_SKIP_FIGHT_REQ& rstCsPkgReq, SC_PKG_PVE_SKIP_FIGHT_RSP& rstScPkgRsp);

	int HandleMajestLvUp(PlayerData* poPlayerData);
    int ResetCount(PlayerData* poPlayerData, CS_PKG_PVE_PURCHASE_TIMES_REQ& rstPurchaseTimesReq, SC_PKG_PVE_PURCHASE_TIMES_RSP& rstPurchaseTimesRsp);

	int CheckActivityPveCd(PlayerData* poPlayerData, uint32_t dwActivityType); //�Ƿ��ѹ���ȴʱ��
	int ResetColdTime(PlayerData* poPlayerData, uint32_t dwActivityType, DT_SYNC_ITEM_INFO& rstSyncItemInfo); //���ûcd


private:

	void _UpdateColdDownTime(PlayerData* poPlayerData, int iActivityType, DT_ACTIVITY_INFO& rstActivityInfo, uint64_t m_ullCdStartTimeStamp, uint64_t& m_ullColdDowmTimeStamp); //���»CDʱ���

	void _SendSynMsg(PlayerData* poPlayerData, int iActivityType, DT_ACTIVITY_INFO& rstActivityInfo);							// ����ͬ����Ϣ

	void _SendLevelEvent(PlayerData* poPlayerData, uint32_t dwLevelId, uint8_t bType, uint32_t dwValue);

private:
	uint64_t m_ullUptTimeStamp;
	int m_iDailyResetTime;

	//�ڶ����ý����Ļ�Ծ
	int m_iGuildVitality;

	uint64_t m_ullColdTimeGoldList[MAX_VIP_LEVEL+1];
	uint64_t m_ullColdTimeGrainList[MAX_VIP_LEVEL+1];
	uint64_t m_ullColdTimeRoundList[MAX_VIP_LEVEL+1];
	int32_t m_iColdResetCostDiaGold;
	int32_t m_iColdResetCostDiaGrain;
	int32_t m_iColdResetCostDiaRound;

	uint8_t m_szActPveFinishCnt[MAX_ACTIVITY_PVE_NUM]; // pve���ɴ���
	PKGMETA::SCPKG m_stScPkg;
	PKGMETA::SSPKG m_stSsPkg;
};

