#pragma once
#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"

#define MSKILL_LEVEL_DISABLE	(0)			// 未开启 为 0
#define MSKILL_LEVEL_MIN		(1)			// 开启等级 1
#define MSKILL_LEVEL_MAX		(10)		// 满级 10
#define MSKILL_ID_INVALID		(0)			// 无效军师技

using namespace PKGMETA;

class MasterSkill : public TSingleton<MasterSkill>
{
public:
	MasterSkill();
	~MasterSkill(){}

    bool Init();

public:
	int UpgradeMS(PlayerData* pstData, SC_PKG_MS_UPGRADE_RSP& rstMSkillUpdateRsp);
	int UpgradeMSTotal(PlayerData* pstData, SC_PKG_MS_UPGRADE_RSP& rstMSkillUpdateRsp);

	// 不再需要合成
    int Composite(PlayerData* pstData, uint32_t dwId, SC_PKG_MS_COMPOSITE_RSP& rstScPkgRsp);

	int AddDataForNewPlayer(PlayerData* pstData);
	int AddDataForDebug(PlayerData* pstData, DT_ITEM* pstItemList, int iIdx = 0);

	uint32_t GetDefaultMSId();
	DT_ITEM_MSKILL* GetMSkillInfo(PlayerData* pstData, uint32_t dwMSId);

private:
	int Add(PlayerData* pstData, uint32_t dwMSId, uint8_t bLevel = 1);
	DT_ITEM_MSKILL* Find(PlayerData* pstData, uint32_t dwMSId);

private:

	uint32_t m_dwDefaultMSId;
	DT_ITEM_MSKILL m_stMasterSkill;
};


