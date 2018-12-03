#pragma once

#include "define.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include "ss_proto.h"
#include "cs_proto.h"
#include <map>
#include "../gamedata/GameDataMgr.h"

using namespace std;
using namespace PKGMETA;


class DailyChallenge : public TSingleton<DailyChallenge>
{
private:
    const static int SPECIAL_ATTR_VALUE = 5;
    const static uint32_t CONST_GCARD_SKILL_DETA = 5;

    typedef struct
    {
        uint8_t m_bCount;
        uint32_t m_RandomList[MAX_TROOP_NUM_PVP];
        RESDAILYCHALLENGEGENERALPOOL* m_pResPool;
    } RandomNode;

    typedef map<uint8_t, RandomNode> MapRandomNode_t;

public:
    DailyChallenge() {}
    ~DailyChallenge() {}

    bool Init();

    void UpdateServer();

    void UpdatePlayerData(PlayerData* pstData);

    int FightSettle(PlayerData* pstData, uint8_t bResult, DT_SYNC_ITEM_INFO& rstItemInfo);

    int CheckMatch(PlayerData* pstData);

    int RecvReward(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstItemInfo);

    int GenFakePlayer(PlayerData* pstData, DT_FIGHT_PLAYER_INFO& rstPlayerInfo, OUT uint32_t* pdwLi=NULL);

    int SkipFight(PlayerData* pstData, uint8_t bWinCount, DT_SYNC_ITEM_INFO& rstSyncInfo);

	int SetSiegeEquipment(PlayerData* pstData, uint8_t bType);

	int PurchaseBuff(PlayerData* pstData, CS_PKG_DAILY_CHALLENGE_BUY_BUFF_REQ& rstCsReq, SC_PKG_DAILY_CHALLENGE_BUY_BUFF_RSP& rstScRsp);

	void UpdateTroopInfo(PlayerData* pstData, uint8_t bResult, DT_DAILY_CHALLENGE_TROOP_INFO& rstSelfTroopInfo, DT_DAILY_CHALLENGE_TROOP_INFO& rstEnemyTroopInfo);

    //检查上阵武将今日是否已阵亡过,若是，则不允许开战
    int CheckGenerals(PlayerData* pstData);

    //随机敌人信息，获取战场信息
    int LoadDungeonInfo(PlayerData* pstData, uint32_t& rdwLi);

    int LoadSelfFightPlayerInfo(PlayerData* pstData);

private:
    void _ResetPlayerData(PlayerData* pstData);

    int _GenFakeGeneralList(PlayerData* pstData, DT_FIGHT_PLAYER_INFO& rstPlayerInfo);

    int _GenTroopData(uint32_t dwLi, DT_TROOP_INFO& rstTroopInfo);

    void _UpdateScoreRank(PlayerData* pstData);

    void _SendClearRankNtf();

    void _SendSettleRankNtf();

	bool _CheckShipNumIsLegal(PlayerData* pstData, uint8_t bWinCount);

	int _AddBuffBenifitHp(PlayerData* pstData, DT_DAILY_CHALLENGE_TROOP_INFO& rstSelfTroop);

    int _AddBuffBenifitMorale(PlayerData* pstData, DT_DAILY_CHALLENGE_TROOP_INFO& rstSelfTroop);

	int _IsGeneralGetHurt(PlayerData* pstData, uint32_t dwGeneralId);

	int _IsEnemyGetHurt(PlayerData* pstData, uint32_t dwGeneralId);

	int _RandomBuffs(PlayerData* pstData);

    bool _CheckGeneralAlive(PlayerData* pstData, uint32_t dwGeneralId);


	void _ClearHurtGeneralList(DT_DAILY_CHALLENGE_GENERALS_INFO& rstGeneralsInfo)	{ rstGeneralsInfo.m_bCount=0; }
private:
    SSPKG m_stSsPkg;

    bool m_bUptFlag;//刷新标志

    int m_iUptTimeHour; //刷新时间点
    uint64_t m_ullTimeStamp;//刷新时间戳

    uint8_t m_bRequireLevel;//开放等级

    uint8_t m_bWinTimesLimit;//胜利次数上限
    uint8_t m_bLoseTimesLimit;//失败次数上限

    MapRandomNode_t m_oRandomNodeMap;//随机节点
    MapRandomNode_t::iterator m_oIter; //迭代器

	float m_fMoraleMax;			// 士气上限
    float m_fInitMorale;        //初识士气值

	//每日挑战赛可购买buf个数
	uint16_t m_wBuffNum[MAX_VIP_LEVEL+1];

    uint8_t m_bRandomList[MAX_GUILD_LEVELS_NUM];
};
