#include "Gem.h"
#include "GeneralCard.h"
#include "Props.h"
#include "LogMacros.h"
#include "../gamedata/GameDataMgr.h"
#include "Item.h"
#include "Consume.h"
#include "dwlog_def.h"
#include "player/Player.h"
#include "Marquee.h"
#include "ov_res_public.h"
#include "Majesty.h"

using namespace PKGMETA;
using namespace DWLOG;

#define GEM_SLOT_EMPTY 0
#define GEM_SYNTHETICTYPE_ONE 1
#define GEM_SYNTHETICTYPE_ALL 2
#define GEM_GEM_TYPE_X 255      //所有宝石类型


//  穿上宝石    -- 如果当前位置有,会替换掉
int Gem::Up(PlayerData* pstData, IN CS_PKG_GEM_UP_REQ& rstReq)
{
	/*
	//等级限制
	int iErrNo = Majesty::Instance().IsArriveLevel(pstData, LEVEL_LIMIT_GEM_UP);
	if (ERR_NONE != iErrNo)
	{
		return iErrNo;
	}
	*/
    //检查参数
    if (!_CheckSlotValid(rstReq.m_bSlot))
    {
        LOGERR("Argus error,Player Cheat!! Uin=%lu, GCard=%u Slot=%hhu, GemId=%u ", pstData->m_ullUin, rstReq.m_dwGeneralID, rstReq.m_bSlot, rstReq.m_dwGemSeq);
        return ERR_WRONG_PARA;
    }
    DT_ITEM_GCARD* pstGCard = GeneralCard::Instance().Find(pstData, rstReq.m_dwGeneralID);
    if (NULL == pstGCard)
    {
        LOGERR("GeneralCard not found. Uin=%lu, GCard=%u Slot=%hhu, GemId=%u ", pstData->m_ullUin, rstReq.m_dwGeneralID, rstReq.m_bSlot, rstReq.m_dwGemSeq);
        return ERR_NOT_FOUND;
    }
    int iRet = _UpOneGem(pstData, pstGCard, rstReq.m_bSlot, rstReq.m_dwGemSeq);
    if (iRet == ERR_NONE)
    {
        GeneralCard::Instance().TriggerUpdateGCardLi(pstData, pstGCard, false);
    }
    return iRet;
}




int Gem::_UpOneGem(PlayerData* pstData, DT_ITEM_GCARD* pstGCard, uint8_t bSlot, uint32_t dwGemSeq)
{
    uint8_t bSlotIndex = bSlot;
    uint32_t dwGeneralID = pstGCard->m_dwId;
    //检查包裹中宝石
    DT_ITEM_PROPS* pstProps = Props::Instance().Find(pstData, dwGemSeq);
    if (NULL == pstProps || pstProps->m_dwNum < 1)
    {
        LOGERR("Gem not found. Uin=%lu, GCard=%u Slot=%hhu, GemId=%u ", pstData->m_ullUin, dwGeneralID, bSlot, dwGemSeq);
        return ERR_NOT_FOUND;
    }
    RESGEMLIST* poResGem = CGameDataMgr::Instance().GetResGemListMgr().Find( dwGemSeq );
    if (NULL == poResGem)
    {
        LOGERR("The poResGem is NULL. Uin=%lu, GCard=%u Slot=%hhu, GemId=%u ", pstData->m_ullUin, dwGeneralID, bSlot, dwGemSeq);
        return ERR_SYS;
    }

    //检查该部位能否放该类型宝石
    RESGEMLIMIT* poRsgGemLimit = CGameDataMgr::Instance().GetResGemLimitMgr().GetResByPos(bSlotIndex);
    if (NULL == poRsgGemLimit)
    {
        LOGERR("The poRsgGemLimit is NULL. Uin<%lu>, ResIndex<%hhu>", pstData->m_ullUin, bSlotIndex);
        return ERR_SYS;
    }
    bool bIsOk = false;
    for (int i = 0; i < sizeof(poRsgGemLimit->m_szPara)/sizeof(*poRsgGemLimit->m_szPara); i++)
    {
        if (poRsgGemLimit->m_szPara[i] == GEM_GEM_TYPE_X ||  poRsgGemLimit->m_szPara[i] == poResGem->m_bGemType)
        {
            bIsOk = true;
        }
    }
    if (!bIsOk)
    {
        return ERR_GEM_TYPE_INVALID;
    }

    uint32_t uDownSeq = pstGCard->m_GemSlot[bSlotIndex];
    //检查孔是否开启
    if (poRsgGemLimit->m_bLvLimit > pstGCard->m_bLevel)
    {
        LOGERR("The slot does not open.  Uin=%lu, GCard=%u Slot=%hhu, GemId=%u ", pstData->m_ullUin, dwGeneralID, bSlot, dwGemSeq);
        return ERR_GEM_SLOT_NOT_OPEN;
    }

    //穿上宝石
    pstGCard->m_GemSlot[bSlotIndex] = dwGemSeq;
    Item::Instance().AddItem(pstData, ITEM_TYPE_PROPS, dwGemSeq, -1, m_stTmpItem, METHOD_GEM_UP);
    if(uDownSeq != GEM_SLOT_EMPTY)
    {
        Item::Instance().AddItem(pstData, ITEM_TYPE_PROPS, uDownSeq, 1, m_stTmpItem, METHOD_GEM_DOWN);
    }
    return ERR_NONE;
}

int Gem::Down(PlayerData* pstData, IN CS_PKG_GEM_DOWN_REQ& rstReq)
{
    //检查参数
    if (!_CheckSlotValid(rstReq.m_bSlot))
    {
        LOGERR("Argus error,Player Cheat!! Uin=%lu, GCardId=%u, Slot=%hhu", pstData->m_ullUin, rstReq.m_dwGeneralID, rstReq.m_bSlot);
        return ERR_WRONG_PARA;
    }
    uint8_t bSlotIndex = rstReq.m_bSlot;

    //检查装备
    DT_ITEM_GCARD* pstGCard = GeneralCard::Instance().Find(pstData, rstReq.m_dwGeneralID);
    if (NULL == pstGCard)
    {
        LOGERR("GeneralCard not found. Uin=%lu, GCardId=%u", pstData->m_ullUin, rstReq.m_dwGeneralID);
        return ERR_NOT_FOUND;
    }
    int iRet = ERR_NONE;
    if (CONST_DOWN_TYPE_ALL == rstReq.m_bType)
    {
        for (size_t i = 0; i < MAX_GEM_SLOT_NUM; i++)
        {
            _DownOneGem(pstData, pstGCard, i);
        }
        iRet = ERR_NONE;
    }
    else
    {
        iRet = _DownOneGem(pstData, pstGCard, bSlotIndex);
        if (ERR_NONE != iRet)
        {
            LOGERR("The slot ie empty. Uin=%lu, GCardId=%u, Slot=%hhu", pstData->m_ullUin, rstReq.m_dwGeneralID, rstReq.m_bSlot);
        }
    }
    if (iRet == ERR_NONE)
    {
        GeneralCard::Instance().TriggerUpdateGCardLi(pstData, pstGCard, false);
    }
    return iRet;
}




int Gem::_DownOneGem(PlayerData* pstData, DT_ITEM_GCARD* pstGCard, uint8_t bSlotIndex)
{
    if (GEM_SLOT_EMPTY == pstGCard->m_GemSlot[bSlotIndex])
    {//没有宝石可卸载
        //LOGERR("The slot ie empty. Uin=%lu, GCardId=%u, Slot=%hhu", pstData->m_ullUin, rstReq.m_dwGeneralID, rstReq.m_bSlot);
        return ERR_GEM_NO_GEM_TO_DOWN;
    }
    Item::Instance().AddItem(pstData, ITEM_TYPE_PROPS, pstGCard->m_GemSlot[bSlotIndex], 1, m_stTmpItem, METHOD_GEM_DOWN);
    pstGCard->m_GemSlot[bSlotIndex] = GEM_SLOT_EMPTY;
    return ERR_NONE;
}


//合成宝石
int Gem::Synthetic(PlayerData* pstData, IN CS_PKG_GEM_SYNTHETIC_REQ& rstReq, OUT SC_PKG_GEM_SYNTHETIC_RSP& rstRsp)
{
    uint32_t dwGemSeq = rstReq.m_dwGemSeq;
    uint8_t bSlotIndex = rstReq.m_bSlot;
    DT_ITEM_GCARD* pstGCard = NULL;
    int iRet = ERR_NONE;
    if (1 == rstReq.m_bDownUp)
    {
        pstGCard = GeneralCard::Instance().Find(pstData, rstReq.m_dwGeneralID);
        if (NULL == pstGCard)
        {
            LOGERR("GeneralCard not found. Uin=%lu, GCardId=%u", pstData->m_ullUin, rstReq.m_dwGeneralID);
            return ERR_NOT_FOUND;
        }
        dwGemSeq = pstGCard->m_GemSlot[bSlotIndex];

        iRet = _DownOneGem(pstData, pstGCard, rstReq.m_bSlot);
        if (iRet != ERR_NONE)
        {//没有宝石
            LOGERR("The slot ie empty. Uin=%lu, GCardId=%u, Slot=%hhu", pstData->m_ullUin, rstReq.m_dwGeneralID, bSlotIndex);
            return iRet;
        }

    }
    ResGemListMgr_t& rstResGemListMgr = CGameDataMgr::Instance().GetResGemListMgr();
    RESGEMLIST* poResGem = rstResGemListMgr.Find(dwGemSeq);
    if (NULL == poResGem)
    {
        LOGERR("The poResGem is NULL. Uin<%lu>, GemListID<%u>", pstData->m_ullUin, dwGemSeq);
        return ERR_SYS;
    }
    if (poResGem->m_dwGemSyntheticId == poResGem->m_dwItemId || 0 == poResGem->m_dwGemSyntheticId)
    {//合成已满
        return ERR_GEM_SYNTHETIC_MAX;
    }
    //检查包裹中的宝石
    DT_ITEM_PROPS* pstProps = Props::Instance().Find(pstData, dwGemSeq);
    if (NULL == pstProps || pstProps->m_dwNum < 1)
    {
        LOGERR("Gem not found. Uin=%lu, Seq=%u", pstData->m_ullUin, dwGemSeq);
        return ERR_NOT_FOUND;
    }
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
    int iSyntheticCnt = (rstReq.m_bType == GEM_SYNTHETICTYPE_ONE) ? 1: (pstProps->m_dwNum / poResGem->m_bGemConsume); //宝石可升级的次数

    iSyntheticCnt = MIN(iSyntheticCnt, rstMajestyInfo.m_dwGold/poResGem->m_dwGoldConsume);                            //宝石可升级的次数 金币可支持的升级次数 取最小
    if (iSyntheticCnt < 1)
    {
        return ERR_GEM_NO_ENOUGH_GOLD_GEM;
    }
    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, dwGemSeq, -iSyntheticCnt*poResGem->m_bGemConsume, rstRsp.m_stSyncItemInfo, METHOD_GEM_SYNTHETIC);
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_GOLD, 0, -iSyntheticCnt*poResGem->m_dwGoldConsume, rstRsp.m_stSyncItemInfo, METHOD_GEM_SYNTHETIC);

    if (1 == rstReq.m_bDownUp)
    {
        Item::Instance().AddItem(pstData, ITEM_TYPE_PROPS, poResGem->m_dwGemSyntheticId, iSyntheticCnt, m_stTmpItem, METHOD_GEM_SYNTHETIC);
        iRet = _UpOneGem(pstData, pstGCard, bSlotIndex, poResGem->m_dwGemSyntheticId);
        if (iRet != ERR_NONE)
        {
            LOGERR("Uin=%lu, after Synthetic, up the gem error! Seq=%u", pstData->m_ullUin, poResGem->m_dwGemSyntheticId);
        }
    }
    else
    {
        Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, poResGem->m_dwGemSyntheticId, iSyntheticCnt, rstRsp.m_stSyncItemInfo, METHOD_GEM_SYNTHETIC);
    }

	poResGem = rstResGemListMgr.Find(poResGem->m_dwGemSyntheticId);
	if (poResGem->m_bGemLv == 9)
	{
		Marquee::Instance().AddLocalMsg(pstData->GetRoleBaseInfo().m_szRoleName, NINE_LEVEL_GEM, poResGem->m_dwItemId);
	}

    return ERR_NONE;

}


int Gem::Repair(PlayerData* pstData)
{

	int iSlot1 = 4;
	int iSlot2 = 5;
	DT_ROLE_GCARD_INFO& rstGcardInfo = pstData->GetGCardInfo();
	for (int i = 0; i < rstGcardInfo.m_iCount; i++)
	{
        for(int j = 0 ; j < MAX_GEM_SLOT_NUM; j++)
        {
		    _RepairDown(pstData, rstGcardInfo.m_astData[i], j);
        }

	}
}

void Gem::_RepairDown(PlayerData* pstData, DT_ITEM_GCARD& rstGCard, int iSlotIndex)
{
	uint32_t dwGemId = rstGCard.m_GemSlot[iSlotIndex];
	if (dwGemId == GEM_SLOT_EMPTY)
	{
		return;
	}
	RESGEMLIMIT* pstGemLimit = CGameDataMgr::Instance().GetResGemLimitMgr().GetResByPos(iSlotIndex);
	//等级不够
	if (NULL == pstGemLimit || pstGemLimit->m_bLvLimit > rstGCard.m_bLevel)
	{
		return;
	}

	RESGEMLIST* poResGem = CGameDataMgr::Instance().GetResGemListMgr().Find(dwGemId);
	if (NULL == poResGem)
	{
		return;
	}

	bool bIsOk = false;
	for (int i = 0; i < sizeof(pstGemLimit->m_szPara) / sizeof(*pstGemLimit->m_szPara); i++)
	{
		if (pstGemLimit->m_szPara[i] == GEM_GEM_TYPE_X || pstGemLimit->m_szPara[i] == poResGem->m_bGemType)
		{
			bIsOk = true;
		}
	}
	//不满足条件, 卸下
	if (!bIsOk)
	{
		Item::Instance().AddItem(pstData, ITEM_TYPE_PROPS, dwGemId, 1, m_stTmpItem, METHOD_GEM_DOWN);
		rstGCard.m_GemSlot[iSlotIndex] = GEM_SLOT_EMPTY;
        LOGWARN("Uin=%lu repair and down the gem<%u>  ", pstData->m_ullUin, dwGemId);
	}

}

//自动开孔
// void Gem::AutoOpenSlot(PlayerData* pstData, DT_ITEM_GCARD* pstGCard)
// {
//     if (NULL == pstData || NULL == pstGCard)
//     {
//         LOGERR("pstData or pstEquip are null ");
//         return ;
//     }
//     RESGEMLIMIT* poRsgGemLimit = NULL;
//     for (uint8_t i = 0; i < MAX_GEM_SLOT_NUM; i++)
//     {
//         if (GEM_SLOT_LOCK != pstGCard->m_GemSlot[i] )
//         {
//             continue;
//         }
//         poRsgGemLimit = CGameDataMgr::Instance().GetResGemLimitMgr().GetResByPos(i);
//         if (NULL == poRsgGemLimit)
//         {
//             LOGERR("The poRsgGemLimit is NULL. Uin<%lu>, ResIndex<%hhu>", pstData->m_ullUin, i);
//             return;
//         }
//         if (poRsgGemLimit->m_bLvLimit > pstGCard->m_bLevel)
//         {// 武将等级不足
//             break;
//         }
//         pstGCard->m_GemSlot[i] = GEM_SLOT_EMPTY;
//         LOGRUN("Uin<%lu> Gem:open the SlotIndex <%hhu> of the GCard<%u> OK!", pstData->m_ullUin, i, pstGCard->m_dwId);
//     }
//     return;
// }


