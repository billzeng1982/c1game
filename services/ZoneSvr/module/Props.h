#pragma once
#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"


class Props : public TSingleton<Props>
{

    //批量处理宝箱最大数量,处理时间不能超过50ms

public:
	Props();
	virtual ~Props(){}

private:
	DT_ITEM_PROPS m_stProps;

public:
	int Init();
	int Add(PlayerData* pstData, uint32_t dwId, int32_t iNum, uint32_t dwApproach);
    bool IsEnough(PlayerData* pstData, uint32_t dwId, uint32_t dwNum);
	uint32_t GetNum(PlayerData* pstData, uint32_t dwId);
	DT_ITEM_PROPS* Find(PlayerData* pstData, uint32_t dwId);
	int GetClassScore(uint32_t dwId);

	int AddDataForNewPlayer(PlayerData* pstData);
	int AddDataForDebug(PlayerData* pstData, DT_ITEM* pstItemList);

	int PurchaseProps(PlayerData* pstData, uint32_t dwId, uint32_t dwNum, SC_PKG_PURCHASE_RSP& rstScPkgBodyRsp);

	//道具合成
	int CompositeProps(PlayerData* pstData, uint32_t dwId, uint32_t dwNum, DT_SYNC_ITEM_INFO& rstSyncItemInfo);

    //  开宝箱,调用此函数需要保证id为保宝箱的物品ID
    int OpenTreasureBox(PlayerData* pstData, uint32_t dwPropId, uint32_t dwNum, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach);

	//  开选择宝箱
	int OpenChosenBox(PlayerData* pstData, uint32_t dwPropId, uint32_t dwNum, uint8_t index, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach);

	//	开等级宝箱
	int OpenLevelBox(PlayerData* pstData, uint32_t dwPropId, uint32_t dwNum, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach);

    //  通用领取奖励, 只需奖励ID即可
    int GetCommonAward(PlayerData* pstData, uint32_t dwAwardId, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach);

	//	是否为等级宝箱
	bool IsLevelBox(uint32_t dwPropId);
};


