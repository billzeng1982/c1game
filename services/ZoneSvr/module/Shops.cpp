#include "Shops.h"
#include "common_proto.h"
#include "utils/FakeRandom.h"
#include "sys/GameTime.h"
#include "dwlog_def.h"
#include "ZoneLog.h"
#include "dwlog_svr.h"
#include "player/Player.h"
#include "Lottery.h"
#include "Consume.h"
#include "Item.h"



using namespace std;
using namespace PKGMETA;
using namespace DWLOG;

bool Shops::_InitShops()
{
	//初始化时间戳和更新时间
	ResBasicMgr_t& rstResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
	RESBASIC* pUpdateTime = rstResBasicMgr.Find(GUILD_MARKET_REFRESH_TIME);
	if (pUpdateTime == NULL)
	{
		LOGERR("pUpdateTime is NULL");
		return false;
	}
	m_iMarketUptTime1 = (int)(pUpdateTime->m_para[0]);
	m_iMarketUptTime2 = (int)(pUpdateTime->m_para[1]);
	m_iMarketUptTime3 = (int)(pUpdateTime->m_para[2]);
	m_iMarketUptTime4 = (int)(pUpdateTime->m_para[3]);
	int iHour = CGameTime::Instance().GetCurrHour();
	if ((iHour < m_iMarketUptTime1) || (iHour >= m_iMarketUptTime4))
	{
		iHour = m_iMarketUptTime4;
	}
	else if (iHour >= m_iMarketUptTime3)
	{
		iHour = m_iMarketUptTime3;
	}
	else if (iHour >= m_iMarketUptTime2)
	{
		iHour = m_iMarketUptTime2;
	}
	else
	{
		iHour = m_iMarketUptTime1;
	}
	m_ullMarketLastUptTime = CGameTime::Instance().GetSecOfHourInCurrDay(iHour);
	if (CGameTime::Instance().GetCurrHour() < iHour)
	{
		m_ullMarketLastUptTime -= SECONDS_OF_DAY;
	}
	m_bMarketUptFlag = false;

	//刷新次数相关时间戳和更新时间
	pUpdateTime = rstResBasicMgr.Find(COMMON_UPDATE_TIME);
	if (pUpdateTime == NULL)
	{
		LOGERR("pUpdateTime is NULL");
		return false;
	}
	m_iResetTimesUptTime = (int)(pUpdateTime->m_para[0]);
	m_ullResetTimesLastUptTime = CGameTime::Instance().GetSecOfHourInCurrDay(m_iResetTimesUptTime);
    //LOGRUN("m_ullResetTimesLastUptTime<%lu>", m_ullResetTimesLastUptTime);
	if(CGameTime::Instance().GetCurrHour() < m_iResetTimesUptTime)
	{
		m_ullResetTimesLastUptTime -= SECONDS_OF_DAY;
	}
	m_bResetTimesUptFlag = false;

	if(!InitCompositeShop())
	{
		return false;
	}

	return true;
}

bool Shops::_InitPara()
{
    m_oShopSlot.Init();

	return true;
}

bool Shops::Init()
{
	if (!_InitPara())
	{
		LOGERR("Init CoinShop failed");
		return false;
	}
	if (!_InitShops())
	{
		LOGERR("Init CoinShop failed");
		return false;
	}

	return true;
}

void Shops::InitPlayerData(PlayerData* pstData)
{
	DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();

	//更新刷新次数
	if(rstMiscInfo.m_ullShopsUpdateTime < m_ullResetTimesLastUptTime)
	{
        //LOGRUN("rstMiscInfo.m_ullShopsUpdateTime<%lu>, m_ullResetTimesLastUptTime<%lu>", rstMiscInfo.m_ullShopsUpdateTime, m_ullResetTimesLastUptTime);
		rstMiscInfo.m_stGeneralCoinData.m_bRefreshTimes = 0;
		rstMiscInfo.m_stEquipCoinData.m_bRefreshTimes = 0;
		rstMiscInfo.m_stAsyncPVPCoinData.m_bRefreshTimes = 0;
		rstMiscInfo.m_stSyncPVPCoinData.m_bRefreshTimes = 0;
		rstMiscInfo.m_stGuildCoinData.m_bRefreshTimes = 0;
        rstMiscInfo.m_stPeakArenaCoinData.m_bRefreshTimes = 0;
        rstMiscInfo.m_stAwakeCoinData.m_bRefreshTimes = 0;
		rstMiscInfo.m_stExpeditionCoinData.m_bRefreshTimes = 0;
		rstMiscInfo.m_ullShopsUpdateTime = m_ullResetTimesLastUptTime;
	}

	//更新商店物品
	if (rstMiscInfo.m_stGeneralCoinData.m_ullUpdateTime < m_ullMarketLastUptTime)
	{
        //for (int i = COIN_SHOP_BEGIN; i <= COIN_SHOP_END; i++)
        //{
        //    RefreshCompositeShop(pstData, rstMiscInfo.m_stGeneralCoinData, )
        //}
        
        RefreshCompositeShop(pstData, rstMiscInfo.m_stGeneralCoinData, SHOP_TYPE_GENERAL);
        RefreshCompositeShop(pstData, rstMiscInfo.m_stEquipCoinData, SHOP_TYPE_EQUIP);
        RefreshCompositeShop(pstData, rstMiscInfo.m_stSyncPVPCoinData, SHOP_TYPE_SYNCPVP);
        RefreshCompositeShop(pstData, rstMiscInfo.m_stAsyncPVPCoinData, SHOP_TYPE_ASYNCPVP);
        RefreshCompositeShop(pstData, rstMiscInfo.m_stGuildCoinData, SHOP_TYPE_GUILD);
        RefreshCompositeShop(pstData, rstMiscInfo.m_stPeakArenaCoinData, SHOP_TYPE_PEAK_ARENA);
		RefreshCompositeShop(pstData, rstMiscInfo.m_stAwakeCoinData, SHOP_TYPE_AWAKE);
		RefreshCompositeShop(pstData, rstMiscInfo.m_stExpeditionCoinData, SHOP_TYPE_EXPEDITION);

		rstMiscInfo.m_stGeneralCoinData.m_ullUpdateTime = m_ullMarketLastUptTime;
	}

	return;
}

void Shops::UpdatePlayerData(PlayerData* pstData)
{
	InitPlayerData(pstData);
}

int Shops::UpdateServer()
{
	int iHour = CGameTime::Instance().GetCurrHour();

	//商店刷新次数重置
	if(iHour == m_iResetTimesUptTime && m_bResetTimesUptFlag)
	{
		m_bResetTimesUptFlag = false;
		m_ullResetTimesLastUptTime = CGameTime::Instance().GetCurrSecond();
	}
	else if(iHour != m_iResetTimesUptTime)
	{
		m_bResetTimesUptFlag = true;
	}

	//商店商品重置
	if ((iHour == m_iMarketUptTime1 || iHour == m_iMarketUptTime2 || iHour== m_iMarketUptTime3 || iHour== m_iMarketUptTime4)  && m_bMarketUptFlag)
	{
		m_bMarketUptFlag = false;
		m_ullMarketLastUptTime = CGameTime::Instance().GetCurrSecond();
	}
	else if (iHour != m_iMarketUptTime1 && iHour != m_iMarketUptTime2 && iHour != m_iMarketUptTime3 && iHour != m_iMarketUptTime4)
	{
		m_bMarketUptFlag = true;
	}

	return 0;
}

int Shops::SynchOrResetShop(PlayerData* pstData, CS_PKG_COIN_SHOP_UPDATE_REQ& rstReq, SC_PKG_COIN_SHOP_UPDATE_RSP& rstRsp)
{
	if(0 == rstReq.m_bIsSync)
	{
		int iRet = ERR_NONE;
		rstRsp.m_bCoinType = rstReq.m_bCoinType;
		DT_CSHOP_INFO* pstGeneralShop = GetPlayerShopData(pstData, rstReq.m_bCoinType);
		if (!pstGeneralShop)
		{
			LOGERR("Uin<%lu> get player shop data error. ShopType<%d>", pstData->m_ullUin, rstReq.m_bCoinType);
			return ERR_WRONG_PARA;
		}
		iRet = ResetShop(pstData, rstReq, rstRsp, *pstGeneralShop);
		if (iRet == ERR_NONE)
		{
			ZoneLog::Instance().WriteMarketRefreshLog(pstData, GetShopTypeString(rstReq.m_bCoinType));
		}
		_LoadGoods(pstData, rstRsp);
		return iRet;
	}
	else if(1 == rstReq.m_bIsSync)
	{
		rstRsp.m_bCoinType = rstReq.m_bCoinType;
        _LoadGoods(pstData, rstRsp);
		return ERR_NONE;
	}
	else
	{
		return ERR_WRONG_PARA;
	}
}

PKGMETA::DT_CSHOP_INFO* Shops::GetPlayerShopData(PlayerData* pstData, uint8_t bShopType)
{
	DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();

	DT_CSHOP_INFO* pstCoinShopInfo = NULL;
	switch (bShopType)
	{
	case SHOP_TYPE_GENERAL:
		pstCoinShopInfo = &rstMiscInfo.m_stGeneralCoinData;
		break;
	case SHOP_TYPE_EQUIP:
		pstCoinShopInfo = &rstMiscInfo.m_stEquipCoinData;
		break;
	case SHOP_TYPE_SYNCPVP:
		pstCoinShopInfo = &rstMiscInfo.m_stSyncPVPCoinData;
		break;
	case SHOP_TYPE_ASYNCPVP:
		pstCoinShopInfo = &rstMiscInfo.m_stAsyncPVPCoinData;
		break;
	case SHOP_TYPE_GUILD:
		pstCoinShopInfo = &rstMiscInfo.m_stGuildCoinData;
		break;
	case SHOP_TYPE_PEAK_ARENA:
		pstCoinShopInfo = &rstMiscInfo.m_stPeakArenaCoinData;
		break;
	case SHOP_TYPE_AWAKE:
		pstCoinShopInfo = &rstMiscInfo.m_stAwakeCoinData;
		break;
	case SHOP_TYPE_EXPEDITION:
		pstCoinShopInfo = &rstMiscInfo.m_stExpeditionCoinData;
	default:
		break;
	}
	return pstCoinShopInfo;
}

const char * Shops::GetShopTypeString(uint8_t bShopType)
{
	switch (bShopType)
	{
	case SHOP_TYPE_GENERAL:
		return "GeneralShop";
	case SHOP_TYPE_EQUIP:
		return "EquipShop";
	case SHOP_TYPE_SYNCPVP:
		return "SyncPvp";
	case SHOP_TYPE_ASYNCPVP:
		return "AsyncPvp";
	case SHOP_TYPE_GUILD:
		return "Guild";
	case SHOP_TYPE_PEAK_ARENA:
		return "PeakArenaShop";
	case SHOP_TYPE_AWAKE:
		return "AwakeShop";
	case SHOP_TYPE_EXPEDITION:
		return "ExpeditionShop";
	default:
		return "NoneShop";
	}
}

void Shops::_LoadOneShopGoodS(DT_CSHOP_INFO& rstShopInfoDes, DT_CSHOP_INFO& rstShopInfoSrc)
{
	rstShopInfoDes.m_ullUpdateTime = rstShopInfoSrc.m_ullUpdateTime;
	rstShopInfoDes.m_bRefreshTimes = rstShopInfoSrc.m_bRefreshTimes;
	rstShopInfoDes.m_stShopInfo.m_bShowItemCount = rstShopInfoSrc.m_stShopInfo.m_bShowItemCount;
	for (int i=0; i<rstShopInfoDes.m_stShopInfo.m_bShowItemCount; i++)
	{
		rstShopInfoDes.m_stShopInfo.m_astShowItemList[i] = rstShopInfoSrc.m_stShopInfo.m_astShowItemList[i];
	}
}

void Shops::_LoadGoods(PlayerData* pstData, SC_PKG_COIN_SHOP_UPDATE_RSP& rstRsp)
{
	DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();

	_LoadOneShopGoodS(rstRsp.m_stGeneralCoinInfo, rstMiscInfo.m_stGeneralCoinData);
	_LoadOneShopGoodS(rstRsp.m_stEquipCoinInfo, rstMiscInfo.m_stEquipCoinData);
	_LoadOneShopGoodS(rstRsp.m_stSyncPVPCoinInfo, rstMiscInfo.m_stSyncPVPCoinData);
	_LoadOneShopGoodS(rstRsp.m_stAsyncPVPCoinInfo ,rstMiscInfo.m_stAsyncPVPCoinData);
	_LoadOneShopGoodS(rstRsp.m_stGuildCoinInfo, rstMiscInfo.m_stGuildCoinData);
    _LoadOneShopGoodS(rstRsp.m_stPeakArenaCoinInfo, rstMiscInfo.m_stPeakArenaCoinData);
	_LoadOneShopGoodS(rstRsp.m_stAwakeEquipCoinInfo, rstMiscInfo.m_stAwakeCoinData);
	_LoadOneShopGoodS(rstRsp.m_stExpeditionCoinInfo, rstMiscInfo.m_stExpeditionCoinData);

}

int Shops::PurChase(PlayerData* pstData, CS_PKG_COIN_SHOP_PURCHASE_REQ& rstCsPkgReq, SC_PKG_COIN_SHOP_PURCHASE_RSP& rstScPkgRsp)
{
	rstScPkgRsp.m_bItemIndex = rstCsPkgReq.m_bItemIndex;
	rstScPkgRsp.m_stSyncItems.m_bSyncItemCount = 0;

	rstScPkgRsp.m_bCoinType = rstCsPkgReq.m_bCoinType;


	DT_CSHOP_INFO* pstCoinShopInfo = GetPlayerShopData(pstData, rstCsPkgReq.m_bCoinType);
	if (!pstCoinShopInfo)
	{
		return ERR_WRONG_PARA;
	}

	if (rstCsPkgReq.m_ullTimeStamp != pstCoinShopInfo->m_ullUpdateTime)
	{
		return ERR_DEFAULT;
	}

	if (rstCsPkgReq.m_bItemIndex >= pstCoinShopInfo->m_stShopInfo.m_bShowItemCount)
	{
		return ERR_OUT_OF_BOUND;
	}

	DT_ITEM_SHOW& rstItem = pstCoinShopInfo->m_stShopInfo.m_astShowItemList[rstCsPkgReq.m_bItemIndex];
	if ((rstItem.m_bIsAvailable != 1) ||(rstItem.m_bIsReadyPurchased == 1))
	{
		return ERR_ITEM_IS_READY_PURCHASE;
	}

	if (!Consume::Instance().IsEnough(pstData, rstItem.m_bPriceType, rstItem.m_dwPriceValue))
	{
		return ERR_RES_NOT_ENOUGH;
	}

	Item::Instance().ConsumeItem(pstData, rstItem.m_bPriceType, 0, -rstItem.m_dwPriceValue, rstScPkgRsp.m_stSyncItems, METHOD_COIN_SHOP_PURCHAS);
	Item::Instance().RewardItem(pstData, rstItem.m_bItemType, rstItem.m_dwItemId, rstItem.m_dwItemNum, rstScPkgRsp.m_stSyncItems, METHOD_COIN_SHOP_PURCHAS);

	rstItem.m_bIsReadyPurchased = 1;

	//商品购买日志
	ZoneLog::Instance().WriteItemPurchaseLog(pstData, rstItem, METHOD_COIN_SHOP_PURCHAS);

	return ERR_NONE;
}

bool Shops::InitCompositeShop()
{
    //初始化武将代币商店物品表
    ResCoinShopMgr_t& rstResShopMgr= CGameDataMgr::Instance().GetResCoinShopMgr();
	int iSumCount = rstResShopMgr.GetResNum();
	if (!iSumCount)
	{
		LOGERR("the error iSumCount: %d",iSumCount);
		return false;
	}
	uint8_t bTypeId = 1;
	uint16_t wSlotLv = 1;
    uint8_t bShopType = 1;
    uint32_t dwLastRange = 1;
    uint32_t dwIndex = 0;
    for (int i = 0; i < iSumCount; i++)
    {
        RESCOINSHOP* pResShop = rstResShopMgr.GetResByPos(i);
        if (pResShop->m_bTypeId > MAX_SHOP_SLOT_NUM)
        {
            LOGERR("TypeId<%u> out of range<%u>", pResShop->m_bTypeId, MAX_SHOP_SLOT_NUM);
            break;
        }
        if (bTypeId != pResShop->m_bTypeId || wSlotLv != pResShop->m_wSlotLv || bShopType != pResShop->m_bShopType) /*读取到不同TypeId创建新的RandomNodeList*/
        {
            RandomNodeList stNodeList;
            stNodeList.m_dwCount = i -dwIndex;
            stNodeList.m_astArray = new RandomNode[stNodeList.m_dwCount];

            for (int j=dwIndex, k=0; j < i; j++, k++)
            {
                RESCOINSHOP* pResTemp = rstResShopMgr.GetResByPos(j);
                stNodeList.m_astArray[k].m_dwLowRange = dwLastRange;
                stNodeList.m_astArray[k].m_dwHighRange = pResTemp->m_dwRange;
                stNodeList.m_astArray[k].m_dwValue = j;
                dwLastRange = pResTemp->m_dwRange + 1;

                /*LOGRUN("Type2BonusMap insert TypeId-%u, low-%u, high-%u, j=%d,k=%d", bTypeId,
                        stNodeList.m_astArray[k].m_dwLowRange,
                        stNodeList.m_astArray[k].m_dwHighRange, j, k);*/
            }
            bool bIsAddSlotNum = bTypeId != pResShop->m_bTypeId ? true : false;
            bool bRet = m_oShopSlot.InitSlot(bShopType, bTypeId, wSlotLv, stNodeList, bIsAddSlotNum);
            if (bRet == false)
            {
                return false;
            }
            dwLastRange = 1;
            bTypeId = pResShop->m_bTypeId;
			wSlotLv = pResShop->m_wSlotLv;
            bShopType = pResShop->m_bShopType;
            dwIndex = i;
        }
    }

    //最后一组需要特殊处理
    RandomNodeList stNodeList;
    stNodeList.m_dwCount = iSumCount -dwIndex;
    stNodeList.m_astArray = new RandomNode[stNodeList.m_dwCount];
    for (int j=dwIndex, k=0; j < iSumCount; j++, k++)
    {
        RESCOINSHOP* pResTemp = rstResShopMgr.GetResByPos(j);
        stNodeList.m_astArray[k].m_dwLowRange = dwLastRange;
        stNodeList.m_astArray[k].m_dwHighRange = pResTemp->m_dwRange;
        stNodeList.m_astArray[k].m_dwValue = j;
        dwLastRange = pResTemp->m_dwRange + 1;

        /*LOGRUN("Type2BonusMap insert TypeId-%u, low-%u, high-%u", bTypeId,
                stNodeList.m_astArray[k].m_dwLowRange,
                stNodeList.m_astArray[k].m_dwHighRange);*/
    }
    bool bRet = m_oShopSlot.InitSlot(bShopType, bTypeId, wSlotLv, stNodeList, true);
    if (bRet == false)
    {
        return false;
    }

	return true;
}


void Shops::RefreshCompositeShop(PlayerData* pstData, DT_CSHOP_INFO& rstShopInfo, uint8_t bShopType)
{
    ResCoinShopMgr_t& rstResShopMgr= CGameDataMgr::Instance().GetResCoinShopMgr();

    //动态读取商店格子数目
    int iSumCount = rstResShopMgr.GetResNum();
    if (!iSumCount)
    {
        LOGERR("the error iSumCount: %d",iSumCount);
        return;
    }

    RESCOINSHOP* pResShop = NULL;
    uint8_t bShopSlotNum = m_oShopSlot.GetShopSlotNum(bShopType);
    rstShopInfo.m_stShopInfo.m_bShowItemCount = bShopSlotNum;

    for (int i=0; i < bShopSlotNum; i++)
    {
        DT_ITEM_SHOW& rstItem = rstShopInfo.m_stShopInfo.m_astShowItemList[i];

        uint16_t wLevel = pstData->GetMajestyInfo().m_wLevel;

        RandomNodeList* pstNodeList = m_oShopSlot.GetRandomList(bShopType, i+1, wLevel);
        if (pstNodeList == NULL)
        {
            LOGERR("pstNodeList is NULL.");
            return;
        }
        uint32_t dwRandom = CFakeRandom::Instance().Random(1, MAX_LOTTERY_PROBABILITY);
        m_stRandomNode.SetRange(dwRandom);
        int iEqual = 0;
        int iIndex = MyBSearch(&m_stRandomNode, pstNodeList->m_astArray, pstNodeList->m_dwCount, sizeof(RandomNode), &iEqual, RandomNodeFindCmp);
        if (!iEqual)
        {
            LOGERR("TypeId-%u, Random-%u is not found", i, dwRandom);
            continue;
        }
        uint32_t dwLotteryId = pstNodeList->m_astArray[iIndex].m_dwValue;
        pResShop = rstResShopMgr.GetResByPos(dwLotteryId);
        if (pResShop==NULL)
        {
            LOGERR("pResGuildShop is null, LotteryId=%u", dwLotteryId);
            continue;
        }
        rstItem.m_bItemType = pResShop->m_dwPropType;
        rstItem.m_dwItemId = pResShop->m_dwPropId;
        rstItem.m_dwItemNum = pResShop->m_dwPropNum;
        rstItem.m_bPriceType = pResShop->m_dwPropPriceType;
        rstItem.m_dwPriceValue = pResShop->m_dwPropPrice;
        rstItem.m_bIsReadyPurchased = 0;
        rstItem.m_bIsAvailable = 1;
    }
}

int Shops::ResetShop(PlayerData* pstData, CS_PKG_COIN_SHOP_UPDATE_REQ& rstReq, SC_PKG_COIN_SHOP_UPDATE_RSP& rstRsp, DT_CSHOP_INFO& rstShopInfo)
{
    RESCONSUME* pResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(905);
    if (NULL == pResConsume)
    {
        LOGERR("RESCONSUME not find  Uin<%lu>",pstData->m_pOwner->GetUin());
        return ERR_NOT_FOUND;
    }

    //刷新次数根据VIP等级变化
    RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(pstData->GetVIPLv()+1);
    if (rstShopInfo.m_bRefreshTimes >= pResVip->m_dwUpdateGuildShop)
    {
        return ERR_PVE_RESET_TIMES_NOT_ENOUGH;
    }
    uint32_t dwConsumeDia = pResConsume->m_lvList[rstShopInfo.m_bRefreshTimes];
    if (!Consume::Instance().IsEnoughDiamond(pstData, dwConsumeDia))
    {//钻石不足
        return ERR_NOT_ENOUGH_DIAMOND;
    }
    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -dwConsumeDia, rstRsp.m_stSyncItemInfo, DWLOG::METHOD_EQUIP_SHOP_REFRESH);
    rstShopInfo.m_bRefreshTimes++ ;
    RefreshCompositeShop(pstData, rstShopInfo, rstReq.m_bCoinType);
    rstRsp.m_bCoinType = rstReq.m_bCoinType;
    LOGRUN("Player<%lu> reset EquipShop, consume dia<%u>, EquipShopResetCnt<%u> ", pstData->m_pOwner->GetUin(), dwConsumeDia, rstShopInfo.m_bRefreshTimes);
    return ERR_NONE;
}

