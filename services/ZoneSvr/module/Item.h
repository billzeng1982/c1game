#pragma once
#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"

class Item : public TSingleton<Item>
{
public:
    static const int CONST_SHOW_PROPERTY_NO = 0;        //不展示
    static const int CONST_SHOW_PROPERTY_NORMAL = 1;    //普通展示
    static const int CONST_SHOW_PROPERTY_EXTRA = 2;     //额外展示

public:
    Item();
    virtual ~Item();

public:
    //  目前支持counsme中的检测和 Props检测,后续可添加
    bool IsEnough(PlayerData* pstData, uint8_t bItemType, uint32_t dwId, int32_t iNum);

    //Item奖励,默认展示
    int RewardItem(PlayerData* pstData, uint8_t bItemType, uint32_t dwId, int32_t iNum,
        DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach, uint8_t bShowProperty = CONST_SHOW_PROPERTY_NORMAL, bool bIsMerge = false);

    //奖励物品 合并
    int RewardItemByMerge(PlayerData* pstData, uint8_t bItemType, uint32_t dwId, int32_t iNum,
        DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach, uint8_t bShowProperty);

    //Item消耗
    int ConsumeItem(PlayerData* pstData, uint8_t bItemType, uint32_t dwId, int32_t iNum,
        DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach);

    //调用此接口必须保证武将可以添加
    //  如无必要,请使用RewardItem和ConsumeItem
    int AddItem(PlayerData* pstData, uint8_t bItemType, uint32_t dwId, int32_t iNum, DT_ITEM& rstItem, uint32_t dwApproach);




};

