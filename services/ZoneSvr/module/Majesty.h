#pragma once
#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include "../gamedata/GameDataMgr.h"

using namespace PKGMETA;

class Majesty : public TSingleton<Majesty>
{
public:
	Majesty();
	virtual ~Majesty(){};

private:
	int m_iMaxLv;
	uint32_t m_dwMaxExp;
	int m_iUpdateTime;
	int m_iChgNamePrice;

	SCPkg m_stScPkg;

public:
	bool Init();
	int SetName(PlayerData* pstData, char* szName);
	void UpdatePlayerData(PlayerData* pstData); 					// 刷新玩家数据
    void CalMajestyLv(PlayerData* pstData);

	uint32_t AddExp(PlayerData* pstData, uint32_t dwValue);
	uint32_t GetExp(PlayerData* pstData);
	uint32_t GetExpNextLv(PlayerData* pstData);

	uint16_t GetLevel(PlayerData* pstData);

	void GmSetLv(PlayerData* pstData, uint16_t Lv);

	int ChgImageId(PlayerData* pstData, uint16_t wImageId);

	int SetDefaultItems(DT_ROLE_MAJESTY_INFO& rstMajestyInfo, DT_ROLE_ITEMS_INFO& rstRoleItemsInfo);

	uint32_t GetBattleMSkill(PlayerData* pstData, uint8_t bBattleArrayType = BATTLE_ARRAY_TYPE_NORMAL /*阵容类型*/);
	int SetBattleMSkill(PlayerData* pstData, uint32_t bMSId, uint8_t bBattleArrayType = BATTLE_ARRAY_TYPE_NORMAL /*阵容类型*/);

    int SetBattleTactics(PlayerData* pstData, uint8_t bTacticsType, uint8_t bBattleArrayType = BATTLE_ARRAY_TYPE_NORMAL /*阵容类型*/);
    int GetBattleTactics(PlayerData* pstData, uint8_t bTacticsType, uint8_t bBattleArrayType = BATTLE_ARRAY_TYPE_NORMAL /*阵容类型*/);

	int GetBattleGeneralList(PlayerData* pstData, uint8_t& bCount, uint16_t* pastGeneralId[], uint8_t bBattleArrayType = BATTLE_ARRAY_TYPE_NORMAL);
	int SetBattleGeneralList(PlayerData* pstData, uint8_t bCount, uint32_t astGeneralId[], uint8_t bBattleArrayType = BATTLE_ARRAY_TYPE_NORMAL);

	int GetBattleSkillFlag(PlayerData* pstData, uint8_t& bSkillFlag, uint8_t bBattleArrayType = BATTLE_ARRAY_TYPE_NORMAL);
	int SetBattleSkillFlag(PlayerData* pstData, uint8_t bSkillFlag, uint8_t bBattleArrayType = BATTLE_ARRAY_TYPE_NORMAL);

	DT_BATTLE_ARRAY_INFO* GetBattleArrayInfo(PlayerData* pstData, uint8_t bBattleArrayType = BATTLE_ARRAY_TYPE_NORMAL, bool bAddNew = true);

	int IsArriveLevel(PlayerData* pstData, int iId);

	int GetPersonalizedSig(PlayerData* pstData, char* stPersonalizedSig);

	int ChangeRoleName(Player* poPlayer, char* stRoleName);

	int GetChgNamePrice()	{ return m_iChgNamePrice; }

private:
	void _HandleLvReward(PlayerData* pstData);

	int _CheckItemId(PlayerData* pstData, uint16_t wId);

	int _CheckBattleGeneralList(PlayerData* pstData, uint8_t& bCount, uint32_t astGeneralId[]);
};

