#pragma once
#include "GeneralCard.h"

class GeneralReborn : public TSingleton<GeneralReborn>
{
public:
    GeneralReborn() {};
    virtual ~GeneralReborn() {};
    bool Init();
    //��������
    int Reborn(uint32_t dwId, uint8_t bType, DT_SYNC_ITEM_INFO* pstSyncItemInfo, PlayerData* pstData);
    //װ������
    int EquipStarDown(uint32_t dwGeneralId, uint8_t bEquipType, DT_SYNC_ITEM_INFO* pstSyncItemInfo, PlayerData* pstData);
    //װ���Ǽ�����
    int EquipStarReborn(uint32_t dwGeneralId, PlayerData* pstData, DT_SYNC_ITEM_INFO* pstSyncItemInfo);

private:
    //�佫����
    int _GeneralReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //װ������
    int _EquipReborn(uint32_t dwRatio, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, map<uint8_t, uint32_t> *pToken);
    //��������
    int _ArmyReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //�ȼ�����
    int _LvReborn(uint32_t dwRatio, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //�Ǽ�������Ŀǰ�����������в������Ǽ�����
    int _StarReborn(uint32_t* pGold, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //Ʒ������
    int _PhaseReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //���ܵ�����
    int _SkillReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //���ֵȼ�����
    int _ArmyLvReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //����Ʒ������
    int _ArmyPhaseReborn(uint32_t dwRatio, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //װ��Ʒ������
    int _EquipPhaseReborn(uint32_t dwRatio, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, map<uint8_t, uint32_t> *pToken);
    //װ���ȼ�����
    int _EquipLvReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, PlayerData * pstData, map<uint32_t, uint32_t> * pTmpPropReturn, DT_ITEM_GCARD * pstGeneral);
    //����װ������
    void _ResetGeneralDataAfterReborn(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);

private:
    //�佫���鿨ID
    uint32_t m_GeneralExpCardId[MAX_NUM_GENERAL_EXPCARD_TYPE];
    //�佫���鿨�ṩ�ľ���
    uint32_t m_GeneralExpCardExp[MAX_NUM_GENERAL_EXPCARD_TYPE];
    //��Ʒ���鿨ID
    uint32_t m_BookExpCardId[MAX_NUM_EQUIP_EXPCARD_TYPE];
    //��Ʒ���鿨�ṩ�ľ���
    uint32_t m_BookExpCardExp[MAX_NUM_EQUIP_EXPCARD_TYPE];
    //���ﾭ�鿨ID
    uint32_t m_MountExpCardId[MAX_NUM_EQUIP_EXPCARD_TYPE];
    //���ﾭ�鿨�ṩ�ľ���
    uint32_t m_MountExpCardExp[MAX_NUM_EQUIP_EXPCARD_TYPE];

    static const int BOOK_EXP_ITEM_ID;//��Ʒ������Ʒ��basic���е�id
    static const int MOUNT_EXP_ITEM_ID;//���ﾭ����Ʒ��basic���е�id
};
