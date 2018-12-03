#pragma once
#include "GeneralCard.h"

class GeneralReborn : public TSingleton<GeneralReborn>
{
public:
    GeneralReborn() {};
    virtual ~GeneralReborn() {};
    bool Init();
    //完整重生
    int Reborn(uint32_t dwId, uint8_t bType, DT_SYNC_ITEM_INFO* pstSyncItemInfo, PlayerData* pstData);
    //装备降星
    int EquipStarDown(uint32_t dwGeneralId, uint8_t bEquipType, DT_SYNC_ITEM_INFO* pstSyncItemInfo, PlayerData* pstData);
    //装备星级重生
    int EquipStarReborn(uint32_t dwGeneralId, PlayerData* pstData, DT_SYNC_ITEM_INFO* pstSyncItemInfo);

private:
    //武将重生
    int _GeneralReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //装备重生
    int _EquipReborn(uint32_t dwRatio, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, map<uint8_t, uint32_t> *pToken);
    //兵种重生
    int _ArmyReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //等级重生
    int _LvReborn(uint32_t dwRatio, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //星级重生，目前的重生规则中不进行星级重生
    int _StarReborn(uint32_t* pGold, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //品阶重生
    int _PhaseReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //技能点重生
    int _SkillReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //兵种等级重生
    int _ArmyLvReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //兵种品阶重生
    int _ArmyPhaseReborn(uint32_t dwRatio, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //装备品阶重生
    int _EquipPhaseReborn(uint32_t dwRatio, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, map<uint8_t, uint32_t> *pToken);
    //装备等级重生
    int _EquipLvReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, PlayerData * pstData, map<uint32_t, uint32_t> * pTmpPropReturn, DT_ITEM_GCARD * pstGeneral);
    //重置装备属性
    void _ResetGeneralDataAfterReborn(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);

private:
    //武将经验卡ID
    uint32_t m_GeneralExpCardId[MAX_NUM_GENERAL_EXPCARD_TYPE];
    //武将经验卡提供的经验
    uint32_t m_GeneralExpCardExp[MAX_NUM_GENERAL_EXPCARD_TYPE];
    //饰品经验卡ID
    uint32_t m_BookExpCardId[MAX_NUM_EQUIP_EXPCARD_TYPE];
    //饰品经验卡提供的经验
    uint32_t m_BookExpCardExp[MAX_NUM_EQUIP_EXPCARD_TYPE];
    //坐骑经验卡ID
    uint32_t m_MountExpCardId[MAX_NUM_EQUIP_EXPCARD_TYPE];
    //坐骑经验卡提供的经验
    uint32_t m_MountExpCardExp[MAX_NUM_EQUIP_EXPCARD_TYPE];

    static const int BOOK_EXP_ITEM_ID;//饰品经验物品在basic表中的id
    static const int MOUNT_EXP_ITEM_ID;//坐骑经验物品在basic表中的id
};
