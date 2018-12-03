#include "Skin.h"
#include "GameDataMgr.h"
#include "GeneralCard.h"
#include "AsyncPvp.h"
#include "Item.h"
#include "Props.h"

using namespace PKGMETA;

int SkinCmp(const void *pstFirst, const void *pstSecond)
{
    DT_ITEM_SKIN* pstItemFirst = (DT_ITEM_SKIN*)pstFirst;
    DT_ITEM_SKIN* pstItemSecond = (DT_ITEM_SKIN*)pstSecond;

    int iResult = (int)pstItemFirst->m_dwId - (int)pstItemSecond->m_dwId;
    return iResult;
}

Skin::Skin()
{
}

Skin::~Skin()
{
}

DT_ITEM_SKIN* Skin::FindSkin(PlayerData* pstData, uint32_t dwId)
{
    DT_ROLE_MISC_INFO& rstInfo = pstData->GetMiscInfo();

    m_stSkin.m_dwId = dwId;

    int iEqual = 0;
    int iIndex = MyBSearch(&m_stSkin, rstInfo.m_astSkinList, rstInfo.m_wSkinCnt, sizeof(DT_ITEM_SKIN), &iEqual, SkinCmp);
    if (iEqual)
    {
        return &rstInfo.m_astSkinList[iIndex];
    }
    else
    {
        return NULL;
    }
}

int Skin::BuySkin(PlayerData* pstData, uint32_t dwSkinId, DT_SYNC_ITEM_INFO& rstSyncInfo)
{
    RESGENERALSKIN* pResSkin = CGameDataMgr::Instance().GetResGeneralSkinMgr().Find(dwSkinId);
    if (!pResSkin)
    {
        LOGERR("Player<%lu> ResSkin<%u> is NULL", pstData->m_ullUin, dwSkinId);
        return ERR_NOT_FOUND;
    }

    int iPrice = pResSkin->m_dwPrice * pResSkin->m_bDiscount / 100;
    if (!Item::Instance().IsEnough(pstData, ITEM_TYPE_DIAMOND, 0, iPrice))
    {
        LOGERR("Player<%lu> Buy Skin<%u> has not enough diamand<%u>",
                    pstData->m_ullUin, dwSkinId, pResSkin->m_dwPrice);
        return ERR_NOT_ENOUGH_DIAMOND;
    }

    if (!Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -(iPrice), rstSyncInfo, 0))
    {
        LOGERR("Player<%lu> Buy Skin<%u> Consume Diamand<%u> failed",
                    pstData->m_ullUin, dwSkinId, pResSkin->m_dwPrice);
        return ERR_NOT_ENOUGH_DIAMOND;
    }

    if (!Item::Instance().RewardItem(pstData, ITEM_TYPE_SKIN, dwSkinId, 0, rstSyncInfo, 0))
    {
        LOGERR("Player<%lu> Add Skin<%u> failed",  pstData->m_ullUin, dwSkinId);
        return ERR_SYS;
    }

    if (!Item::Instance().RewardItem(pstData, ITEM_TYPE_MAJESTY_ITEM, pResSkin->m_dwHeadImageId, 1, rstSyncInfo, 0))
    {
        LOGERR("Player<%lu> Buy Skin<%u> Reward HeadImage<%u> failed",
                    pstData->m_ullUin, dwSkinId, pResSkin->m_dwHeadImageId);
        return ERR_SYS;
    }

    return ERR_NONE;
}

int Skin::AddSkin(PlayerData* pstData, DT_ITEM_SKIN& rstSkin)
{
    DT_ROLE_MISC_INFO& rstInfo = pstData->GetMiscInfo();
    if (rstInfo.m_wSkinCnt >= MAX_NUM_SKIN)
    {
        return ERR_DEFAULT;
    }

    RESGENERALSKIN* pResSkin = CGameDataMgr::Instance().GetResGeneralSkinMgr().Find(rstSkin.m_dwId);
    if (!pResSkin)
    {
        return ERR_NOT_FOUND;
    }

    DT_ITEM_SKIN* pstSkin = this->FindSkin(pstData, rstSkin.m_dwId);
    if (pstSkin == NULL)
    {
        size_t nmemb = (size_t)rstInfo.m_wSkinCnt;
        int iRet = MyBInsert(&rstSkin, rstInfo.m_astSkinList, &nmemb, sizeof(DT_ITEM_SKIN), 1, SkinCmp);
        if (iRet != 1)
        {
            return ERR_SYS;
        }
        rstInfo.m_wSkinCnt = (uint16_t)nmemb;
    }
    else
    {
        RESGENERAL* pstGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pResSkin->m_dwGeneralId);
        if (!pstGeneral)
        {
            return ERR_SYS;
        }
        Props::Instance().Add(pstData, pstGeneral->m_dwExchangeId, pResSkin->m_dwFragmentNum, 0);
    }

    return ERR_NONE;
}

int Skin::ChgSkin(PlayerData* pstData, uint32_t dwGeneralId, uint32_t dwSkinId)
{
    if (dwSkinId != 0)
    {
        RESGENERALSKIN* pResSkin = CGameDataMgr::Instance().GetResGeneralSkinMgr().Find(dwSkinId);
        if (!pResSkin)
        {
            LOGERR("Player<%lu> ResSkin<%u> is NULL", pstData->m_ullUin, dwSkinId);
            return ERR_NOT_FOUND;
        }

        if (dwGeneralId != pResSkin->m_dwGeneralId)
        {
            LOGERR("Player<%lu> ResSkin<%u> is not match General<%u>", pstData->m_ullUin, dwSkinId, dwGeneralId);
            return ERR_NOT_SATISFY_COND;
        }

        DT_ROLE_MISC_INFO& rstInfo = pstData->GetMiscInfo();

        DT_ITEM_SKIN stSkin;
        stSkin.m_dwId = dwSkinId;

        int iEqual = 0;
        MyBSearch(&stSkin, rstInfo.m_astSkinList, rstInfo.m_wSkinCnt, sizeof(DT_ITEM_SKIN), &iEqual, SkinCmp);
        if (!iEqual)
        {
            LOGERR("Player<%lu> Skin<%u> not found", pstData->m_ullUin, dwSkinId);
            return ERR_NOT_FOUND;
        }

    }

    DT_ITEM_GCARD* pstGeneralCard = GeneralCard::Instance().Find(pstData, dwGeneralId);
    if (!pstGeneralCard)
    {
        return ERR_NOT_FOUND;
    }

    pstGeneralCard->m_dwSkinId = dwSkinId;

    AsyncPvp::Instance().UptToAsyncSvr(pstData);

    return ERR_NONE;
}

