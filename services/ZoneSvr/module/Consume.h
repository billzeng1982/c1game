#pragma once
#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"

class Consume : public TSingleton<Consume>
{
public:
    Consume(){}
    virtual ~Consume(){}

public:
    bool IsEnough(PlayerData* pstData, uint8_t bType, uint32_t dwId, uint32_t dwValue);

    bool IsEnough(PlayerData* pstData, uint8_t bType, uint32_t dwValue);
    uint32_t Add(PlayerData* pstData, uint8_t bType, int32_t iValue, uint32_t dwApproach);

	uint32_t BaseAdd(PlayerData* pstData, uint8_t bType, int32_t iValue, uint32_t dwApproach, INOUT uint32_t& rdwOutValue);

    bool IsEnoughGold(PlayerData* pstData, uint32_t dwValue);
    uint32_t AddGold(PlayerData* pstData, int32_t iValue, uint32_t dwApproach);

    bool IsEnoughDiamond(PlayerData* pstData, uint32_t dwValue);
    uint32_t AddDiamond(PlayerData* pstData, int32_t iValue, uint32_t dwApproach);

    bool IsEnoughYuan(PlayerData* pstData, uint32_t dwValue);
    uint32_t AddYuan(PlayerData* pstData, int32_t iValue, uint32_t dwApproach);

	bool IsEnoughGeneralCoin(PlayerData* pstData, uint32_t dwValue);
	uint32_t AddGeneralCoin(PlayerData* pstData, int32_t iValue, uint32_t dwApproach);

	bool IsEnoughEquipCoin(PlayerData* pstData, uint32_t dwValue);
	uint32_t AddEquipCoin(PlayerData* pstData, int32_t iValue, uint32_t dwApproach);

	bool IsEnoughSyncPVPCoin(PlayerData* pstData, uint32_t dwValue);
	uint32_t AddSyncPVPCoin(PlayerData* pstData, int32_t iValue, uint32_t dwApproach);

	bool IsEnoughAsyncPVPCoin(PlayerData* pstData, uint32_t dwValue);
	uint32_t AddAsyncPVPCoin(PlayerData* pstData, int32_t iValue, uint32_t dwApproach);

	bool IsEnoughPeakArenaCoin(PlayerData* pstData, uint32_t dwValue);
	uint32_t AddPeakArenaCoin(PlayerData* pstData, int32_t iValue, uint32_t dwApproach);

    bool IsEnoughAwakeEquipCoin(PlayerData* pstData, uint32_t dwValue);
    uint32_t AddAwakeEquipCoin(PlayerData* pstData, int32_t iValue, uint32_t dwApproach);

	bool IsEnoughExpeditionCoin(PlayerData* pstData, uint32_t dwValue);
	uint32_t AddExpeditionCoin(PlayerData* pstData, int32_t iValue, uint32_t dwApproach);

	bool IsEnoughGuildContribution(PlayerData* pstData, uint32_t dwValue);
	uint32_t AddGuildContribution(PlayerData* pstData, int32_t iValue, uint32_t dwApproach);

    int PurchaseGold(PlayerData* pstData, uint32_t dwNum, SC_PKG_PURCHASE_RSP& rstScPkgBodyRsp);
};
