#include "Tactics.h"
#include "dwlog_def.h"
#include "../gamedata/GameDataMgr.h"
#include "Item.h"
#include "Consume.h"


static int TacticsCmp(const void* pstFirst, const void* pstSecond)
{
    DT_ITEM_TACTICS* pstItemFirst = (DT_ITEM_TACTICS*)pstFirst;
    DT_ITEM_TACTICS* pstItemSecond = (DT_ITEM_TACTICS*)pstSecond;

    return (int)pstItemFirst->m_bType - (int)pstItemSecond->m_bType;
}

static int FindCmp(const void* pstFirst, const void* pstSecond)
{
    Type2Id* pstItemFirst = (Type2Id*)pstFirst;
    Type2Id* pstItemSecond = (Type2Id*)pstSecond;

    return (int)pstItemFirst->m_bType - (int)pstItemSecond->m_bType;
}

int Tactics::Init()
{
    ResBasicMgr_t& rstResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* pResBasic = rstResBasicMgr.Find(TACTICS_OPEN_LEVEL);
    if (!pResBasic)
    {
        LOGERR("pResBasic is NULL.");
        return false;
    }
    ResConsumeMgr_t& rstResMgr = CGameDataMgr::Instance().GetResConsumeMgr();
    m_pResConsume = rstResMgr.Find(pResBasic->m_para[0]);
    if (!m_pResConsume)
    {
        LOGERR("m_pResConsume is NULL.");
        return false;
    }

    //初始化通过类型查找ID的数组
    m_bCount = 0;
    ResTacticsMgr_t& rstResTacticsMgr = CGameDataMgr::Instance().GetResTacticsMgr();
    for (int i = 0; i < rstResTacticsMgr.GetResNum(); i++)
    {
        RESTACTICS* pResTactics = rstResTacticsMgr.GetResByPos(i);
        if (!pResTactics)
        {
            LOGERR("pResTactics is NULL.");
            return false;
        }
        size_t nmenb = m_bCount;
        if (nmenb >= MAX_TACTICS_NUM)
        {
            LOGERR("m_bCount<%d> reaches the max.", m_bCount);
            return false;
        }
        m_stType2Id.m_bType = pResTactics->m_bType;
        m_stType2Id.m_dwId = pResTactics->m_dwId;
        int iBinRet = MyBInsert(&m_stType2Id, m_astType2Id, &nmenb, sizeof(m_stType2Id), 1, FindCmp);
        m_bCount = nmenb;
        if (iBinRet == 0)
        {
            LOGERR("Insert failed. i<%d>", i);
            return false;
        }
    }

    return true;
}

void Tactics::Add(PlayerData* pstData, CS_PKG_TACTICS_ADD_REQ& rstReq, SC_PKG_TACTICS_ADD_RSP& rstRsp)
{
    DT_ROLE_TACTICS_INFO& rstInfo = pstData->GetTacticsInfo();

    int iRet = ERR_NONE;
    do 
    {
        uint8_t bMaxCount = GetTacticsNum(pstData);
        if (rstInfo.m_iCount >= bMaxCount)
        {
            LOGRUN("Used Count<%d> is larger or equal maxCount for current level <%d>", rstInfo.m_iCount, bMaxCount);
            iRet = ERR_SYS;
            break;
        }

        m_stTactics.m_bType = rstReq.m_bType;
        m_stTactics.m_bLevel = 1;
        size_t nmemb = (size_t)rstInfo.m_iCount;
        if (nmemb >= MAX_NUM_ROLE_TACTICS)
        {
            LOGERR("rstInfo.m_iCount<%d> reaches the max.", rstInfo.m_iCount);
            iRet = ERR_SYS;
            break;
        }
        int iInsertRet = MyBInsert(&m_stTactics, rstInfo.m_astData, &nmemb, sizeof(m_stTactics), 1, TacticsCmp);
        rstInfo.m_iCount = (int)nmemb;

        if (iInsertRet==0)
        {
            LOGERR("Add New Tactics<%d> failed.", rstReq.m_bType);
            iRet = ERR_SYS;
            break;
        }

        rstRsp.m_bType = rstReq.m_bType;
    } while (false);

    rstRsp.m_nErrNo = iRet;
}

void Tactics::LvUp(PlayerData* pstData, CS_PKG_TACTICS_LV_UP_REQ& rstReq, SC_PKG_TACTICS_LV_UP_RSP& rstRsp)
{


    int iRet = ERR_NONE;
    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    do 
    {
        DT_ITEM_TACTICS* pstTactics = GetTacticsItem(pstData, rstReq.m_bType);
        if (pstTactics == NULL)
        {
            LOGERR("pstTactics is NULL");
            iRet = ERR_SYS;
            break;
        }

        uint32_t dwTacticsId = GetTacticsId(pstData, rstReq.m_bType);
        if (dwTacticsId == 0)
        {
            LOGERR("dwTacticsId is not found. Type<%d>, Level<%d>.", pstTactics->m_bType, pstTactics->m_bLevel);
            iRet = ERR_SYS;
            break;
        }
        RESTACTICS* pResTactics = CGameDataMgr::Instance().GetResTacticsMgr().Find(dwTacticsId);
        if (!pResTactics)
        {
            LOGERR("pResTactics is NULL.");
            iRet = ERR_SYS;
            break;
        }

        RESCONSUME* pResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(pResTactics->m_dwGoldLvUpGrowId);
        if (!pResConsume)
        {
            LOGERR("pResConsume is NULL.");
            iRet = ERR_SYS;
            break;
        }
        int iGoldNum = (int)pResConsume->m_lvList[pstTactics->m_bLevel - 1];
        if (!Consume::Instance().IsEnoughGold(pstData, iGoldNum))
        {
            LOGERR("Gold is not enough.");
            iRet = ERR_NOT_ENOUGH_GOLD;
            break;
        }

        uint8_t bIndex = 0;
        for (int i = 0; i < pResTactics->m_bItemTypeCount; i++)
        {
            if(pstTactics->m_bLevel >= pResTactics->m_roleLevelNeeded[i])
            {
                bIndex = i;
            }
        }
        uint8_t bItemType = pResTactics->m_szItemType[bIndex];
        uint32_t dwItemId = pResTactics->m_itemId[bIndex];
        pResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(pResTactics->m_itemCountId[bIndex]);
        if (!pResConsume)
        {
            LOGERR("pResConsume is NULL.");
            iRet = ERR_SYS;
            break;
        }
        if (pstTactics->m_bLevel >= pResConsume->m_dwLvCount || pstTactics->m_bLevel >= MAX_TACTICS_LEVEL)
        {
            LOGERR("Already max level.");
            iRet = ERR_TACTICS_MAX_LEVEL;
            break;
        }
        int iItemCount = pResConsume->m_lvList[pstTactics->m_bLevel - 1];

        if (!Item::Instance().IsEnough(pstData, bItemType, dwItemId, iItemCount))
        {
            LOGERR("ItemType<%d> ItemId<%d> is not enough. needCount<%d>", bItemType, dwItemId, iItemCount);
            iRet = ERR_NOT_ENOUGH_PROPS;
            break;
        }

        Item::Instance().ConsumeItem(pstData, bItemType, dwItemId, -iItemCount, rstRsp.m_stSyncItemInfo, DWLOG::METHOD_TACTICS_LV_UP);
        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_GOLD, 0, -iGoldNum, rstRsp.m_stSyncItemInfo, DWLOG::METHOD_TACTICS_LV_UP);
        pstTactics->m_bLevel++;

        rstRsp.m_bType = rstReq.m_bType;
    } while (false);

    rstRsp.m_nErrNo = iRet;
}

uint8_t Tactics::GetTacticsNum(PlayerData* pstData)
{
    uint8_t bCount = 0;
    uint16_t wLevel = pstData->GetMajestyInfo().m_wLevel;
    for (int i = 0; i < (int)m_pResConsume->m_dwLvCount; i++)
    {
        if (wLevel >= m_pResConsume->m_lvList[i])
        {
            bCount++;
            continue;
        }
        break;
    }

    return bCount;
}

DT_ITEM_TACTICS* Tactics::GetTacticsItem(PlayerData* pstData, uint8_t bType)
{
    DT_ROLE_TACTICS_INFO& rstInfo = pstData->GetTacticsInfo();

    m_stTactics.m_bType = bType;
    m_stTactics.m_bLevel = 1;

    size_t nmenb = rstInfo.m_iCount;
    int iEqual = 0;
    int iRet = MyBSearch(&m_stTactics, rstInfo.m_astData, nmenb, sizeof(m_stTactics), &iEqual, TacticsCmp);
    if (iEqual)
    {
        return &rstInfo.m_astData[iRet];
    }
    else
    {
        return NULL;
    }
}

uint32_t Tactics::GetTacticsId(PlayerData* pstData, uint8_t bType)
{


    size_t nmenb = m_bCount;
    m_stType2Id.m_bType = bType;
    int iEqual = 0;
    int iBinRet = MyBSearch(&m_stType2Id, m_astType2Id, nmenb, sizeof(m_stType2Id), &iEqual, FindCmp);
    if (!iEqual)
    {
        return 0;
    }
    return m_astType2Id[iBinRet].m_dwId;
}


