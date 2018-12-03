#include <vector>
#include "GeneralReborn.h"
#include "../gamedata/GameDataMgr.h"
#include "ZoneLog.h"
#include "Item.h"
#include "../../../common/protocol/DWLOG/dwlog_def.h"
#include "Equip.h"
#include "../../../common/log/LogMacros.h"
#include "Consume.h"

using namespace DWLOG;

const int GeneralReborn::BOOK_EXP_ITEM_ID = 1901;
const int GeneralReborn::MOUNT_EXP_ITEM_ID = 1902;

bool GeneralReborn::Init()
{
    ResBasicMgr_t& rstResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* pstResBasic = rstResBasicMgr.Find(GENERAL_LV_UP_EXPERIENCE);
    if (pstResBasic == NULL)
    {
        LOGERR("Init GeneralCard failed, pstResbasic is null");
        return false;
    }

    ResPropsMgr_t& rstResPropsMgr = CGameDataMgr::Instance().GetResPropsMgr();
    for (int i = 0; i < MAX_NUM_GENERAL_EXPCARD_TYPE && i < 6; i++)
    {
        m_GeneralExpCardId[i] = (uint32_t)pstResBasic->m_para[i];
        RESPROPS* pResprops = rstResPropsMgr.Find(m_GeneralExpCardId[i]);
        if (pResprops == NULL)
        {
            LOGERR("Init GeneralCard failed, pResprops is null");
            return false;
        }
        m_GeneralExpCardExp[i] = pResprops->m_dwExp;
    }
    pstResBasic = rstResBasicMgr.Find(BOOK_EXP_ITEM_ID);
    if (pstResBasic == NULL)
    {
        LOGERR("Init GeneralCard failed, pstResBasic is null");
        return false;
    }
    for (int i = 0; i < MAX_NUM_EQUIP_EXPCARD_TYPE; i++)
    {
        m_BookExpCardId[i] = (uint32_t)pstResBasic->m_para[i];
        RESPROPS* pResprops = rstResPropsMgr.Find(m_BookExpCardId[i]);
        if (pResprops == NULL)
        {
            LOGERR("Init equip failed, pResprops is null");
            return false;
        }
        m_BookExpCardExp[i] = pResprops->m_dwExpEquip;
    }
    pstResBasic = rstResBasicMgr.Find(MOUNT_EXP_ITEM_ID);
    if (pstResBasic == NULL)
    {
        LOGERR("Init GeneralReborn failed, pstResBasic is null");
        return false;
    }
    for (int i = 0; i < MAX_NUM_EQUIP_EXPCARD_TYPE; i++)
    {
        m_MountExpCardId[i] = (uint32_t)pstResBasic->m_para[i];
        RESPROPS* pResprops = rstResPropsMgr.Find(m_MountExpCardId[i]);
        if (pResprops == NULL)
        {
            LOGERR("Init equip failed, pResprops is null");
            return false;
        }
        m_MountExpCardExp[i] = pResprops->m_dwExpEquip;
    }

    return true;
}

int GeneralReborn::Reborn(uint32_t dwId, uint8_t bType, DT_SYNC_ITEM_INFO * pstSyncItemInfo, PlayerData * pstData)
{
    DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(pstData, dwId);
    if (!pstGeneral)
    {
        LOGERR("Player(%lu) Reborn failed, General(%u) not found", pstData->m_ullUin, dwId);
        return ERR_NOT_FOUND;
    }

    map<uint32_t, uint32_t> tmpPropReturn;
    map<uint8_t, uint32_t> tempToken;

    int iRet = ERR_NONE;
    RESBASIC* pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(REBORN_RETURN_RATIO);
    if (pResBasic == NULL)
    {
        iRet = ERR_NOT_FOUND;
        return iRet;
    }
    uint32_t dwRatio = (bType == 1) ? pResBasic->m_para[0] : pResBasic->m_para[1];
    //uint32_t dwSumGold = 0;

    _GeneralReborn(dwRatio, &tempToken, &tmpPropReturn, pstData, pstGeneral);
    _EquipReborn(dwRatio, &tmpPropReturn, pstData, pstGeneral, &tempToken);
    _ArmyReborn(dwRatio, &tempToken, &tmpPropReturn, pstData, pstGeneral);

    //如果是完美重生，则需要根据物品计算需要消耗的钻石
    if (bType == 1)
    {
        float fDiamond = 0;
        //计算完美重生物品所需要消耗的钻石
        ResPropsToDiamondMgr_t& rstResPropsToDiamondMgr = CGameDataMgr::Instance().GetResPropsToDiamondMgr();
        RESPROPSTODIAMOND* pResPropsToDiamond = NULL;
        for (map<uint32_t, uint32_t>::iterator iter = tmpPropReturn.begin(); iter != tmpPropReturn.end(); iter++)
        {
            pResPropsToDiamond = rstResPropsToDiamondMgr.Find(iter->first);
            if (pResPropsToDiamond != NULL)
            {
                fDiamond += pResPropsToDiamond->m_fDiamongValue * iter->second;
            }
        }
        //计算完美重生代币所消耗的钻石
        ResTokenToDiamondMgr_t& rstResTokenToDiamondMgr = CGameDataMgr::Instance().GetResTokenToDiamondMgr();
        RESTOKENTODIAMOND* pResTokenToDiamond = NULL;
        for (map<uint8_t, uint32_t>::iterator iter = tempToken.begin(); iter != tempToken.end(); iter++)
        {
            pResTokenToDiamond = rstResTokenToDiamondMgr.Find(iter->first);
            if (pResTokenToDiamond != NULL)
            {
                fDiamond += pResTokenToDiamond->m_fDiamondValue * iter->second;
            }
        }
        /*pResTokenToDiamond = rstResTokenToDiamondMgr.Find(ITEM_TYPE_GOLD);
        if (pResTokenToDiamond != NULL)
        {
            fDiamond += pResTokenToDiamond->m_fDiamondValue * static_cast<float>(tempToken[ITEM_TYPE_GOLD]);
        }*/
        if (Consume::Instance().IsEnoughDiamond(pstData, static_cast<int>(fDiamond)) == false)
        {
            LOGERR("Player(%s) Uin(%lu) Reborn failed, Diamond num(%d) is not enough", pstData->GetRoleBaseInfo().m_szRoleName,
                pstData->m_ullUin, static_cast<int>(fDiamond));
            return ERR_NOT_ENOUGH_DIAMOND;
        }
        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -(static_cast<int>(fDiamond)), *pstSyncItemInfo, METHOD_GENERAL_OP_REBORN);
    }

    //重置玩家属性
    _ResetGeneralDataAfterReborn(pstData, pstGeneral);

    for (map<uint32_t, uint32_t>::iterator iter = tmpPropReturn.begin(); iter != tmpPropReturn.end(); iter++)
    {
        if (iter->second != 0)
        {
            Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, iter->first, iter->second, *pstSyncItemInfo, METHOD_GENERAL_OP_REBORN);
        }
    }
    for (map<uint8_t, uint32_t>::iterator iter = tempToken.begin(); iter != tempToken.end(); iter++)
    {
        if (iter->second != 0)
        {
            Item::Instance().RewardItem(pstData, iter->first, 0, iter->second, *pstSyncItemInfo, METHOD_GENERAL_OP_REBORN);
        }
    }
    GeneralCard::Instance().CalGcardLi(pstData, dwId);

    //武将重生日志
    ZoneLog::Instance().WriteRebornLog(pstData->m_pOwner, dwId, pstGeneral->m_bLevel, bType);

    return iRet;
}

int GeneralReborn::EquipStarDown(uint32_t dwGeneralId, uint8_t bEquipType, DT_SYNC_ITEM_INFO * pstSyncItemInfo, PlayerData * pstData)
{
    DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(pstData, dwGeneralId);
    if (!pstGeneral)
    {
        LOGERR("Player(%lu) Reborn failed, General(%u) not found", pstData->m_ullUin, dwGeneralId);
        return ERR_NOT_FOUND;
    }

    uint32_t dwEquipSeq = pstGeneral->m_astEquipList[bEquipType - 1].m_dwEquipSeq;
    DT_ITEM_EQUIP *pstEquip = Equip::Instance().Find(pstData, dwEquipSeq);
    if (pstEquip == NULL)
    {
        LOGERR("Player(%s) Uin(%lu) Equip star down failed, Equip seq(%u) not found", pstData->GetRoleBaseInfo().m_szRoleName,
            pstData->m_ullUin, dwEquipSeq);
        return ERR_NOT_FOUND;
    }

    if (pstEquip->m_wStar == 0)
    {
        return ERR_DEFAULT;
    }

    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    RESGENERAL* pResGeneral = rstResGeneralMgr.Find(pstGeneral->m_dwId);
    assert(pResGeneral);

    uint16_t wGeneralEquipStarId = pResGeneral->m_equipStarList[bEquipType - 1];
    ResGeneralEquipStarMgr_t& rstResGeneralEquipStarMgr = CGameDataMgr::Instance().GetResGeneralEquipStarMgr();
    RESGENERALEQUIPSTAR* pResGeneralEquipStar = rstResGeneralEquipStarMgr.Find(wGeneralEquipStarId);
    if (pResGeneralEquipStar == NULL)
    {
        LOGERR("Player(%s) Uin(%lu) Equip star down failed, ResGeneralEquipStar(%u) not found", pstData->GetRoleBaseInfo().m_szRoleName,
            pstData->m_ullUin, wGeneralEquipStarId);
        return ERR_NOT_FOUND;
    }

    uint16_t wEquipCurStar = pstEquip->m_wStar;
    ResEquipStarMgr_t& rstResEquipStarMgr = CGameDataMgr::Instance().GetResEquipStarMgr();
    RESEQUIPSTAR* pResEquipStar = rstResEquipStarMgr.Find(pResGeneralEquipStar->m_starList[wEquipCurStar]);
    if (pResEquipStar == NULL)
    {
        LOGERR("Player(%s) Uin(%lu) Equip star down failed, ResEquipStar(%u) not found", pstData->GetRoleBaseInfo().m_szRoleName,
            pstData->m_ullUin, pResGeneralEquipStar->m_starList[wEquipCurStar]);
        return ERR_NOT_FOUND;
    }

    //将需要返还的物品增加到TempProps中
    map<uint32_t, uint32_t> TempProps;
    for (size_t i = 0; i < pResEquipStar->m_wMeterials; i++)
    {
        TempProps[pResEquipStar->m_meterialId[i]] += pResEquipStar->m_meterialNum[i];
        /*Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, pResEquipStar->m_meterialId[i],
            pResEquipStar->m_meterialNum[i], *pstSyncItemInfo, METHOD_GENERAL_OP_REBORN);*/
    }
    ResPropsToDiamondMgr_t& rstResPropsToDiamondMgr = CGameDataMgr::Instance().GetResPropsToDiamondMgr();
    RESPROPSTODIAMOND* pResPropsToDiamond = NULL;
    //根据TempProps中的物品计算需要消耗的钻石
    float fDiamond = 0;
    for (map<uint32_t, uint32_t>::iterator iter = TempProps.begin(); iter != TempProps.end(); ++iter)
    {
        pResPropsToDiamond = rstResPropsToDiamondMgr.Find(iter->first);
        if (pResPropsToDiamond != NULL)
        {
            fDiamond = pResPropsToDiamond->m_fDiamongValue * iter->second;
        }
    }

    //判断钻石是否足够
    if (Consume::Instance().IsEnoughDiamond(pstData, static_cast<int>(fDiamond)) == false)
    {
        LOGERR("Player(%s) Uin(%lu) Reborn failed, Diamond num(%d) is not enough", pstData->GetRoleBaseInfo().m_szRoleName,
            pstData->m_ullUin, static_cast<int>(fDiamond));
        return ERR_NOT_ENOUGH_DIAMOND;
    }

    //返还物品，消耗钻石
    for (map<uint32_t, uint32_t>::iterator iter = TempProps.begin(); iter != TempProps.end(); ++iter)
    {
        Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, iter->first, iter->second, *pstSyncItemInfo, METHOD_GENERAL_OP_REBORN);
    }
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -(static_cast<int>(fDiamond)), *pstSyncItemInfo, METHOD_GENERAL_OP_REBORN);

    //修改相关属性
    --pstEquip->m_wStar;
    GeneralCard::Instance().CalGcardLi(pstData, dwGeneralId);

    return 0;
}

int GeneralReborn::_GeneralReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral)
{
    _LvReborn(dwRatio, pTmpPropReturn, pstData, pstGeneral);
    _SkillReborn(dwRatio, pToken, pstData, pstGeneral);
    _PhaseReborn(dwRatio, pToken, pTmpPropReturn, pstData, pstGeneral);
    return 0;
}

int GeneralReborn::_EquipReborn(uint32_t dwRatio, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, map<uint8_t, uint32_t> *pToken)
{
    _EquipPhaseReborn(dwRatio, pTmpPropReturn, pstData, pstGeneral, pToken);
    _EquipLvReborn(dwRatio, pToken, pstData, pTmpPropReturn, pstGeneral);
    return 0;
}

int GeneralReborn::_ArmyReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral)
{
    _ArmyLvReborn(dwRatio, pToken, pTmpPropReturn, pstData, pstGeneral);
    _ArmyPhaseReborn(dwRatio, pTmpPropReturn, pstData, pstGeneral);
    return 0;
}

int GeneralReborn::_LvReborn(uint32_t dwRatio, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData * pstData, DT_ITEM_GCARD * pstGeneral)
{
    uint32_t dwExp = static_cast<uint64_t>(pstGeneral->m_dwExp) * dwRatio / 100;

    for (int i = MAX_NUM_GENERAL_EXPCARD_TYPE; i > 0; i--)
    {
        if (dwExp == 0)
        {
            break;
        }
        uint32_t dwNum = dwExp / m_GeneralExpCardExp[i - 1];
        if (dwNum > 0)
        {
            (*pTmpPropReturn)[m_GeneralExpCardId[i - 1]] += dwNum;
            //Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, m_GeneralExpCardId[i], dwNum, *pstRewardItemInfo, METHOD_GENERAL_OP_REBORN);
        }
        dwExp -= dwNum * m_GeneralExpCardExp[i - 1];
    }

    return 0;
}

int GeneralReborn::_StarReborn(uint32_t * pGold, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData * pstData, DT_ITEM_GCARD * pstGeneral)
{
    RESGENERAL* pResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pstGeneral->m_dwId);
    assert(pResGeneral);

    ResConsumeMgr_t& rstResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();
    RESCONSUME* pResPropsConsume = rstResConsumeMgr.Find(pResGeneral->m_dwUpStarConsume);
    RESCONSUME* pResGoldConsume = rstResConsumeMgr.Find(pResGeneral->m_dwGoldgeneralstaruse);
    assert(pResPropsConsume);
    assert(pResGoldConsume);

    uint32_t dwGold = 0;
    uint32_t dwNum = 0;
    for (int i = pstGeneral->m_bStar; i > 1; i--)
    {
        dwGold += pResGoldConsume->m_lvList[i - 1];
        dwNum += pResPropsConsume->m_lvList[i - 1];
    }

    *pGold += dwGold;
    (*pTmpPropReturn)[pResGeneral->m_dwUpStarProp] += dwNum;
    //Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, pResGeneral->m_dwUpStarProp, dwNum, *pstRewardItemInfo, METHOD_GENERAL_OP_REBORN);

    return ERR_NONE;
}

int GeneralReborn::_PhaseReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData * pstData, DT_ITEM_GCARD * pstGeneral)
{
    ResGeneralPhaseMgr_t& roResGenrPhaseMgr = CGameDataMgr::Instance().GetResGeneralPhaseMgr();
    RESGENERALPHASE * pResPhase = roResGenrPhaseMgr.Find(pstGeneral->m_dwId);
    if (pResPhase == NULL)
    {
        LOGERR("pResPhase is null");
        return ERR_NOT_FOUND;
    }
    assert(pstGeneral->m_bPhase > 0);

    ResGeneralPhasePropsMgr_t& roResGenrPhasePropsMgr = CGameDataMgr::Instance().GetResGeneralPhasePropsMgr();

    uint32_t dwGold = 0;

    map<uint32_t, uint32_t> Tmp;

    for (int i = pstGeneral->m_bPhase - 1; i > 0; --i)
    {
        uint32_t dwPhasePropsID = pResPhase->m_phaseUpProps[i - 1];
        RESGENERALPHASEPROPS* poResGenrPhaseProps = roResGenrPhasePropsMgr.Find(dwPhasePropsID);
        if (poResGenrPhaseProps == NULL)
        {
            LOGERR("pResGenrPhaseProps is null, id: %u", dwPhasePropsID);
            return ERR_NOT_FOUND;
        }
        dwGold += poResGenrPhaseProps->m_dwGoldenNum;
        for (size_t j = 0; j < poResGenrPhaseProps->m_dwMeterialsNum; j++)
        {
            uint32_t dwNum = poResGenrPhaseProps->m_propNum[j];
            if (dwNum != 0)
            {
                Tmp[poResGenrPhaseProps->m_propId[j]] += dwNum;
                //Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, poResGenrPhaseProps->m_propId[j], dwNum, *pstRewardItemInfo, METHOD_GENERAL_OP_REBORN);
            }
        }
    }

    //*pGold += static_cast<uint64_t>(dwGold) * dwRatio / 100;
    (*pToken)[ITEM_TYPE_GOLD] += static_cast<uint64_t>(dwGold) * dwRatio / 100;

    for (map<uint32_t, uint32_t>::iterator iter = Tmp.begin(); iter != Tmp.end(); iter++)
    {
        if (iter->second > 1)
        {
            (*pTmpPropReturn)[iter->first] += iter->second * dwRatio / 100;
        }
        else if (iter->second == 1)
        {
            (*pTmpPropReturn)[iter->first] += iter->second;
        }
    }

    return ERR_NONE;
}

int GeneralReborn::_SkillReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, PlayerData * pstData, DT_ITEM_GCARD * pstGeneral)
{
    uint8_t bDstSkillLevel = 0;

    RESGENERAL* pResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pstGeneral->m_dwId);
    assert(pResGeneral);

    RESGENERALSTAR* pResGeneralStar = CGameDataMgr::Instance().GetResGeneralStarMgr().Find(pstGeneral->m_dwId);
    assert(pResGeneralStar);

    ResConsumeMgr_t& rstResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();

    uint32_t dwGoldConsume = 0;
    for (size_t i = 0; i < MAX_NUM_GENERAL_SKILL; i++)
    {
        bDstSkillLevel = pstGeneral->m_szSkillLevel[i];
        RESCONSUME* pResGoldConsume = rstResConsumeMgr.Find(pResGeneralStar->m_goldConsume[i]);
        assert(pResGoldConsume);
        for (size_t j = 0; j < bDstSkillLevel; j++)
        {
            if (pResGoldConsume != NULL)
            {
                dwGoldConsume += pResGoldConsume->m_lvList[j];
            }
        }
    }
    //*pGold += static_cast<uint64_t>(dwGoldConsume) * dwRatio / 100;
    (*pToken)[ITEM_TYPE_GOLD] += static_cast<uint64_t>(dwGoldConsume) * dwRatio / 100;

    return ERR_NONE;
}

int GeneralReborn::_ArmyLvReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData* pstData, DT_ITEM_GCARD* pstGeneral)
{
    RESGENERAL* pResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pstGeneral->m_dwId);
    assert(pResGeneral);

    //金币消耗数据档
    ResConsumeMgr_t& rstResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();
    RESCONSUME* pResGoldConsume = rstResConsumeMgr.Find(pResGeneral->m_dwArmyUpGold);
    assert(pResGoldConsume);

    //返还金币
    uint32_t dwGold = 0;
    for (int j = pstGeneral->m_bArmyLv; j > 1; j--)
    {
        dwGold += pResGoldConsume->m_lvList[j - 1];
    }
    //*pGold += static_cast<uint64_t>(dwGold) * dwRatio / 100;
    (*pToken)[ITEM_TYPE_GOLD] += static_cast<uint64_t>(dwGold) * dwRatio / 100;

    map<uint32_t, uint32_t> Tmp;

    //返还道具
    for (uint32_t i = 0; i < 3; i++)
    {
        //道具消耗数据档
        RESCONSUME* pResPropsConsume = rstResConsumeMgr.Find(pResGeneral->m_armyUpMaterialNums[i]);
        assert(pResPropsConsume);

        //计算数量
        uint32_t dwNum = 0;
        for (uint32_t j = pstGeneral->m_bArmyLv; j > 1; j--)
        {
            dwNum += pResPropsConsume->m_lvList[j - 1];
        }
        if (dwNum > 0)
        {
            Tmp[pResGeneral->m_armyUpMaterialIDs[i]] += dwNum;
            //Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, pResGeneral->m_armyUpMaterialIDs[i], dwNum, *pstRewardItemInfo, METHOD_GENERAL_OP_REBORN);
        }
    }

    for (map<uint32_t, uint32_t>::iterator iter = Tmp.begin(); iter != Tmp.end(); iter++)
    {
        if (iter->second > 1)
        {
            (*pTmpPropReturn)[iter->first] += iter->second * dwRatio / 100;
        }
        else if (iter->second == 1)
        {
            (*pTmpPropReturn)[iter->first] += iter->second;
        }
    }

    return ERR_NONE;
}

int GeneralReborn::_ArmyPhaseReborn(uint32_t dwRatio, map<uint32_t, uint32_t> *pTmpPropReturn, PlayerData * pstData, DT_ITEM_GCARD * pstGeneral)
{
    RESGENERAL* pResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pstGeneral->m_dwId);
    assert(pResGeneral);

    map<uint32_t, uint32_t> Tmp;

    for (int i = pstGeneral->m_bArmyPhase - 1; i > 0; i--)
    {
        uint32_t dwNum = pResGeneral->m_armyGradeNums[i - 1];
        if (dwNum > 0)
        {
            Tmp[pResGeneral->m_armyGradeMaterialIDs[i - 1]] += dwNum;
            //Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, pResGeneral->m_armyGradeMaterialIDs[i - 1], dwNum, *pstRewardItemInfo, METHOD_GENERAL_OP_REBORN);
        }
    }

    for (map<uint32_t, uint32_t>::iterator iter = Tmp.begin(); iter != Tmp.end(); iter++)
    {
        if (iter->second > 1)
        {
            (*pTmpPropReturn)[iter->first] += iter->second * dwRatio / 100;
        }
        else if (iter->second == 1)
        {
            (*pTmpPropReturn)[iter->first] += iter->second;
        }
    }

    return ERR_NONE;
}

int GeneralReborn::_EquipPhaseReborn(uint32_t dwRatio, map<uint32_t, uint32_t> * pTmpPropReturn, PlayerData * pstData, DT_ITEM_GCARD * pstGeneral, map<uint8_t, uint32_t> *pToken)
{
    map<uint32_t, uint32_t> Tmp;
    uint32_t dwEquipSeq = 0;
    uint8_t bCurEquipPhase = 0;
    ResEquipMgr_t& rstResEquipMgr = CGameDataMgr::Instance().GetResEquipMgr();
    ResEquipsListMgr_t& rstResEquipsListMgr = CGameDataMgr::Instance().GetResEquipsListMgr();
    RESEQUIPSLIST* pResEquipsList = NULL;
    RESEQUIP* pstCurEquip = NULL;
    DT_ITEM_EQUIP* pstEquip = NULL;
    //DT_ITEM stTmpItem;
    //遍历所有装备
    for (size_t i = 0; i < MAX_EQUIP_TYPE_NUM; ++i)
    {
        dwEquipSeq = pstGeneral->m_astEquipList[i].m_dwEquipSeq;
        pstEquip = Equip::Instance().Find(pstData, dwEquipSeq);
        if (pstEquip == NULL)
        {
            LOGERR("Player(%s) Uin(%lu) Equip phase Reborn failed, Equip seq(%u) not found", pstData->GetRoleBaseInfo().m_szRoleName,
                pstData->m_ullUin, dwEquipSeq);
            return ERR_NOT_FOUND;
        }
        bCurEquipPhase = pstEquip->m_bPhase;
        pResEquipsList = rstResEquipsListMgr.Find(pstEquip->m_dwEquipListId);
        if (pResEquipsList == NULL)
        {
            LOGERR("Player(%s) Uin(%lu) Equip phase reborn failed, Equip list(%u) not found", pstData->GetRoleBaseInfo().m_szRoleName,
                pstData->m_ullUin, pstEquip->m_dwEquipListId);
            return ERR_NOT_FOUND;
        }
        //遍历当前装备的品阶
        for (int j = bCurEquipPhase - 1; j > 0; --j)
        {
            pstCurEquip = rstResEquipMgr.Find(pResEquipsList->m_grade[j]);
            if (pstCurEquip == NULL)
            {
                LOGERR("Player(%s) Uin(%lu) Equip reborn failed, Equip id(%u) not found", pstData->GetRoleBaseInfo().m_szRoleName,
                    pstData->m_ullUin, pstCurEquip->m_dwEquipId);
                return ERR_NOT_FOUND;
            }
            //遍历当前品阶的材料
            for (int k = 0; k < pstCurEquip->m_wMeterials; k++)
            {
                if (pstCurEquip->m_szMeterialType[k] == ITEM_TYPE_PROPS)
                {
                    Tmp[pstCurEquip->m_meterialId[k]] += pstCurEquip->m_meterialNum[k];
                }
                else
                {
                    (*pToken)[pstCurEquip->m_szMeterialType[k]] += static_cast<uint64_t>(pstCurEquip->m_meterialNum[k]) * dwRatio / 100;
                }
            }
            (*pToken)[ITEM_TYPE_GOLD] += static_cast<uint64_t>(pstCurEquip->m_dwGoldConsume) * dwRatio / 100;
        }
    }

    for (map<uint32_t, uint32_t>::iterator iter = Tmp.begin(); iter != Tmp.end(); iter++)
    {
        if (iter->second > 1)
        {
            (*pTmpPropReturn)[iter->first] += iter->second * dwRatio / 100;
        }
        else if (iter->second == 1)
        {
            (*pTmpPropReturn)[iter->first] += iter->second;
        }
    }

    return ERR_NONE;
}

int GeneralReborn::EquipStarReborn(uint32_t dwGeneralId, PlayerData * pstData, DT_SYNC_ITEM_INFO* pstSyncItemInfo)
{
    DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(pstData, dwGeneralId);
    if (!pstGeneral)
    {
        LOGERR("Player(%lu) Reborn failed, General(%u) not found", pstData->m_ullUin, dwGeneralId);
        return ERR_NOT_FOUND;
    }

    DT_ITEM_EQUIP* pstEquip = NULL;
    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    RESGENERAL* pResGeneral = rstResGeneralMgr.Find(pstGeneral->m_dwId);
    assert(pResGeneral);
    ResEquipStarMgr_t& rstResEquipStarMgr = CGameDataMgr::Instance().GetResEquipStarMgr();
    RESEQUIPSTAR* pResEquipStar = NULL;
    ResGeneralEquipStarMgr_t& rstResGeneralEquipStarMgr = CGameDataMgr::Instance().GetResGeneralEquipStarMgr();
    RESGENERALEQUIPSTAR* pResGeneralEquipStar = NULL;
    uint16_t wGeneralEquipStarId = 0;
    uint32_t dwEquipSeq = 0;
    map<uint32_t, uint32_t> tmpPropReturn;
    //遍历所有装备
    for (int i = 0; i < MAX_EQUIP_TYPE_NUM; ++i)
    {
        dwEquipSeq = pstGeneral->m_astEquipList[i].m_dwEquipSeq;
        wGeneralEquipStarId = pResGeneral->m_equipStarList[i];
        pstEquip = Equip::Instance().Find(pstData, dwEquipSeq);
        if (pstEquip == NULL)
        {
            LOGERR("Player(%s) Uin(%lu) Equip star reborn failed, Equip seq(%u) not found", pstData->GetRoleBaseInfo().m_szRoleName,
                pstData->m_ullUin, dwEquipSeq);
            return ERR_NOT_FOUND;
        }
        pResGeneralEquipStar = rstResGeneralEquipStarMgr.Find(wGeneralEquipStarId);
        assert(pResGeneralEquipStar);
        uint16_t wEquipStar = pstEquip->m_wStar;
        //遍历当前装备的当前星级
        for (int j = wEquipStar; j > 0; --j)
        {
            pResEquipStar = rstResEquipStarMgr.Find(pResGeneralEquipStar->m_starList[j]);
            assert(pResEquipStar);
            //遍历升到当前星级所需的所有材料
            for (int k = 0; k < pResEquipStar->m_wMeterials; k++)
            {
                tmpPropReturn[pResEquipStar->m_meterialId[k]] += pResEquipStar->m_meterialNum[k];
                /*Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, pResEquipStar->m_meterialId[k],
                    pResEquipStar->m_meterialNum[k], *pstRewardItemInfo, METHOD_GENERAL_OP_REBORN);*/
            }
            --pstEquip->m_wStar;
        }
    }

    float fDiamond = 0;
    //计算完美重生物品所需要消耗的钻石
    ResPropsToDiamondMgr_t& rstResPropsToDiamondMgr = CGameDataMgr::Instance().GetResPropsToDiamondMgr();
    RESPROPSTODIAMOND* pResPropsToDiamond = NULL;
    for (map<uint32_t, uint32_t>::iterator iter = tmpPropReturn.begin(); iter != tmpPropReturn.end(); iter++)
    {
        pResPropsToDiamond = rstResPropsToDiamondMgr.Find(iter->first);
        if (pResPropsToDiamond != NULL)
        {
            fDiamond += pResPropsToDiamond->m_fDiamongValue * iter->second;
        }
    }
    if (Consume::Instance().IsEnoughDiamond(pstData, static_cast<int>(fDiamond)) == false)
    {
        return ERR_NOT_ENOUGH_DIAMOND;
    }

    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -static_cast<int>(fDiamond), *pstSyncItemInfo, METHOD_GENERAL_OP_REBORN);

    //返还道具
    for (map<uint32_t, uint32_t>::iterator iter = tmpPropReturn.begin(); iter != tmpPropReturn.end(); iter++)
    {
        Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, iter->first, iter->second,
            *pstSyncItemInfo, METHOD_GENERAL_OP_REBORN);
    }
    return ERR_NONE;
}

int GeneralReborn::_EquipLvReborn(uint32_t dwRatio, map<uint8_t, uint32_t> *pToken, PlayerData * pstData, map<uint32_t, uint32_t> * pTmpPropReturn, DT_ITEM_GCARD * pstGeneral)
{
    ;
    DT_ITEM_EQUIP* pstEquip = NULL;
    uint32_t dwEquipSeq = 0;
    uint32_t dwExp = 0;
    uint32_t dwNum = 0;
    //ResEquipMgr_t& rstResEquipMgr = CGameDataMgr::Instance().GetResEquipMgr();
    //RESEQUIP* pstCurEquip = NULL;
    uint32_t dwExpToGold = 0;
    //uint32_t dwGoldConsume = 0;
    for (int i = 0; i < MAX_EQUIP_TYPE_NUM; i++)
    {
        dwEquipSeq = pstGeneral->m_astEquipList[i].m_dwEquipSeq;
        pstEquip = Equip::Instance().Find(pstData, dwEquipSeq);
        if (pstEquip == NULL)
        {
            LOGERR("Equip not found");
            return ERR_NOT_FOUND;
        }
        dwExp = static_cast<uint64_t>(pstEquip->m_dwExp) * dwRatio / 100;
        for (int j = MAX_NUM_EQUIP_EXPCARD_TYPE; j > 0; --j)
        {
            if (0 == dwExp)
            {
                break;
            }
            if (pstEquip->m_bType == EQUIP_TYPE_BOOK)
            {
                dwNum = dwExp / m_BookExpCardExp[j - 1];
                if (dwNum > 0)
                {
                    (*pTmpPropReturn)[m_BookExpCardId[j - 1]] += dwNum;
                    /*Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, m_EquipExpCardId[j],
                    dwNum, *pstRewardItemInfo, METHOD_GENERAL_OP_REBORN);*/
                }
            }
            else if (pstEquip->m_bType == EQUIP_TYPE_MOUNT)
            {
                dwNum = dwExp / m_MountExpCardExp[j - 1];
                if (dwNum > 0)
                {
                    (*pTmpPropReturn)[m_MountExpCardId[j - 1]] += dwNum;
                    /*Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, m_EquipExpCardId[j],
                    dwNum, *pstRewardItemInfo, METHOD_GENERAL_OP_REBORN);*/
                }
            }
            dwExp %= m_BookExpCardExp[j - 1];
        }
        //金币返还
        RESBASIC* pstResExpToGold = CGameDataMgr::Instance().GetResBasicMgr().Find(EQUIP_EXP_TO_GOLD);
        assert(pstResExpToGold);
        if (pstResExpToGold == NULL)
        {
            LOGERR("Player(%s) Uin(%lu) Equip Lv Reborn failed, basic id(%d) not found", pstData->GetRoleBaseInfo().m_szRoleName,
                pstData->m_ullUin, EQUIP_EXP_TO_GOLD);
            return false;
        }
        if (pstEquip->m_bType == EQUIP_TYPE_MOUNT || pstEquip->m_bType == EQUIP_TYPE_BOOK)
        {
            dwExpToGold = (uint32_t)pstResExpToGold->m_para[0];
        }
        else
        {
            dwExpToGold = 1;
        }
        //*pGold += dwGoldConsume;
        (*pToken)[ITEM_TYPE_GOLD] += static_cast<uint64_t>(pstEquip->m_dwExp) * dwExpToGold * dwRatio / 100;
    }

    return 0;
}

void GeneralReborn::_ResetGeneralDataAfterReborn(PlayerData * pstData, DT_ITEM_GCARD * pstGeneral)
{
    DT_ITEM_EQUIP* pstEquip = NULL;
    uint32_t dwEquipSeq = 0;
    ResEquipsListMgr_t& rstResEquipsListMgr = CGameDataMgr::Instance().GetResEquipsListMgr();
    RESEQUIPSLIST* pResEquipsList = NULL;
    for (int i = 0; i < MAX_EQUIP_TYPE_NUM; i++)
    {
        dwEquipSeq = pstGeneral->m_astEquipList[i].m_dwEquipSeq;
        pstEquip = Equip::Instance().Find(pstData, dwEquipSeq);
        if (pstEquip == NULL)
        {
            LOGERR("Equip not found");
            break;
        }
        pResEquipsList = rstResEquipsListMgr.Find(pstEquip->m_dwEquipListId);
        if (pResEquipsList != NULL)
        {
            pstEquip->m_dwId = pResEquipsList->m_grade[0];
            pstEquip->m_bLevel = 1;
            pstEquip->m_dwExp = 0;
            pstEquip->m_bGrowPhase = 0;
            pstEquip->m_bPhase = 1;
        }
    }
    pstGeneral->m_bArmyPhase = 1;
    pstGeneral->m_bArmyLv = 1;
    for (int i = 0; i < MAX_NUM_GENERAL_SKILL; i++)
    {
        pstGeneral->m_szSkillLevel[i] = 1;
    }
    pstGeneral->m_bPhase = 1;
    pstGeneral->m_bLevel = 1;
    pstGeneral->m_dwExp = 0;
    pstGeneral->m_bLvPhase = 1;
}
