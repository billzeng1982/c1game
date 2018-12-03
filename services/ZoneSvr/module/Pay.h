#pragma once
#include "define.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include "player/PlayerMgr.h"
#include "../gamedata/GameDataMgr.h"
#include "TimeCycle.h"

using namespace PKGMETA;

#define DISCOUNT_INFO_FILE "discount_info"

class Pay : public TSingleton<Pay>
{
private:
    const static int LI_REWARD_MAIL_ID = 10011;
    const static int DISCOUNT_ACTIVITY_ID = 5;
    const static uint32_t CONST_MONTH_CARD_FOREVER = 2;     //永久卡


public:
    Pay() { m_ullMonthCardDurationSec = MAX_MONTH_CARD_DAILY_NUM * SECONDS_OF_DAY ;};
    ~Pay() {};

    bool Init();

    bool _InitFromFile();

    void _WriteToFile();

    void Fini();

    void UpdateServer();

    void UpdatePlayerData(PlayerData* pstData);

	// 检查月卡，周卡，各种卡并通过邮件发奖励
    void CheckCardReward(PlayerData* pstData);

    //  数据档加载更新赋值
    bool LoadGameData();

    //  充值成功处理
    void DoPayOk(PKGMETA::SS_PKG_SDK_PAY_CB_NTF& rstPayCbNtf);

    //  用于GM手动处理充值失败
    void GmDoPayOk(uint64_t ullUin, uint64_t ullProductID);

    //  首充双倍奖励
    void AwardFirstDoubleAward(PlayerData* pstData, RESPAY* pResPay, OUT SC_PKG_PAY_SYN_NTF& rstSync);

    //  激活首充奖励
    void ActivateFirstPayAward(PlayerData* pstData);

    //  激活首日充值
    void ActivateFDayPayAward(PlayerData* pstData);

    //  领取首充奖励
    void GetFDayPayAward(PlayerData* pstData, CS_PKG_PAY_GET_AWARD_REQ& rstPkgReq);

    //  领取月卡奖励
    void GetMonthCardAward(PlayerData* pstData, CS_PKG_PAY_GET_AWARD_REQ& rstPkgReq);

    //  领取累积充值奖励
    void GetActTotalPayAward(PlayerData* pstData, CS_PKG_PAY_GET_AWARD_REQ& rstPkgReq);

    //领取累计消费奖励
    void GetTotalConsumeAward(PlayerData* pstData, CS_PKG_PAY_GET_AWARD_REQ& rstPkgReq);

    //  领取首日充值奖励
    void GetFirstPayAward(PlayerData* pstData, CS_PKG_PAY_GET_AWARD_REQ& rstPkgReq);

    //  充值活动相关的接口
    void DoActPay(PlayerData* pstData, uint32_t dwDetaValue);

    //获取折扣物品
    void GetDiscountProps(PlayerData* pstData, CS_PKG_DISCOUNT_PROPS_GET_REQ& rstPkgReq, SC_PKG_DISCOUNT_PROPS_GET_RSP& rstPkgRsp);

    //获取折扣物品
    void BuyDiscountProps(PlayerData* pstData, CS_PKG_DISCOUNT_PROPS_BUY_REQ& rstPkgReq, SC_PKG_DISCOUNT_PROPS_BUY_RSP& rstPkgRsp);

    //增加累计消费
    void DoActConsume(PlayerData* pstData, uint32_t dwDetaValue);

    //  判断是否能购买奥特莱斯商品
    bool CheckGiftBag(PlayerData* pstData, uint32_t dwPayId);

    //结算战力排行奖励
    void SettleLiRank();

private:
    //  购买月卡
    void _PayForMonthCard(PlayerData* pstData, RESPAY* pResPay);

    //  购买钻石,普通充值
    void _PayForDiamond(PlayerData* pstData, RESPAY* pResPay);

    //奥特莱斯
    void _PayForGiftBag(PlayerData* pstData, RESPAY* pResPay);

    //  增加累计充值
    void _AddActTotalPay(PlayerData* pstData, uint64_t ullStartTime, uint32_t dwDetaValue);

    //  增加累计充值
    void _AddActTotalConsume(PlayerData* pstData, uint64_t ullStartTime, uint32_t dwDetaValue);

    //结算战力奖励
    void _SettleLiReward(PlayerData* pstData);

    //检查月卡的有效性
    bool _CheckMonthCard(PlayerData* pstData, uint32_t dwMonthCardId);

    //  购买奥特莱斯商品后返回对应的物品
    bool _GiftBagReward(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstItemInfo, uint32_t dwId);

	// 购买月卡，周卡，各种卡后返回对应的物品
    bool _CardReward(PlayerData* pstData, uint32_t dwId);

	//将订单信息发送到talkingdata
	void _SendtoTD(SS_PKG_SDK_PAY_CB_NTF& rstPayCbNtf, RESPAY* pResPay, Player* poPlayer);

private:
    PKGMETA::SCPKG m_stScPkg;
    PKGMETA::SSPKG m_stSsPkg;
    int m_iUptHour;

    uint64_t m_ullMonthCardDurationSec;

    //折扣活动物品列表
    DT_DISCOUNT_PROPS_INFO m_astDiscountPropsList[MAX_DISCOUNT_ACTIVITY_DAY*MAX_DISCOUNT_PROPS_NUM];

    //战力活动奖励区间
    uint8_t m_bSettleFlag;
    uint8_t m_bLiRewardSecCnt;
    uint32_t* m_LiRewardSec;

    FILE* m_fp;
};


