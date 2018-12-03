#pragma once
#include "define.h"
#include "player/PlayerData.h"
#include "ss_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "GeneralCard.h"

class Match : public TSingleton<Match>
{
public:
    Match();
    virtual ~Match();

private:
    PKGMETA::SCPKG m_stScPkg;
    PKGMETA::SSPKG m_stSsPkg;

public:
    int Start(PlayerData* poData, uint8_t bMatchType);
    void Cancel(PlayerData* poData, uint8_t bMatchType);
    int CheckStart(PlayerData* poData, uint8_t bMatchType);   //检查参数等
    int InitFightPlayerInfo(PlayerData* poData, uint8_t bMatchType);
    int InitOneTroopInfo(PlayerData* poData, DT_ITEM_GCARD* pstGCard, OUT DT_TROOP_INFO& rstTroopInfo);
    int InitOneTroopInfo(PlayerData* poData, uint32_t dwGcardId, OUT DT_TROOP_INFO& rstTroopInfo);

	//初始化指定阵容的战斗数据 rstArrayFightInfo最好bzero
	int InitBattleArrayFightInfo(PlayerData* poData, uint8_t bArrayType, OUT DT_BATTLE_ARRAY_FIGHT_INFO& rstArrayFightInfo);
};

