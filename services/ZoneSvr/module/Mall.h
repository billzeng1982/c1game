#pragma once

#include <map>
#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include "../gamedata/GameDataMgr.h"

using namespace PKGMETA;

class Mall : public TSingleton<Mall> 
{
public:
    Mall() {};
    virtual ~Mall() {};
    bool Init();
    // 服务器更新
    void UpdateServer();
    void UpdatePlayerData(PlayerData* pstData);
    void UpdateFromGameData();
    void SendNtf(PlayerData* pstData);
    int Buy(PlayerData* pstData, CS_PKG_MALL_BUY_REQ& rstReq, SC_PKG_MALL_BUY_RSP& rstRsp); //购买接口
    int GiveFriendGift(PlayerData* pstData,  CS_PKG_MALL_BUY_REQ& rstReq, SC_PKG_MALL_BUY_RSP& rstRsp);

    int HandleMsgSendMail(PlayerData* pstData, uint64_t FriendUin, uint16_t wNum, uint32_t dwPropId, RESPRIMAIL* poResPriMail);
private:
    //  检查购买等级
    bool _CheckLvLimit(PlayerData* pstData, RESMALLPROPS* poResMallProp);
    //  物品是否上架,主要为了Gm控制
    bool _IsGoodsUp(uint32_t dwId) { return (m_GoodsBuyTag[dwId] == 1); }    
    uint8_t _GetGoodsDiscount(RESMALLPROPS* poResMallProp);
    int _IsDiscountTime(uint32_t dwTimeId);
    int _IsBuyTime(uint32_t dwTimeId);
    int _IsBuyNumLimit(DT_MALL_GOODS_BUY_INFO* poGoodsBuyInfo, DT_MALL_INFO& rstMallInfo, RESMALLPROPS* poResMallProps);
    void _AddBuyNum(DT_MALL_GOODS_BUY_INFO* poGoodsBuyInfo, uint16_t wBuyNum, DT_MALL_INFO& rstMallInfo, RESMALLPROPS* poResMallProps);
    DT_MALL_GOODS_BUY_INFO* _FindGoodsBuyInfo(RESMALLPROPS* poResMallProp, DT_MALL_INFO& rstMallInfo);
  

private:
    SCPKG m_stScPkg;
    SSPKG m_stSsPkg;
    map<uint32_t, uint8_t> m_GoodsBuyTag;    //商品是否下架, 1上架,0下架
    uint64_t m_ullDayUpdateLastTime;        //每天刷新时间
    int m_iUptTime;
};
