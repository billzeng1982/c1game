#pragma once
#include "define.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "singleton.h"
#include "./player/PlayerData.h"
#include "./player/Player.h"



//成长奖励  包括	
//					1.等级成长奖励
//					2.在线时间成长奖励
//					3.待设计

#define INDEX_TO_BINARY(X) (1 << ((X)-1))


class GrowthAward : public TSingleton<GrowthAward>
{
public:
	GrowthAward(){};
	virtual ~GrowthAward() {};
public:
	
    //等级成长奖励
    int GrowthAwardLevel(PlayerData* pstData, IN PKGMETA::CS_PKG_GROWTH_AWARD_REQ& rstReq, OUT PKGMETA::SC_PKG_GROWTH_AWARD_RSP& rstRsp);

    //在线时间成长奖励
    int GrowthAwardOnline(PlayerData* pstData, IN PKGMETA::CS_PKG_GROWTH_AWARD_REQ& rstReq, OUT PKGMETA::SC_PKG_GROWTH_AWARD_RSP& rstRsp);

    //购买等级基金
    int BuyLvFund(PlayerData* pstData, IN PKGMETA::CS_PKG_BUY_LV_FUND_REQ& rstReq, OUT PKGMETA::SC_PKG_BUY_LV_FUND_RSP& rstRsp);

    //领取等级基金奖励
    int GetLvFundAward(PlayerData* pstData, IN PKGMETA::CS_PKG_GET_LV_FUND_AWARD_REQ& rstReq, OUT PKGMETA::SC_PKG_GET_LV_FUND_AWARD_RSP& rstRsp);

private:


    
	bool _CheckAward(const uint32_t & rdwValue, int iIndex) { return  (rdwValue & INDEX_TO_BINARY(iIndex)) != 0 ;}				//检查奖励是否已领取, true为已领取
	void _SetAward(uint32_t&  rdwValue, int iIndex )	{ rdwValue = rdwValue | INDEX_TO_BINARY(iIndex) ; }

};
