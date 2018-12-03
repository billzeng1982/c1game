#include "GloryItemsMgr.h"
#include "LogMacros.h"
#include "../gamedata/GameDataMgr.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "./player/Player.h"
#include "MasterSkill.h"
#include "AP.h"
#include "Item.h"
#include "Lottery.h"
#include "Task.h"
#include "ActivityMgr.h"
#include "ov_res_public.h"
#include "dwlog_svr.h"
#include "ZoneLog.h"
#include "Guild.h"
#include "GeneralCard.h"


GloryItemsMgr::GloryItemsMgr()
{

}

bool GloryItemsMgr::Init()
{
	return true;
}

int GloryItemsMgr::AddMajestyItems(PlayerData* pstData, uint32_t dwId)
{
    RESMAJESTYITEM* poResMajestyItem = CGameDataMgr::Instance().GetResMajestyItemMgr().Find(dwId);
    if (!poResMajestyItem)
    {
        return ERR_NOT_FOUND;
    }

    return this->_AddMajestyItems(pstData, poResMajestyItem);
}

int GloryItemsMgr::AddMajestyItems(PlayerData* pstData, uint32_t dwApproach, uint32_t dwPara)
{
	SC_PKG_MAJESTY_ITEM_NTF& rstScPkgBody = m_stScPkg.m_stBody.m_stMajestyItemNtf;
	rstScPkgBody.m_stSyncItemInfo.m_bSyncItemCount = 0;
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MAJESTY_ITEM_NTF;
	DT_ITEM& rstItemInfo = rstScPkgBody.m_stSyncItemInfo.m_astSyncItemList[rstScPkgBody.m_stSyncItemInfo.m_bSyncItemCount];
	rstItemInfo.m_bItemType = ITEM_TYPE_MAJESTY_ITEM;

	ResMajestyItemMgr_t& rResMajestyItemMgr = CGameDataMgr::Instance().GetResMajestyItemMgr();
	RESMAJESTYITEM* poResMajestyItem = NULL;
	int iResSum = rResMajestyItemMgr.GetResNum();

	for (int i = 0; i < iResSum; i++)
	{
		poResMajestyItem = rResMajestyItemMgr.GetResByPos(i);

		if (NULL == poResMajestyItem)
		{
			continue;
		}

		if (poResMajestyItem->m_bAccessType == dwApproach)
		{
			if (poResMajestyItem->m_bType == MAJESTY_ITEM_HEAD_FRAME)
			{
				//头像框比较id参数
				if (poResMajestyItem->m_dwId == dwPara)
				{
					_AddMajestyItems(pstData, poResMajestyItem);
					break;
				}
			}
			else
			{
				// 大于等于类型
				if((uint32_t)MAJESTY_ITEM_ACCESS_GOLD_HIT == dwApproach
					|| (uint32_t)MAJESTY_ITEM_ACCESS_AP == dwApproach
					|| (uint32_t)MAJESTY_ITEM_ACCESS_GUILD_PVP == dwApproach)
				{
					if (poResMajestyItem->m_dwAccessParam <= dwPara)
					{
						rstItemInfo.m_dwItemId = poResMajestyItem->m_dwId;
						rstScPkgBody.m_stSyncItemInfo.m_bSyncItemCount++;
						_AddMajestyItems(pstData, poResMajestyItem);
						break;
					}
				}
				else
				{
					// 等于类型
					if (poResMajestyItem->m_dwAccessParam == dwPara)
					{
						rstItemInfo.m_dwItemId = poResMajestyItem->m_dwId;
						rstScPkgBody.m_stSyncItemInfo.m_bSyncItemCount++;
						_AddMajestyItems(pstData, poResMajestyItem);
						break;
					}
				}
			}
		}

	}

	if (rstScPkgBody.m_stSyncItemInfo.m_bSyncItemCount != 0)
	{
		ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
	}

	return 0;
}


int GloryItemsMgr::_AddMajestyItems(PlayerData* pstData, RESMAJESTYITEM* poResMajestyItem)
{
	DT_ROLE_ITEMS_INFO& rstRoleItemsInfo = pstData->GetItemsInfo();

	int idx = 0;
	for (; idx < rstRoleItemsInfo.m_iCount; idx++)
	{
		if (rstRoleItemsInfo.m_astData[idx].m_dwId == poResMajestyItem->m_dwId)
		{
			break;
		}
	}

	if (idx == rstRoleItemsInfo.m_iCount)
	{
		rstRoleItemsInfo.m_iCount++;
	}

	rstRoleItemsInfo.m_astData[idx].m_dwId = poResMajestyItem->m_dwId;
	uint64_t ullTimeStamp = poResMajestyItem->m_dwExpiryTime;

	if (ullTimeStamp != 0)
	{
		ullTimeStamp += CGameTime::Instance().GetCurrSecond();
	}

	rstRoleItemsInfo.m_astData[idx].m_ullValidTime = ullTimeStamp;

	return 0;
}