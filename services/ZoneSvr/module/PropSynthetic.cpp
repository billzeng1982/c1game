#include "PropSynthetic.h"
#include "Props.h"
#include "Consume.h"
#include "Item.h"
#include "dwlog_def.h"
#include "Lottery.h"
#include "LogMacros.h"
//合成指定道具
#define SYNTHETIC_TYPE_SOME_ONE 1
//随机某一个
#define SYNTHETIC_TYPE_RAND_ONE 2

#define USED_ONE 1
#define USED_ALL 2


int PropSynthetic::Synthetic(PlayerData* pstData, CS_PKG_PROP_SYNTHETIC_REQ& rstReq, SC_PKG_PROP_SYNTHETIC_RSP& rstRsp)
{
#if 0
    //先直接做单步
   if (1 != rstReq.m_bType && 2 != rstReq.m_bType && !(rstReq.m_dwCount > 0 && rstReq.m_dwCount < MAX_PROP_SYNTHETIC_NUM))
   {
       LOGERR("Uin<%lu> para  wrong", pstData->m_ullUin);
        return ERR_WRONG_PARA;
   }
   RESSYNTHETIC* poResSynThetic = CGameDataMgr::Instance().GetResSyntheticMgr().Find(rstReq.m_dwRecipe);
   if (NULL == poResSynThetic)
   {
       LOGERR("Uin<%lu> poResSynThetic is NULL ", pstData->m_ullUin);
       return ERR_SYS;
   }
   int iRet = 0;
   switch (poResSynThetic->m_bType)
   {
   case SYNTHETIC_TYPE_SOME_ONE:
       iRet = SyntheticSomeOne(pstData, poResSynThetic, rstReq, rstRsp);
       break;
   case SYNTHETIC_TYPE_RAND_ONE:
       iRet = SyntheticRandOne(pstData, poResSynThetic, rstReq, rstRsp);
       break;
   default:
       LOGERR("Uin<%lu> poResSynThetic->m_bType<%u> error", pstData->m_ullUin, poResSynThetic->m_bType);
       iRet = ERR_SYS;
       break;
   }
  return iRet;
#endif
    return 0;
}
//单物品合成一
int PropSynthetic::SyntheticSomeOne(PlayerData* pstData, RESSYNTHETIC* poResSynThetic, CS_PKG_PROP_SYNTHETIC_REQ& rstReq, SC_PKG_PROP_SYNTHETIC_RSP& rstRsp)
{
#if 0
    uint32_t dwConsumeId = (uint32_t)poResSynThetic->m_para[0];
    uint32_t dwConsumeNum = 0;
    uint32_t dwOnceConsumeNum = (uint32_t)poResSynThetic->m_para[1];
    uint32_t dwConsumeGold = 0;
    uint32_t dwOnceConsumeGold = poResSynThetic->m_dwGoldenNum;
    uint32_t dwTargetId = poResSynThetic->m_dwResultPara;
    uint32_t dwTargetNum = 1;

    DT_ITEM_PROPS* poConsumeProp = Props::Instance().Find(pstData, dwConsumeId);
    if (NULL == poConsumeProp)
    {
        LOGERR("Uin<%lu> prop not enough", pstData->m_ullUin);
        return ERR_NOT_ENOUGH_PROPS;
    }

    if (USED_ALL == rstReq.m_bType)
    {
        dwTargetNum = poConsumeProp->m_dwNum / dwOnceConsumeNum;
    }
    else
    {
        dwTargetNum = 1;
    }
    dwConsumeNum = dwTargetNum * dwOnceConsumeNum;
    dwConsumeGold = dwOnceConsumeGold * dwTargetNum;

    if (dwConsumeId != rstReq.m_ItemId[0])
    {
        LOGERR("Uin<%lu> para  wrong", pstData->m_ullUin);
        return ERR_WRONG_PARA;
    }
    if (! Props::Instance().IsEnough(pstData, dwConsumeId, dwConsumeNum))
    {
        LOGERR("Uin<%lu> prop not enough", pstData->m_ullUin);
        return ERR_NOT_ENOUGH_PROPS;
    }
    if (! Consume::Instance().IsEnoughGold(pstData, dwConsumeGold))
    {
        LOGERR("Uin<%lu> Gold not enought", pstData->m_ullUin);
        return ERR_NOT_ENOUGH_GOLD;
    }
    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_GOLD, 0, -dwConsumeGold, rstRsp.m_stSyncItemInfo, DWLOG::METHOD_PROPSYNTHETIC_SOMEONE);
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, dwConsumeId, -dwConsumeNum, rstRsp.m_stSyncItemInfo, DWLOG::METHOD_PROPSYNTHETIC_SOMEONE);
    Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, dwTargetId, dwTargetNum, rstRsp.m_stSyncItemInfo,DWLOG::METHOD_PROPSYNTHETIC_SOMEONE);
    return ERR_NONE;
#endif
    return 0;
}

int PropSynthetic::SyntheticRandOne(PlayerData* pstData, RESSYNTHETIC* poResSynThetic, CS_PKG_PROP_SYNTHETIC_REQ& rstReq, SC_PKG_PROP_SYNTHETIC_RSP& rstRsp)
{
#if 0
    //单步合成
    RESSYNTHETICPROPS* poResSynTheticProps = NULL;
    uint32_t dwPropType = (uint32_t)poResSynThetic->m_para[0];
    uint32_t dwPropPhase = (uint32_t)poResSynThetic->m_para[1];
    uint32_t dwConusmeNum = 0;
    uint32_t dwOnceConsumeNum = (uint32_t)poResSynThetic->m_para[2];
    uint32_t dwOnceConsumeGold = poResSynThetic->m_dwGoldenNum;
    uint32_t dwConsumeGold = 0;
    uint32_t dwBoxId = poResSynThetic->m_dwResultPara;
    uint32_t dwTargetNum = 0;


    RESTREASUREBOX* pstResBox = CGameDataMgr::Instance().GetResTreasureBoxMgr().Find(dwBoxId);
    if (pstResBox == NULL)
    {
        LOGERR("Uin<%lu> not found Id<%u> ", pstData->m_ullUin, dwBoxId );
        return ERR_SYS;
    }
    for (size_t i = 0; i < rstReq.m_dwCount; i++)
    {
         poResSynTheticProps = CGameDataMgr::Instance().GetResSyntheticPropsMgr().Find(rstReq.m_ItemId[i]);
        if (NULL == poResSynTheticProps)
        {
            LOGERR("Uin<%lu> PrpId<%u> poResSynThetic is NULL ", pstData->m_ullUin, rstReq.m_ItemId[i]);
            return ERR_SYS;
        }
        if (! Props::Instance().IsEnough(pstData, rstReq.m_ItemId[i], rstReq.m_ItemNum[i]))
        {
            LOGERR("Uin<%lu> prop<%u> not enough", pstData->m_ullUin, rstReq.m_ItemId[i]);
            return ERR_NOT_ENOUGH_PROPS;
        }
        if ( !(poResSynTheticProps->m_bPropsPhase == dwPropPhase && poResSynTheticProps->m_bPropsType == dwPropType && poResSynTheticProps->m_synthetic[0] == rstReq.m_dwRecipe))
        {
            LOGERR("Uin<%lu> para wrong ", pstData->m_ullUin);
            return ERR_WRONG_PARA;
        }
        bool bRecipeOk = false;
        for (size_t j = 0 ; j < poResSynTheticProps->m_bSyntheticNum; j++)
        {
            if (poResSynTheticProps->m_synthetic[j] == rstReq.m_dwRecipe)
            {
                bRecipeOk = true;
                break;
            }
        }
        if (!bRecipeOk)
        {
            LOGERR("Uin<%lu> GameData error : PropId<%u> don't have  Recipe<%u>", pstData->m_ullUin, rstReq.m_ItemId[i], rstReq.m_dwRecipe) ;
            return ERR_SYS;
        }
       dwConusmeNum += rstReq.m_ItemNum[i];
    }
    if (USED_ALL == rstReq.m_bType)
    {
       dwTargetNum = dwConusmeNum / dwOnceConsumeNum;
    }
    else
    {
        dwTargetNum = (dwConusmeNum / dwOnceConsumeNum > 0) ? 1 : 0;
    }
    if (dwTargetNum < 1)
    {
        LOGERR("Uin<%lu> prop not enough, NUm<%u>", pstData->m_ullUin , dwConusmeNum);
        return ERR_NOT_ENOUGH_PROPS;
    }
	dwConsumeGold  = dwOnceConsumeGold * dwTargetNum;
    if (! Consume::Instance().IsEnoughGold(pstData, dwConsumeGold))
    {
        LOGERR("Uin<%lu> Gold not enought", pstData->m_ullUin);
        return ERR_NOT_ENOUGH_GOLD;
    }
    dwConusmeNum = dwOnceConsumeNum * dwTargetNum;  // 这里是为了不扣除多余的道具
    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_GOLD, 0, -dwConsumeGold, rstRsp.m_stSyncItemInfo, DWLOG::METHOD_PROPSYNTHETIC_RANDONE);
    for (uint32_t i = 0; i < rstReq.m_dwCount && dwConusmeNum > 0; i++)
    {
        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, rstReq.m_ItemId[i], -rstReq.m_ItemNum[i], rstRsp.m_stSyncItemInfo, DWLOG::METHOD_PROPSYNTHETIC_RANDONE);
        dwConusmeNum -= rstReq.m_ItemNum[i];
    }
    for (uint32_t i = 0 ; i < dwTargetNum; i ++ )
    {
        Lottery::Instance().DrawLotteryByPond(pstData, pstResBox->m_lotteryPoolIdList[0], rstRsp.m_stSyncItemInfo, DWLOG::METHOD_PROPSYNTHETIC_RANDONE);
    }
    return ERR_NONE;
#endif
    return 0;
}




