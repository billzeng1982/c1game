#pragma once

#include "define.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"

using namespace PKGMETA;

class SkillPoint : public TSingleton<SkillPoint>
{
public:
    SkillPoint() {}
    ~SkillPoint() {}

    bool Init();
    bool InitPlayerData(PlayerData* pstData); // 非初始化登陆调用
	void UpdatePlayerData(PlayerData* pstData); // 刷新玩家数据

	int PurchaseSP(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo);

public:
    bool IsEnough(PlayerData* pstData, uint16_t wValue);
    uint32_t Add(PlayerData* pstData, int32_t iValue);

private:
    //回复间隔时间，根据VIP等级,时间有所不同
    uint32_t m_dwUpdateInterval[MAX_VIP_LEVEL+1];
	//技能点上限，根据VIP等级变化
	uint16_t m_wTopSPLimit[MAX_VIP_LEVEL+1];
	//技能点可购买次数，根据VIP等级变化
	uint16_t m_wBuySPLTimes[MAX_VIP_LEVEL+1];
	//购买sp索引
	int m_iSPGetIndex;
	int m_iDiamondCostIndex;
};
