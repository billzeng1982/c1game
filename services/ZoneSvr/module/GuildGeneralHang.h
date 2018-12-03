#pragma once

#include "define.h"
#include "ov_res_public.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"

using namespace PKGMETA;

class GuildGeneralHang : public TSingleton<GuildGeneralHang>
{
public:
	GuildGeneralHang()	{}
	~GuildGeneralHang()	{}

	bool Init();
	bool InitPlayerData(PlayerData* pstData);
	//bool UpdatePlayerData(PlayerData* pstData);

	//unlock后转为semilock状态，buy后才转为unlocked
	int UnlockLevelSlot(PlayerData* pstData);
	int BuyLevelSlot(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint8_t bIndex);

	int UnlockVipSlot(PlayerData* pstData);
	int BuyVipSlot(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint8_t bIndex);

	int LayGCard(PlayerData* pstData, SC_PKG_GUILD_HANG_LAY_GENERAL_RSP& rstRsp, CS_PKG_GUILD_HANG_LAY_GENERAL_REQ& rstReq);

	int SettleExpAllSlot(PlayerData* pstData, SC_PKG_GUILD_HANG_SETTLE_RSP& rstRsp);

	int AddSpeedExp(PlayerData* pstData, uint8_t bCount);

private:
	uint32_t _SettleExp(PlayerData* pstData, DT_ONE_GUILD_HANG_INFO& rstOneHangInfo);
	void _SettleExpAllSlot(PlayerData* pstData, SC_PKG_GUILD_HANG_SETTLE_RSP* poRsp = NULL);
    void _HangInfoNtf(PlayerData* pstData);

private:
    SCPKG m_stScPkg;

	//加经验间隔时间
	uint32_t m_dwUpdateInterval;
	//加经验数值，与主公等级挂钩
	uint32_t m_dwExpValue[RES_MAX_GENERAL_LV_LEN+1];
	//被加速一次所得经验值，与主公等级挂钩
	uint32_t m_dwExpSpeed[RES_MAX_GENERAL_LV_LEN+1];
	//vip栏位个数
	uint16_t m_wVipVacancy[MAX_VIP_LEVEL+1];
	//等级栏位个数
	uint16_t m_wLevelVacancy[RES_MAX_GENERAL_LV_LEN+1];
	//等级栏位价格
	int m_iLevelPrice[GUILD_GENERAL_HANG_LEVEL_NUM];
	//vip栏位价格
	int m_iVipPrice[GUILD_GENERAL_HANG_VIP_NUM];
};











