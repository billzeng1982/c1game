#pragma once
#include "../../common/define.h"
#include "cs_proto.h"
#include "ss_proto.h"
#include "../../common/utils/singleton.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../gamedata/GameDataMgr.h"
#include "player/Player.h"
#include "player/PlayerMgr.h"
#include "player/PlayerData.h"
#include <map>
#include <set>
#include "Lottery.h"
#include "ShopSlot.h"

using namespace std;
using namespace PKGMETA;


class Shops : public TSingleton<Shops>
{
public:
	Shops(){};
	virtual ~Shops(){};
	bool Init();
public:
    bool InitCompositeShop();
    void RefreshCompositeShop(PlayerData* pstData, DT_CSHOP_INFO& rstShopInfo, uint8_t bShopType);
    int ResetShop(PlayerData* pstData, CS_PKG_COIN_SHOP_UPDATE_REQ& rstReq, SC_PKG_COIN_SHOP_UPDATE_RSP& rstRsp, DT_CSHOP_INFO& rstShopInfo);
	void InitPlayerData(PlayerData* pstData);
	void UpdatePlayerData(PlayerData* pstData);
	int UpdateServer();
	int PurChase(PlayerData* pstData, CS_PKG_COIN_SHOP_PURCHASE_REQ& rstCsPkgReq, SC_PKG_COIN_SHOP_PURCHASE_RSP& rstScPkgRsp);
	int SynchOrResetShop(PlayerData* pstData, CS_PKG_COIN_SHOP_UPDATE_REQ& rstReq, SC_PKG_COIN_SHOP_UPDATE_RSP& rstRsp);

	//获取指定商店数据
	DT_CSHOP_INFO* GetPlayerShopData(PlayerData* pstData, uint8_t bShopType);

	//或者指定商店log字符串
	const char * GetShopTypeString(uint8_t bShopType);
private:
	bool _InitShops();
	void _LoadGoods(PlayerData* pstData, SC_PKG_COIN_SHOP_UPDATE_RSP& rstRsp);
	void _LoadOneShopGoodS(DT_CSHOP_INFO& rstShopInfoSrc, DT_CSHOP_INFO& rstShopInfoDes);
	bool _InitPara();
private:
	SSPKG m_stSsPkg;
	SCPKG m_stScPkg;
	RandomNode m_stRandomNode;

    ShopSlot m_oShopSlot;

	//代币商店刷新相关变量
	int m_iMarketUptTime1;
	int m_iMarketUptTime2;
	int m_iMarketUptTime3;
	int m_iMarketUptTime4;
	bool m_bMarketUptFlag;
	uint64_t m_ullMarketLastUptTime;

	int m_iResetTimesUptTime;
	bool m_bResetTimesUptFlag;
	uint64_t m_ullResetTimesLastUptTime;  //用于重置购买次数
	
};