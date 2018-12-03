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

    //��������佫�����Ƿ���������,���ǣ�������ս
    int CheckGenerals(PlayerData* pstData);

    //���������Ϣ����ȡս����Ϣ
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

    bool m_bUptFlag;//ˢ�±�־

    int m_iUptTimeHour; //ˢ��ʱ���
    uint64_t m_ullTimeStamp;//ˢ��ʱ���

    uint8_t m_bRequireLevel;//���ŵȼ�

    uint8_t m_bWinTimesLimit;//ʤ����������
    uint8_t m_bLoseTimesLimit;//ʧ�ܴ�������

    MapRandomNode_t m_oRandomNodeMap;//����ڵ�
    MapRandomNode_t::iterator m_oIter; //������

	float m_fMoraleMax;			// ʿ������
    float m_fInitMorale;        //��ʶʿ��ֵ

	//ÿ����ս���ɹ���buf����
	uint16_t m_wBuffNum[MAX_VIP_LEVEL+1];

    uint8_t m_bRandomList[MAX_GUILD_LEVELS_NUM];
};
