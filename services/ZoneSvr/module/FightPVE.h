#pragma once
#include "singleton.h"
#include "player/PlayerData.h"
#include "cs_proto.h"
#include "dwlog_svr.h"
#include "../gamedata/GameDataMgr.h"

using namespace PKGMETA;

class FightPVE : public TSingleton<FightPVE>
{
public:
	FightPVE();
	virtual ~FightPVE(){}

public:
	int Init(PlayerData* pstData);

	// 服务器更新
	int UpdateServer();
	int UpdatePlayerData(PlayerData* pstData);

	// pve 正常结算
	bool AddReward(PlayerData* pstData, uint8_t bChapterType, uint32_t dwPveLevelId, SC_PKG_FIGHT_PVE_SETTLE_RSP& rSettleRsp, uint8_t bTutorial);
	// pve 扫荡结算
	bool AddReward(PlayerData* pstData, uint8_t bChapterType, uint32_t dwPveLevelId, SC_PKG_PVE_SKIP_FIGHT_RSP& rSkipFightRsp);

	int CanPlayLevel(PlayerData* pstData, uint32_t dwPveLevelId);
	bool CanSkipLevel(PlayerData* pstData, uint32_t dwPveLevelId, uint32_t dwSkipTimes);
	int GetPveChallengeTimesLeft(PlayerData* pstData, uint32_t dwPveLevelId);

	// 更新挑战记录记录
	int UpdateRecord(PlayerData* pstData, CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsSettleReq, SC_PKG_FIGHT_PVE_SETTLE_RSP& rstScSettleRsp, uint8_t bChapterType);
	void UpdateRecord(PlayerData* pstData, CS_PKG_PVE_SKIP_FIGHT_REQ& rstCsSkipReq, SC_PKG_PVE_SKIP_FIGHT_RSP& rstScSkipRsp, uint8_t bChapterType);

    //  获取通关宝箱奖励
    int GetTreasure(PlayerData* pstData, CS_PKG_PVE_GET_TREASURE_REQ& rstCsReq, SC_PKG_PVE_GET_TREASURE_RSP& rstScRsp);
	void UpdateRecord4Debug(PlayerData* pstData, uint32_t dwPveLevelId);
	int UpdateRecordChapter(PlayerData* pstData, uint32_t dwPveLevelId, uint8_t bStarAdd);

	// 购买成功更新
    int ResetChallengeTimes(PlayerData* pstData, CS_PKG_PVE_PURCHASE_TIMES_REQ& rstPurchaseTimesReq, SC_PKG_PVE_PURCHASE_TIMES_RSP& rstPurchaseTimesRsp);
	int UpdatePveChallengeTimes(PlayerData* pstData, uint32_t dwPveLevelId, uint32_t dwPurchaseTimes);
	int HandleChapterRewardMsg(PlayerData* pstData, CS_PKG_PVE_CHAPTER_REWARD_REQ& rstCsPkgBodyReq, SC_PKG_PVE_CHAPTER_REWARD_RSP& rstScPkgBodyRsp);


private:
	// 数据操作
	DT_PVE_LEVEL_INFO* FindLevel(PlayerData* pstData, uint32_t dwPveLevelId);
	int AddLevel(PlayerData* pstData, uint32_t dwPveLevelId, uint8_t bIsPassed = 0);

	DT_PVE_CHAPTER_INFO* FindChapter(PlayerData* pstData, uint8_t bChapterType, uint8_t bChapterId);
	int AddChapter(PlayerData* pstData, uint8_t bChapterType, uint8_t bChapterId, uint32_t dwRewardId);

	int _IsDropItem(uint32_t dwProbability, int dwMinCount, int dwMaxCount);
    void _AddRandomDropItem(PlayerData* pstData, RESFIGHTLEVELPL *pLevelPL, DT_SYNC_ITEM_INFO& rstSyncItemInfo, int iDouble = 1);

	void _InitRewardArr(RESPVECHAPTERREWARD* pResReward, int iIdx, uint8_t* arrType, uint32_t* arrId, uint32_t* arrNum);
	int _FindIdByRes(RESFIGHTLEVELPL *pLevelPL);

	DT_PVE_LEVEL_INFO m_stPveLevlData;
	DT_PVE_CHAPTER_INFO m_stPveChapterData;

public:
	uint64_t m_ullUpdateLastTime;
};

