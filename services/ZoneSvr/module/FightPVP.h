#pragma once
#include "common_proto.h"
#include "cs_proto.h"
#include "define.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include "ss_proto.h"
#include <map>

using namespace std;
using namespace PKGMETA;

/*
	数据结构和算法都不相同，3V3和6V6匹配单独各自实现
*/

// Match默认为3v3
class Fight3V3 : public TSingleton<Fight3V3>
{
public:
	Fight3V3();
	virtual ~Fight3V3();

	bool Init();

	void HandleELOSettle(PlayerData* pData, DT_FIGHT_PLAYER_INFO* pstOppoInfo, uint8_t bWinGroup);

private:
	bool _InitChampionScore();
	bool _InitLvIndex();

	void _HandleScore(PlayerData* pData, uint8_t bWinGroup);
	void _HandleEffiency(PlayerData* pData, DT_FIGHT_PLAYER_INFO* pstOppoInfo, uint8_t bWinGroup);

public:
	typedef map<uint32_t, uint32_t> MapScore2LvId_t;
private:
	uint32_t m_dwChampionScore;
	MapScore2LvId_t m_oMapScore2Id;
	uint8_t m_bStreakTimes;
	uint8_t m_bStreakScore;
};

class Fight6V6 : public TSingleton<Fight6V6>
{
public:
	Fight6V6();
	~Fight6V6();

	bool Init();

	void HandleELOSettle(PlayerData* pData, uint8_t bResult);

    void AddFightHistory(PlayerData* pData, DT_FIGHT_PLAYER_INFO* pstMyInfo, uint8_t bWinGroup, uint64_t ulTimeStamp);
    void UpdateTopList(uint8_t bTopListCount, DT_RANK_INFO* pstTopList);
    void GetTopList(DT_RANK_INFO* pstTopList, uint8_t* bTopListCount);

	int UpdateServer();
	void InitPlayerData(PlayerData* pstData);
    void UpdatePlayerData(PlayerData* pstData);

	int HandlePVPDailyReward(PlayerData* pstData, uint32_t dwProgress, SC_PKG_COMMON_REWARD_RSP& rstScPkgBodyRsp);
	int HandlePVPSeasonReward(PlayerData* pstData, uint32_t dwProgress, SC_PKG_COMMON_REWARD_RSP& rstScPkgBodyRsp);

    int SendRankInfoToRankSvr(PlayerData* poPlayerData, SS_PKG_FIGHT_SETTLE_NTF& rstSettleNtf);

private:
	bool _InitChampionScore();
	bool _InitLvIndex();
	void _InitTopList();

	void _HandleScore(PlayerData* pData, uint8_t bWinGroup);
	void _HandleEffiency(PlayerData* pData, DT_FIGHT_PLAYER_INFO* pstOppoInfo, uint8_t bWinGroup);

    void _AddOneHistory(DT_ROLE_PVP_HISTORY_INFO* pHistoryInfo, DT_FIGHT_PLAYER_INFO* pstMyInfo,  uint8_t bWinGroup);

public:
	typedef map<uint32_t, uint32_t> MapScore2LvId_t;

private:
	uint32_t m_dwChampionScore;
	MapScore2LvId_t m_oMapScore2Id;
    uint8_t m_bTopListCount;
    DT_RANK_INFO m_TopList[MAX_RANK_TOP_NUM];
    int m_iUpdateTime;
    uint64_t m_ullUpdateLastTime;
	uint8_t m_bStreakTimes;
	uint8_t m_bStreakScore;

    PKGMETA::SCPKG   m_stScPkg;
    PKGMETA::SSPKG   m_stSsPkg;
};

