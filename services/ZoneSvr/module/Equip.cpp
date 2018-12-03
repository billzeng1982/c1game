#include "Equip.h"
#include "LogMacros.h"
#include "../utils/FakeRandom.h"
#include "../gamedata/GameDataMgr.h"
#include "GeneralCard.h"
#include "Consume.h"
#include "Props.h"
#include "Item.h"
#include "Gem.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "dwlog_svr.h"
#include "ZoneLog.h"
#include "ov_res_public.h"
#include "Majesty.h"

using namespace PKGMETA;
using namespace DWLOG;

#define MAX_EQUIP_LV    (60)

int EquipCmp(const void *pstFirst, const void *pstSecond)
{
    DT_ITEM_EQUIP* pstItemFirst = (DT_ITEM_EQUIP*)pstFirst;
    DT_ITEM_EQUIP* pstItemSecond = (DT_ITEM_EQUIP*)pstSecond;

    int iResult = (int)pstItemFirst->m_dwSeq - (int)pstItemSecond->m_dwSeq;
    return iResult;
}

Equip::Equip()
{
    bzero((char*)&m_stEquip, sizeof(m_stEquip));
    bzero((char*)&m_ExpCardId, sizeof(m_ExpCardId));
    bzero((char*)&m_ExpCardExp, sizeof(m_ExpCardExp));
}

bool Equip::Init()
{
    ResBasicMgr_t& rstResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* pstResBasic = rstResBasicMgr.Find(EQUIPMENT_LV_UP_EXPERIENCE);

    ResPropsMgr_t& rstResPropsMgr = CGameDataMgr::Instance().GetResPropsMgr();

    for (int i=0; i<MAX_NUM_EQUIP_EXPCARD_TYPE; i++)
    {
        m_ExpCardId[i] = (uint32_t)pstResBasic->m_para[i];
        RESPROPS* pResprops = rstResPropsMgr.Find(m_ExpCardId[i]);
        if (pResprops == NULL)
        {
            LOGERR("Init equip failed, pResprops is null");
            return false;
        }
        m_ExpCardExp[i] = pResprops->m_dwExpEquip;
    }

    return true;
}

int Equip::Add(PlayerData* pstData, DT_ITEM_EQUIP* pstItem)
{
    // check
    PlayerEquipInfo& rstInfo = pstData->GetEquipInfo();

    if (rstInfo.m_iCount >= MAX_NUM_ROLE_EQUIP)
    {
        //LOGERR("Equip add failed, count is max. Usr=%s Count=%d", pstData->GetRoleBaseInfo().m_szRoleName, rstInfo.m_iCount);
        return -1;
    }

    pstItem->m_dwSeq = pstData->GetRoleBaseInfo().m_dwBagSeq++;    // add bag sequence
    if (rstInfo.Add(*pstItem))
    {
        //LOGRUN("Uin<%lu> Equip add. Usr=%s, Seq=%d, dwId=%d, Count=%d",
        //        pstData->m_ullUin, pstData->GetRoleBaseInfo().m_szRoleName, pstItem->m_dwSeq, pstItem->m_dwId, rstInfo.m_iCount);

        // 影响黑市计算权重参数
        //BlackMarket::Instance().ChgParamForBlackMarketRule(pstData, ITEM_TYPE_EQUIP, pstItem->m_dwId, true);
    }
    else
    {
        LOGERR("Uin<%lu> Equip add failed. Usr=%s", pstData->m_ullUin, pstData->GetRoleBaseInfo().m_szRoleName);
        return -1;
    }

    return (int)pstItem->m_dwSeq;
}

int Equip::Del(PlayerData* pstData, uint32_t dwSeq)
{
    m_stEquip.m_dwSeq = dwSeq;
    PlayerEquipInfo& rstInfo = pstData->GetEquipInfo();

    DT_ITEM_EQUIP* pstItem = Find(pstData, dwSeq);
    if (!pstItem)
    {
        // 影响黑市计算权重参数
        //BlackMarket::Instance().ChgParamForBlackMarketRule(pstData, ITEM_TYPE_EQUIP, pstItem->m_dwId, false);
    }

    if (!rstInfo.Del(dwSeq))
    {
        return ERR_NOT_FOUND;
    }

    LOGRUN("Equip del. usr=%s, seq=%d, count=%d",
            pstData->GetRoleBaseInfo().m_szRoleName, dwSeq, rstInfo.m_iCount );

    return ERR_NONE;
}

DT_ITEM_EQUIP* Equip::Find(PlayerData* pstData, uint32_t dwSeq)
{
    m_stEquip.m_dwSeq = dwSeq;
    PlayerEquipInfo& rstInfo = pstData->GetEquipInfo();

    DT_ITEM_EQUIP* pstEquip = rstInfo.Find(dwSeq);

    if (pstEquip ==NULL)
    {
        LOGERR("Uin<%lu>,Equip not found. Usr=%s, Seq=%d", pstData->m_ullUin, pstData->GetRoleBaseInfo().m_szRoleName, dwSeq);
        return NULL;
    }
    return pstEquip;
}


int Equip::LvUp(PlayerData* pstData, CS_PKG_EQUIP_LV_UP_REQ& rstEquipLvUpReq, SC_PKG_EQUIP_LV_UP_RSP& rstEquipLvUpRsp)
{
    DT_ITEM_EQUIP* pstEquip = Find(pstData, rstEquipLvUpReq.m_dwSeq);
    if (!pstEquip)
    {
        LOGERR("Player(%s) Uin(%lu) Equip LvUp failed, Equip seq(%u) not found", pstData->GetRoleBaseInfo().m_szRoleName,
                pstData->m_ullUin, rstEquipLvUpReq.m_dwSeq);
        return ERR_NOT_FOUND;
    }

    RESEQUIP* pstResEquip = CGameDataMgr::Instance().GetResEquipMgr().Find(pstEquip->m_dwId);
    if (pstResEquip == NULL)
    {
        LOGERR("Player(%s) Uin(%lu) Equip LvUp failed, Equip id(%u) not found", pstData->GetRoleBaseInfo().m_szRoleName,
                pstData->m_ullUin, pstEquip->m_dwId);
        return ERR_NOT_FOUND;
    }

    if (pstEquip->m_bLevel >= pstResEquip->m_wLevel)
    {
        LOGERR("Player(%s) Uin(%lu) Equip LvUp failed, Level(%d) is max", pstData->GetRoleBaseInfo().m_szRoleName,
                pstData->m_ullUin, pstEquip->m_bLevel );
        return ERR_EQUIP_TOP_LEVEL;
    }

    //计算总经验
    uint32_t dwIncExp = 0;
    ResPropsMgr_t& rstResPropsMgr = CGameDataMgr::Instance().GetResPropsMgr();
    for (int i=0; i<rstEquipLvUpReq.m_stConsumeList.m_bConsumeCount; i++)
    {
        DT_ITEM_CONSUME& rstItemConsume = rstEquipLvUpReq.m_stConsumeList.m_astConsumeList[i];
        if (rstItemConsume.m_bItemType == ITEM_TYPE_VIRTUAL_EXP)
        {
            dwIncExp += rstItemConsume.m_dwItemNum;
        }
        else if (rstItemConsume.m_bItemType == ITEM_TYPE_PROPS)
        {
            RESPROPS* pResProps = rstResPropsMgr.Find(rstItemConsume.m_dwItemId);
            if (!pResProps)
            {
                LOGERR("Player(%s) Uin(%lu) Equip LvUp failed, Props id(%d) not found", pstData->GetRoleBaseInfo().m_szRoleName,
                        pstData->m_ullUin, rstItemConsume.m_dwItemId);
                return ERR_NOT_FOUND;
            }
            if (!Props::Instance().IsEnough(pstData, rstItemConsume.m_dwItemId, rstItemConsume.m_dwItemNum))
            {
                LOGERR("Player(%s) Uin(%lu) Equip LvUp failed, Props id(%d) is not enough", pstData->GetRoleBaseInfo().m_szRoleName,
                        pstData->m_ullUin, rstItemConsume.m_dwItemId);
                return ERR_NOT_ENOUGH_PROPS;
            }
            dwIncExp += pResProps->m_dwExpEquip * rstItemConsume.m_dwItemNum;
        }
    }

    RESBASIC* pstResExpToGold = CGameDataMgr::Instance().GetResBasicMgr().Find(EQUIP_EXP_TO_GOLD);
    if (!pstResExpToGold)
    {
        LOGERR("Player(%s) Uin(%lu) Equip LvUp failed, basic id(%d) not found", pstData->GetRoleBaseInfo().m_szRoleName,
                pstData->m_ullUin, EQUIP_EXP_TO_GOLD);
        return false;
    }

    uint32_t dwExpToGold = 0;
    if (pstEquip->m_bType == EQUIP_TYPE_MOUNT || pstEquip->m_bType == EQUIP_TYPE_BOOK)
    {
        dwExpToGold = (uint32_t)pstResExpToGold->m_para[0];
    }
    else
    {
        dwExpToGold = 1;
    }

    //计算金币消耗
    uint32_t dwGoldConsume = dwIncExp * dwExpToGold;

    if (!Consume::Instance().IsEnoughGold(pstData, dwGoldConsume))
    {
        LOGERR("Player(%s) Uin(%lu) Equip LvUp failed, Gold num(%d) is not enough", pstData->GetRoleBaseInfo().m_szRoleName,
                pstData->m_ullUin, dwGoldConsume);
        return ERR_NOT_ENOUGH_GOLD;
    }

    //物品同步
    //金币消耗
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_GOLD, 0, -dwGoldConsume, rstEquipLvUpRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_EQUIP_LVUP);
    //材料消耗
    for (int i=0; i<rstEquipLvUpReq.m_stConsumeList.m_bConsumeCount; i++)
    {
        DT_ITEM_CONSUME& rstItemConsume = rstEquipLvUpReq.m_stConsumeList.m_astConsumeList[i];
        Item::Instance().ConsumeItem(pstData, rstItemConsume.m_bItemType, rstItemConsume.m_dwItemId, -rstItemConsume.m_dwItemNum,
                                  rstEquipLvUpRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_EQUIP_LVUP);
    }

    //升级经验消耗表
    RESCONSUME* pResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(pstResEquip->m_dwExpId);
    if (!pResConsume)
    {
        LOGERR("Player(%s) Uin(%lu) Equip LvUp failed, ResConsume id(%d) not found", pstData->GetRoleBaseInfo().m_szRoleName,
                 pstData->m_ullUin, pstResEquip->m_dwExpId);
        return ERR_SYS;
    }

    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

    //计算增加经验后的等级
    uint8_t bOldLv = pstEquip->m_bLevel;
    pstEquip->m_dwExp += dwIncExp;
    while ((pstEquip->m_bLevel < pstResEquip->m_wLevel) && (pstEquip->m_bLevel < rstMajestyInfo.m_wLevel))
    {
        if (pstEquip->m_dwExp >= pResConsume->m_lvList[pstEquip->m_bLevel])
        {
            pstEquip->m_bLevel++;
        }
        else
        {
            break;
        }
    }

    rstEquipLvUpRsp.m_bCurLv = pstEquip->m_bLevel;
    rstEquipLvUpRsp.m_dwCurExp = pstEquip->m_dwExp;
    rstEquipLvUpRsp.m_dwIncExp = dwIncExp;

    //设置为需要更新装备信息
    pstData->GetEquipInfo().SetUptFlag();

    DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(pstData, pstEquip->m_dwGCardId);
    //记武将培养日志
    if (pstGeneral)
    {
        if (bOldLv != pstEquip->m_bLevel)
        {
            GeneralCard::Instance().TriggerUpdateGCardLi(pstData, pstGeneral);
        }
        ZoneLog::Instance().WriteGeneralLog(pstData, pstEquip->m_dwGCardId, pstGeneral->m_bLevel, METHOD_GENERAL_OP_EQUIP_LVUP, pstEquip->m_dwId, bOldLv, pstEquip->m_bLevel,
                                            pstGeneral->m_bPhase, pstGeneral->m_wTrainValue, pstGeneral->m_bStar);
    }
    return ERR_NONE;
}


#if 0
// 装备升级等级上限 = min(主公等级, 当前阶配置最高等级)
int Equip::TotalLvUp(PlayerData* pstData, uint32_t dwGeneralId, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(pstData, dwGeneralId);
    if (pstGeneral ==NULL)
    {
        return ERR_NOT_FOUND;
    }

    uint16_t wLevel = pstData->GetMajestyInfo().m_wLevel;

    if (wLevel==0)
    {
        wLevel = 1;
    }

    ResEquipMgr_t& rstResEquipMgr = CGameDataMgr::Instance().GetResEquipMgr();
    ResConsumeMgr_t& rstResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();

    //生成装备信息列表，方便后面的处理
    int iRestNum = EQUIP_TYPE_MAX_NUM;
    DT_ITEM_EQUIP* pstEquip = NULL;
    EQUIP_EXP_INFO EquipList[EQUIP_TYPE_MAX_NUM];
    for (int i=0; i<EQUIP_TYPE_MAX_NUM; i++)
    {
        pstEquip = this->Find(pstData, pstGeneral->m_astEquipList[i].m_dwEquipSeq);
        if (pstEquip==NULL)
        {
            LOGERR("Uin<%lu> pstEquip is null", pstData->m_ullUin);
            return ERR_NOT_FOUND;
        }

        RESEQUIP* pstResEquip = rstResEquipMgr.Find(pstEquip->m_dwId);

        if (pstResEquip==NULL)
        {
            LOGERR("Uin<%lu> pstResEquip is null", pstData->m_ullUin);
            return ERR_SYS;
        }
        RESCONSUME* pResConsume = rstResConsumeMgr.Find(pstResEquip->m_dwExpId);
        if (!pResConsume )
        {
            LOGERR("Uin<%lu> pResConsume is null", pstData->m_ullUin);
            return ERR_SYS;
        }

        EquipList[i].pstEquip = pstEquip;
        EquipList[i].pstResEquip = pstResEquip;

        uint16_t wEquipMaxLv = (wLevel < (pstResEquip->m_wLevel)) ? wLevel : pstResEquip->m_wLevel;
        uint32_t dwMaxExp = wEquipMaxLv < pResConsume->m_dwLvCount ?  pResConsume->m_lvList[wEquipMaxLv] - 1 : pResConsume->m_lvList[wEquipMaxLv - 1];
        int iExp = dwMaxExp - pstEquip->m_dwExp;
        if (iExp <= 0)
        {
            iExp = 0;
            iRestNum--;
        }
        EquipList[i].iReqExp = iExp;
        EquipList[i].iGainExp = 0;
    }

    //计算经验卡总经验
    DT_ITEM_PROPS* pstProps = NULL;
    int iTotalExp = 0;
    for (int i = 0; i < MAX_NUM_EQUIP_EXPCARD_TYPE; i++)
    {
        pstProps = Props::Instance().Find(pstData, m_ExpCardId[i]);

        if (pstProps == NULL)
        {
            m_ExpCardNum[i] = 0;
            continue;
        }
        m_ExpCardNum[i] = pstProps->m_dwNum;
        iTotalExp += m_ExpCardExp[i] * m_ExpCardNum[i];
    }
    //TO DEL
    LOGRUN("Total Exp=%d", iTotalExp);

    if (iTotalExp == 0)
    {
        return ERR_RES_NOT_ENOUGH;
    }

    if (iRestNum <= 0)
    {
        return ERR_EQUIP_TOP_LEVEL;
    }

    //分配经验
    int iRestExp = iTotalExp;
    int i = EQUIP_TYPE_MAX_NUM;
    while (iRestExp != 0 && iRestNum != 0 && (i--) > 0)
    {
        _TotalLvUp(EquipList, iRestExp, &iRestExp, iRestNum, &iRestNum);
        if (iRestExp < 0)
        {
            LOGERR("Uin<%lu> RestExp < 0, error", pstData->m_ullUin);
            return ERR_SYS;
        }
        if (iRestNum < 0)
        {
            LOGERR("Uin<%lu> iRestNum < 0, error", pstData->m_ullUin);
            return ERR_SYS;
        }
    }

    //计算消耗
    _CalConsume(pstData, iTotalExp, iRestExp, rstSyncItemInfo);

    //升级装备
    int ibLevelUpTotal = 0;
    for (int i=0; i < EQUIP_TYPE_MAX_NUM; i++)
    {
        ibLevelUpTotal += _CalEquipLv(EquipList[i].pstEquip, EquipList[i].pstResEquip, EquipList[i].iGainExp);
    }

    pstData->GetEquipInfo().SetUptFlag();

    return ibLevelUpTotal;
}

//根据获得的经验计算等级
int Equip::_CalEquipLv(DT_ITEM_EQUIP* pstEquip, RESEQUIP* pstResEquip, uint32_t dwGainExp)
{
    uint8_t bLevel = pstEquip->m_bLevel;
    uint32_t dwExp = pstEquip->m_dwExp + dwGainExp;

    ResConsumeMgr_t& rstResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();
    RESCONSUME* pResConsume = rstResConsumeMgr.Find(pstResEquip->m_dwExpId);
    if (!pResConsume )
    {
        LOGERR("pResConsume is null");
        return ERR_SYS;
    }

    while (bLevel <= pstResEquip->m_wLevel)
    {
        if (dwExp >= pResConsume->m_lvList[bLevel-1])
        {
            bLevel++;
        }
        else
        {
            break;
        }
    }

    bLevel--;

    pstEquip->m_dwExp = dwExp;
    int ibLevelUp = bLevel - pstEquip->m_bLevel;
    pstEquip->m_bLevel = bLevel;

    return ibLevelUp;
}
//根据经验计算消耗的材料
void Equip::_CalConsume(PlayerData* pstData, int iTotalExp, int iRestExp, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    if (iRestExp == 0)
    {
        for (int i=0; i<MAX_NUM_EQUIP_EXPCARD_TYPE; i++)
        {
            Item::Instance().AddSyncItem(pstData, ITEM_TYPE_PROPS, m_ExpCardId[i], -m_ExpCardNum[i], rstSyncItemInfo, METHOD_GENERAL_OP_EQUIP_LVUP);
        }
        return;
    }

    int iConsumeExp = iTotalExp - iRestExp;
    if (iConsumeExp == 0)
    {
        return;
    }

    for (int i=0; i<MAX_NUM_EQUIP_EXPCARD_TYPE; i++)
    {
        if ((long) m_ExpCardExp[i] * m_ExpCardNum[i] > iConsumeExp)
        {
            Item::Instance().AddSyncItem(pstData, ITEM_TYPE_PROPS, m_ExpCardId[i], -(iConsumeExp/m_ExpCardExp[i] +1), rstSyncItemInfo, METHOD_GENERAL_OP_EQUIP_LVUP);
            break;
        }
        else
        {
            Item::Instance().AddSyncItem(pstData, ITEM_TYPE_PROPS, m_ExpCardId[i], -m_ExpCardNum[i], rstSyncItemInfo, METHOD_GENERAL_OP_EQUIP_LVUP);
            iConsumeExp -= m_ExpCardExp[i] * m_ExpCardNum[i];
        }
    }
}

//升级所有装备
void Equip::_TotalLvUp(EQUIP_EXP_INFO* pstEquipList,IN int iTotalExp, OUT int* pRestExp ,IN int iNum, OUT int* pRestNum)
{
    int iExp = iTotalExp / iNum;
    int iRestExp = iTotalExp;

    LOGRUN("Enter _TotalLvUp, Total Exp=%d, RestExp=%d, Equip Num=%d, Exp=%d", iTotalExp,iRestExp, iNum, iExp);

    for (int i = 0; i < EQUIP_TYPE_MAX_NUM; i++)
    {
        EQUIP_EXP_INFO& rstInfo = pstEquipList[i];
        if (rstInfo.iReqExp == 0)
        {
        }
        else if (rstInfo.iReqExp <= iExp)
        {
            rstInfo.iGainExp= rstInfo.iReqExp;
            rstInfo.iReqExp = 0;
            iRestExp -= rstInfo.iGainExp;
            (*pRestNum)--;
        }
        else
        {
            rstInfo.iGainExp += iExp;
            rstInfo.iReqExp -= iExp;
            iRestExp -= iExp;
        }

        LOGRUN("EuipType=%d, ReqExp=%d, GainExp=%d, RestExp=%d", i, rstInfo.iReqExp, rstInfo.iGainExp, iRestExp);
    }

    if (iRestExp<5 && iRestExp>-5)
    {
        iRestExp = 0;
    }

    *pRestExp = iRestExp;

    LOGRUN("Exit _TotalLvUp, RestExp=%d, RestNum=%d", iRestExp, *pRestNum);

    return;
}
#endif

//返回当前的阶数
int Equip::PhaseUp(PlayerData* pstData, uint32_t dwSeq, SC_PKG_EQUIP_PHASE_UP_RSP& rstEquipPhaseUpRsp)
{
    DT_ITEM_EQUIP* pstEquip = Find(pstData, dwSeq);
    if (!pstEquip)
    {
        LOGERR("Player(%s) Uin(%lu) Equip PhaseUp failed, Seq(%u) not found", pstData->GetRoleBaseInfo().m_szRoleName,
                pstData->m_ullUin, dwSeq);
        return ERR_NOT_FOUND;
    }

    ResEquipMgr_t& rstResEquipMgr = CGameDataMgr::Instance().GetResEquipMgr();

    RESEQUIP* pstResCurEquip = rstResEquipMgr.Find(pstEquip->m_dwId);
    if (!pstResCurEquip)
    {
        LOGERR("Player(%s) Uin(%lu) Equip PhaseUp failed, Equip id(%u) not found", pstData->GetRoleBaseInfo().m_szRoleName,
                pstData->m_ullUin, pstEquip->m_dwId);
        return ERR_NOT_FOUND;
    }

    //满级后才能进阶
    if (pstEquip->m_bLevel != pstResCurEquip->m_wLevel)
    {
        return ERR_NOT_SATISFY_COND;
    }

    //获取进阶后的阶
    RESEQUIPSLIST* pResEquipsList = CGameDataMgr::Instance().GetResEquipsListMgr().Find(pstEquip->m_dwEquipListId);
    if (!pResEquipsList)
    {
        return ERR_SYS;
    }
    if (pstEquip->m_bGrowPhase >= pResEquipsList->m_dwGradeNum - 1)
    {
        return ERR_TOP_LEVEL;
    }

    uint32_t dwNewEquipId = pResEquipsList->m_grade[pstEquip->m_bGrowPhase + 1];

    //获取进阶后的新装备
    RESEQUIP* pstResNewEquip = rstResEquipMgr.Find(dwNewEquipId);
    if (!pstResNewEquip)
    {
        LOGERR("Player(%s) Uin(%lu) Equip PhaseUp failed, NewEquip id(%u) not found", pstData->GetRoleBaseInfo().m_szRoleName,
                pstData->m_ullUin, pstEquip->m_dwId);
        return ERR_SYS;
    }

    //检查材料是否够
    for (int i=0; i < pstResNewEquip->m_wMeterials; i++)
    {
        if (!Consume::Instance().IsEnough(pstData, pstResNewEquip->m_szMeterialType[i], pstResNewEquip->m_meterialId[i], pstResNewEquip->m_meterialNum[i]))
        {
            LOGERR("Player(%s) Uin(%lu) Equip PhaseUp failed, Meterials type(%d) id(%u) num(%u) is not enough", pstData->GetRoleBaseInfo().m_szRoleName,
                    pstData->m_ullUin, pstResNewEquip->m_szMeterialType[i], pstResNewEquip->m_meterialId[i], pstResNewEquip->m_meterialNum[i]);
            return ERR_NOT_ENOUGH_PROPS;
        }
    }

    //物品同步
    for (int i=0; i <pstResNewEquip->m_wMeterials; i++)
    {
        Item::Instance().RewardItem(pstData, pstResNewEquip->m_szMeterialType[i], pstResNewEquip->m_meterialId[i], -pstResNewEquip->m_meterialNum[i], rstEquipPhaseUpRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_EQUIP_PHASEUP);
    }
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_GOLD, 0, -pstResNewEquip->m_dwGoldConsume, rstEquipPhaseUpRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_EQUIP_PHASEUP);

    pstEquip->m_bGrowPhase++;
    pstEquip->m_dwId = dwNewEquipId;
    pstEquip->m_bPhase = pstResNewEquip->m_bPhase;

	//计算phaseup后的等级
	//升级经验消耗表
	RESCONSUME* pResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(pstResNewEquip->m_dwExpId);
	if (!pResConsume)
	{
		LOGERR("Player(%s) Uin(%lu) Equip LvUp failed, ResConsume id(%d) not found", pstData->GetRoleBaseInfo().m_szRoleName,
			pstData->m_ullUin, pstResNewEquip->m_dwExpId);
		return ERR_SYS;
	}

    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
	while (pstEquip->m_bLevel < pstResNewEquip->m_wLevel && pstEquip->m_bLevel < rstMajestyInfo.m_wLevel)
	{
		if (pstEquip->m_dwExp >= pResConsume->m_lvList[pstEquip->m_bLevel])
		{
			pstEquip->m_bLevel++;
		}
		else
		{
			break;
		}
	}

    rstEquipPhaseUpRsp.m_nErrNo = ERR_NONE;
    rstEquipPhaseUpRsp.m_dwSeq = pstEquip->m_dwSeq;
    rstEquipPhaseUpRsp.m_bEquipCnt = 0;
    rstEquipPhaseUpRsp.m_astEquipInfo[rstEquipPhaseUpRsp.m_bEquipCnt++] = *pstEquip;

    pstData->GetEquipInfo().SetUptFlag();

    DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(pstData, pstEquip->m_dwGCardId);
    if (NULL != pstGeneral)
    {
        GeneralCard::Instance().TriggerUpdateGCardLi(pstData, pstGeneral);
        ZoneLog::Instance().WriteGeneralLog(pstData, pstEquip->m_dwGCardId, pstGeneral->m_bLevel, METHOD_GENERAL_OP_EQUIP_PHASEUP, pstEquip->m_dwId, pstEquip->m_bGrowPhase - 1, pstEquip->m_bGrowPhase,
                                            pstGeneral->m_bPhase, pstGeneral->m_wTrainValue, pstGeneral->m_bStar);
    }


    return (int)pstEquip->m_bGrowPhase;
}

//一键进阶
int Equip::PhaseUpTotal(PlayerData* pstData, uint32_t GeneralId, SC_PKG_EQUIP_PHASE_UP_RSP& rstEquipPhaseUpRsp)
{
#if 0
    DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(pstData, GeneralId);
    if (NULL == pstGeneral)
    {
        LOGERR("pstGeneral is null");
        return ERR_NOT_FOUND;
    }

    ResEquipMgr_t& rstResEquipMgr = CGameDataMgr::Instance().GetResEquipMgr();
    DT_ITEM_EQUIP* pstEquip = NULL;

    rstEquipPhaseUpRsp.m_nErrNo = ERR_NOT_ENOUGH_PROPS;
    rstEquipPhaseUpRsp.m_bEquipCnt = 0;
    for (int i = 0; i < MAX_EQUIP_TYPE_NUM; i++)
    {
        pstEquip = this->Find(pstData, pstGeneral->m_astEquipList[i].m_dwEquipSeq);
        if (pstEquip == NULL)
        {
            LOGERR("Uin<%lu> pstEquip is null", pstData->m_ullUin);
            continue;
        }

        RESEQUIP* pstResEquip = rstResEquipMgr.Find(pstEquip->m_dwId);
        if (pstResEquip==NULL)
        {
            LOGERR("Uin<%lu> pstResEquip is null", pstData->m_ullUin);
            return ERR_SYS;
        }

        //满级后才能进阶
        if (pstEquip->m_bLevel != pstResEquip->m_wLevel)
        {
            continue;
        }

        //获取进阶后的阶
        uint32_t dwNewEquipId = 0;
        ResEquipsListMgr_t& rstResEquipsListMgr = CGameDataMgr::Instance().GetResEquipsListMgr();
        RESEQUIPSLIST* pResEquipsList = rstResEquipsListMgr.Find(pstEquip->m_dwEquipListId);
        if (pResEquipsList == NULL)
        {
            return ERR_SYS;
        }

        if (pstEquip->m_bGrowPhase >= pResEquipsList->m_dwGradeNum - 1)
        {
            continue;
        }

        dwNewEquipId = pResEquipsList->m_grade[pstEquip->m_bGrowPhase + 1];

        RESEQUIP* pstResNewEquip = rstResEquipMgr.Find(dwNewEquipId);
        if (pstResNewEquip==NULL)
        {
            LOGERR("Uin<%lu> pstResNewEquip not found, id=%u", pstData->m_ullUin, dwNewEquipId);
            return ERR_SYS;
        }

        //检查材料是否够
        uint8_t bEnoughProps = 1;
        for (int i = 0; i < pstResNewEquip->m_wMeterials; i++)
        {
            if (!Props::Instance().IsEnough(pstData, pstResNewEquip->m_meterialId[i], pstResNewEquip->m_meterialNum[i]))
            {
                LOGERR("Props is not Enough");
                bEnoughProps = 0;
                break;
            }
        }

        if (!bEnoughProps)
        {
            continue;
        }

        if (!Consume::Instance().IsEnough(pstData, ITEM_TYPE_GOLD, pstResNewEquip->m_dwGoldConsume))
        {
            LOGERR("Uin<%lu> Gold is not enough!", pstData->m_ullUin);
            continue;
        }

        //物品同步
        for (int j = 0; j < pstResNewEquip->m_wMeterials; j++)
        {
            Item::Instance().AddSyncItem(pstData, ITEM_TYPE_PROPS, pstResNewEquip->m_meterialId[j], -pstResNewEquip->m_meterialNum[j], rstEquipPhaseUpRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_EQUIP_PHASEUP);
        }

        Item::Instance().AddSyncItem(pstData, ITEM_TYPE_GOLD, 0, -pstResNewEquip->m_dwGoldConsume, rstEquipPhaseUpRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_EQUIP_PHASEUP);
        pstEquip->m_bGrowPhase++;
        pstEquip->m_dwId = dwNewEquipId;
        pstEquip->m_bPhase = pstResNewEquip->m_bPhase;
        //memcpy(rstEquipPhaseUpRsp.m_astEquipInfo[i], pstEquip, sizeof(DT_ITEM_EQUIP));
        rstEquipPhaseUpRsp.m_astEquipInfo[rstEquipPhaseUpRsp.m_bEquipCnt++] = *pstEquip;
        rstEquipPhaseUpRsp.m_nErrNo = ERR_NONE;
    }

    pstData->GetEquipInfo().SetUptFlag();
#endif
    return ERR_NONE;
}

int Equip::UpStar(PlayerData* pstData, uint32_t dwSeq, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    /*
    //等级限制
    int iErrNo = Majesty::Instance().IsArriveLevel(pstData, LEVEL_LIMIT_EQUIP_STAR);
    if (ERR_NONE != iErrNo)
    {
        return iErrNo;
    }
    */

    DT_ITEM_EQUIP* pstEquip = Find(pstData, dwSeq);
    if (!pstEquip)
    {
        LOGERR("Uin<%lu> UpStar:BaseEquip is NULL. Usr=%s, Seq=%d", pstData->m_ullUin, pstData->GetRoleBaseInfo().m_szRoleName, dwSeq);
        return ERR_NOT_FOUND;
    }

    ResEquipMgr_t& rstResEquipMgr = CGameDataMgr::Instance().GetResEquipMgr();
    RESEQUIP* pResEquip = rstResEquipMgr.Find(pstEquip->m_dwId);
    if (!pResEquip )
    {
        LOGERR("Uin<%lu> UpStar:BaseEquipRes is null. Usr=%s, Seq=%d, dwId=%d", pstData->m_ullUin, pstData->GetRoleBaseInfo().m_szRoleName, dwSeq, pstEquip->m_dwId);
        return ERR_NOT_FOUND;
    }

    if (pstEquip->m_wStar >= RES_MAX_STAR_LEVEL -1)
    {
        return ERR_TOP_LEVEL;
    }

    ResEquipStarMgr_t& rstResEquipStarMgr = CGameDataMgr::Instance().GetResEquipStarMgr();

    //通过装备获取武将id，然后通过武将id获取升星信息
    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    RESGENERAL* pResGeneral = rstResGeneralMgr.Find(pstEquip->m_dwGCardId);
    if (!pResGeneral)
    {
        LOGERR("Uin<%lu> pResGeneral is null", pstData->m_ullUin);
        return ERR_SYS;
    }
    uint16_t wGeneralEquipStarId = pResGeneral->m_equipStarList[pstEquip->m_bType - 1];//wGeneralEquipStarId的值为该装备当前星级对应在generalequipstar表中的id
    ResGeneralEquipStarMgr_t& rstResGeneralEquipStarMgr = CGameDataMgr::Instance().GetResGeneralEquipStarMgr();
    RESGENERALEQUIPSTAR* pResGeneralEquipStar = rstResGeneralEquipStarMgr.Find(wGeneralEquipStarId);
    if (!pResGeneralEquipStar)
    {
        LOGERR("Uin<%lu> pResGeneralEquipStar(%d) is null", pstData->m_ullUin, wGeneralEquipStarId);
        return ERR_SYS;
    }

    RESEQUIPSTAR* pResEquipStar = rstResEquipStarMgr.Find(pResGeneralEquipStar->m_starList[pstEquip->m_wStar + 1]);
    if (!pResEquipStar)
    {
        LOGERR("Uin<%lu> pResEquipStar is null", pstData->m_ullUin);
        return ERR_SYS;
    }

    //检查材料是否够
    for (int i=0; i<pResEquipStar->m_wMeterials; i++)
    {
        if (!Props::Instance().IsEnough(pstData, pResEquipStar->m_meterialId[i], pResEquipStar->m_meterialNum[i]))
        {
            LOGERR("Uin<%lu> Props is not Enough", pstData->m_ullUin);
            return ERR_NOT_ENOUGH_PROPS;
        }
    }
    if (!Consume::Instance().IsEnough(pstData, ITEM_TYPE_GOLD, pResEquipStar->m_dwEquipstargolduse))
    {
        LOGERR("Uin<%lu> Gold is not enough!", pstData->m_ullUin);
        return ERR_NOT_ENOUGH_GOLD;
    }
    //物品同步
    for (int j=0; j <pResEquipStar->m_wMeterials; j++)
    {
        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, pResEquipStar->m_meterialId[j], -pResEquipStar->m_meterialNum[j], rstSyncItemInfo, METHOD_GENERAL_OP_EQUIP_STARUP);
    }
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_GOLD, 0, -pResEquipStar->m_dwEquipstargolduse, rstSyncItemInfo, METHOD_GENERAL_OP_EQUIP_STARUP);
    //升星
    pstEquip->m_wStar++;
    pstData->GetEquipInfo().SetUptFlag();

    //记武将培养日志
    DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(pstData, pstEquip->m_dwGCardId);
    if (pstGeneral)
    {
        GeneralCard::Instance().TriggerUpdateGCardLi(pstData, pstGeneral);
        ZoneLog::Instance().WriteGeneralLog(pstData, pstEquip->m_dwGCardId, pstGeneral->m_bLevel, METHOD_GENERAL_OP_EQUIP_STARUP, pstEquip->m_dwId, pstEquip->m_wStar - 1, pstEquip->m_wStar,
            pstGeneral->m_bPhase, pstGeneral->m_wTrainValue, pstGeneral->m_bStar);
    }


    return ERR_NONE;
}

int Equip::Recycle(PlayerData* pstData, uint32_t dwGeneralId, OUT DT_SYNC_ITEM_INFO& rstRewardItemInfo, OUT uint32_t* dwGoldUsed, uint32_t dwRatio)
{
    DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(pstData, dwGeneralId);
    if (pstGeneral == NULL )
    {
        return ERR_NOT_FOUND;
    }
    map<uint32_t,uint32_t> tmpPropUsed;
    uint32_t dwtmpGoldUesd = 0;
    uint32_t dwTotalExp = 0;
    for (int i = 0; i < EQUIP_TYPE_MAX_NUM; i++)
    {
        this->_Recycle(pstData, pstGeneral->m_astEquipList[i].m_dwEquipSeq, dwtmpGoldUesd, tmpPropUsed, dwTotalExp);
    }

    for (int i = MAX_NUM_EQUIP_EXPCARD_TYPE - 1, dwNum = 0; i >= 0; i--)
    {//经验少的序号小
        if (0 == dwTotalExp)
        {
            break;
        }
        dwNum = dwTotalExp / m_ExpCardExp[i];
        if (dwNum > 0)
        {//消耗经验丹
            tmpPropUsed[ m_ExpCardId[i] ] += dwNum;
        }
        dwTotalExp %= m_ExpCardExp[i];
    }

    for (map<uint32_t,uint32_t>::iterator iter = tmpPropUsed.begin(); iter != tmpPropUsed.end(); iter++)
    {
        Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, iter->first, (iter->second * dwRatio + 50)  / 100 ,
            rstRewardItemInfo, METHOD_GENERAL_OP_REBORN);
    }
    *dwGoldUsed  = (dwtmpGoldUesd * dwRatio + 50) / 100;
    return 0;
}

int Equip::_Recycle(PlayerData* pstData,  uint32_t dwSeq, OUT uint32_t& dwGoldUsed, OUT map<uint32_t,uint32_t>& PropUsed, OUT uint32_t& dwTotalExp)
{
    DT_ITEM_EQUIP* pstEquip = Find(pstData, dwSeq);
    if (!pstEquip)
    {
        LOGERR("Uin<%lu> UpStar:BaseEquip is NULL.  Seq=%d", pstData->m_ullUin,  dwSeq);
        return ERR_NOT_FOUND;
    }
    LOGRUN("Recycle equip:Uin<%lu>,Seq<%u>,Id<%u>,lv<%u>,Exp<%u>,Phase<%u>,Star<%u>",pstData->m_ullUin,
        pstEquip->m_dwSeq, pstEquip->m_dwId, pstEquip->m_bLevel, pstEquip->m_dwExp, pstEquip->m_bPhase, pstEquip->m_wStar);
    //升级消耗计算
    //      升级不消耗金币
    map<uint32_t,uint32_t> tmpPropUsed;
    int i, j;
    char RunLog[2014] = {0};

    RESEQUIPSLIST* pResEquipsList = CGameDataMgr::Instance().GetResEquipsListMgr().Find(pstEquip->m_dwEquipListId);
    if (pResEquipsList == NULL)
    {
        LOGERR("Recycle equip:Uin<%lu> pResEquipsList is nuLL", pstData->m_ullUin);
        return ERR_SYS;
    }
    uint32_t dwEquipId = 0;
    RESEQUIP* pResEquip = NULL;
    uint32_t dwTmpGoldUsed = 0;

    //升阶消耗计算
    StrCat(RunLog, 1024, "#Phase:");
    for(i = 1; i <= pstEquip->m_bGrowPhase; i++)
    {
        dwEquipId = pResEquipsList->m_grade[i];

        pResEquip = CGameDataMgr::Instance().GetResEquipMgr().Find(dwEquipId);
        if (pResEquip == NULL)
        {
            LOGERR("Uin<%lu> pResEquip is nuLL id=%u", pstData->m_ullUin, dwEquipId);
            return ERR_SYS;
        }
        dwTmpGoldUsed += pResEquip->m_dwGoldConsume;
        //计算消耗道具
        for (j = 0; j < pResEquip->m_wMeterials; j++)
        {
            tmpPropUsed[ pResEquip->m_meterialId[j] ] += pResEquip->m_meterialNum[j];
            StrCat(RunLog, 1024, "%u-%u|", pResEquip->m_meterialId[j], pResEquip->m_meterialNum[j]);
        }
    }
    StrCat(RunLog, 1024, "Gold-%u#", dwTmpGoldUsed);
    pResEquip = CGameDataMgr::Instance().GetResEquipMgr().Find(pstEquip->m_dwId);

    //新增resgeneralequipstar表，将装备与武将关联起来
    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    RESGENERAL* pResGeneral = rstResGeneralMgr.Find(pstEquip->m_dwGCardId);//判空
    if (pResGeneral == NULL)
    {
        LOGERR("Uin<%lu> pResGeneral is null id=%u", pstData->m_ullUin, pResGeneral->m_dwId);
        return ERR_SYS;
    }
    assert(pstEquip->m_bType >= 1);
    uint16_t wGeneralEquipStarId = pResGeneral->m_equipStarList[pstEquip->m_bType - 1];//wGeneralEquipStarId的值为该装备当前星级对应在generalequipstar表中的id
    ResGeneralEquipStarMgr_t& rstResGeneralEquipStarMgr = CGameDataMgr::Instance().GetResGeneralEquipStarMgr();
    RESGENERALEQUIPSTAR* pResGeneralEquipStar = rstResGeneralEquipStarMgr.Find(wGeneralEquipStarId);//判空
    if (pResGeneralEquipStar == NULL)
    {
        LOGERR("Uin<%lu> pResGeneralEquipStar is null id=%u", pstData->m_ullUin, wGeneralEquipStarId);
        return ERR_SYS;
    }

    if (pResEquip == NULL)
    {
        LOGERR("Uin<%lu> pResEquip is nuLL id=%u", pstData->m_ullUin, dwEquipId);
        return ERR_SYS;
    }
    RESEQUIPSTAR* pResEquipStar = CGameDataMgr::Instance().GetResEquipStarMgr().Find(pResGeneralEquipStar->m_starList[pstEquip->m_wStar + 1]);

    //升星消耗计算
    StrCat(RunLog, 1024, "#Star:");
    for (i = 1; i <= pstEquip->m_wStar; i++)
    {
        pResEquipStar = CGameDataMgr::Instance().GetResEquipStarMgr().Find(pResEquip->m_starGrow[i]);
        if (NULL == pResEquipStar)
        {
            LOGERR("Uin<%lu> pResEquipStar is null", pstData->m_ullUin);
            return ERR_SYS;
        }
        for (j = 0; j < pResEquipStar->m_wMeterials; j++)
        {
            if (pResEquipStar->m_meterialNum[j] == 0)
            {
                continue;
            }
            tmpPropUsed[ pResEquipStar->m_meterialId[j] ] += pResEquipStar->m_meterialNum[j];
           StrCat(RunLog, 1024, "%u-%u|", pResEquip->m_meterialId[j], pResEquip->m_meterialNum[i]);
        }
       dwTmpGoldUsed += pResEquipStar->m_dwEquipstargolduse;
    }
    StrCat(RunLog, 1024, "Gold-%u#", dwTmpGoldUsed);
    // 增加经验
    dwTotalExp += pstEquip->m_dwExp;
     StrCat(RunLog, 1024, "Exp:");
    //重置装备属性
    pstEquip->m_dwId = pResEquipsList->m_grade[0];
    pstEquip->m_bLevel = 1;
    pstEquip->m_dwExp = 0;
    pstEquip->m_bGrowPhase = 0;
    pstEquip->m_bPhase = 1;
    pstEquip->m_wStar = 0;
    //结果返出去

    dwGoldUsed += dwTmpGoldUsed;
    for (map<uint32_t,uint32_t>::iterator iter = tmpPropUsed.begin(); iter != tmpPropUsed.end(); iter++)
    {
        PropUsed[iter->first] += iter->second;
    }
    LOGRUN("Equip:recycle: %s", RunLog);
    return ERR_NONE;
}
