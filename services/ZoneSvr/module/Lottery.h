#pragma once
#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include <map>

int RandomNodeFindCmp(const void *pstFirst, const void *pstSecond);


struct RandomNode
{
	uint32_t m_dwLowRange;
	uint32_t m_dwHighRange;
	uint32_t m_dwValue;

public:
	void SetRange(uint32_t dwRange)
	{
		this->m_dwLowRange = this->m_dwHighRange = dwRange;
	}
};



struct RandomNodeList
{
	uint32_t m_dwCount;
	RandomNode* m_astArray;
};

class Lottery : public TSingleton<Lottery>
{
public:
    static const float CONST_PROBABILITY_REDUCE_RATE = 0.1;
    static const uint32_t CONST_REAL_MAX_PROBABILITY = 0.01 * MAX_LOTTERY_PROBABILITY * MAX_LOTTERY_PROBABILITY;
	static const int GOLD_TICKET_ID				= 12002;
	static const int DIAMOND_TICKET_ID			= 12001;
	static const int AWAKE_EQUIP_TICKET_ID		= 12000;
    static const int LOTTERY_MAIL_ID = 10208;
public:
    struct Bonus
    {
        uint32_t m_dwResId;     //资源Id
        uint32_t m_dwLotteryId;   //奖票Id
        uint64_t m_ullRate;     //概率
    };
public:
	Lottery();
	virtual ~Lottery();

private:
	std::map<uint32_t, uint32_t> m_stDrawType2PondIdMap;
	std::map<uint32_t, RandomNodeList> m_stPond2LotteryMap;
	std::map<uint32_t, RandomNodeList> m_stLottery2BonusMap;
	RandomNode m_stRandomNode;

	// rule
	//uint32_t m_adwDiamondTenthPondId[MAX_LOTTERY_CNT_NUM];


    uint32_t m_dwDiaDrawPondIdOne[MAX_LOTTERY_CNT_NUM];  //钻石单次累计循环的奖池
    uint32_t m_dwDiaDrawPondIdTen[MAX_LOTTERY_CNT_NUM];  //钻石10连抽 每次抽的奖池
	uint32_t m_dwGoldDrawPondIdTen[MAX_LOTTERY_CNT_NUM]; //金币奖池
    uint32_t m_dwActDrawPondIdTen[MAX_LOTTERY_CNT_NUM]; //活动奖池
    uint32_t m_dwActCommonDrawPondIdTen[MAX_LOTTERY_CNT_NUM]; //活动奖池 通用版
	uint32_t m_dwActFMonthDrawPondIdTen[MAX_LOTTERY_CNT_NUM]; //活动奖池 首月奖池
    uint32_t m_dwAwakeEquipDrawPondIdOne;  //觉醒商店单抽 每次抽的奖池
    uint32_t m_dwAwakeEquipDrawPondIdTen[MAX_LOTTERY_CNT_NUM];  //觉醒商店十连抽 每次抽的奖池
    uint32_t m_dwAwakeEquipDrawPondId;  //觉醒商店十连抽 特殊的奖池
	uint32_t m_dwDrawPondIdTen[MAX_LOTTERY_CNT_NUM];     //乱序后的奖池
    uint32_t m_dwFactMax ;   //MAX_LOTTERY_CNT_NUM 的全排列, 用来打乱奖励
    uint32_t m_dwWebDrawPondIdTen[MAX_LOTTERY_CNT_NUM]; //网页抽奖的所有奖池

    

    map<uint32_t, vector<RandomNode> > m_oPondId2RandomNodeVectMap;
    std::map<uint32_t, uint32_t> m_dwFirstDrawNumToPondIdMap;   //玩家首次抽奖次数对应的特殊奖池, 只会触发一次
	std::map<uint32_t, uint32_t>::iterator m_dwFirstDrawNumToPondIdMapIter;
	std::map<uint32_t, uint32_t> m_stFirstDrawTensToPondIdMap;  //玩家前n次十连抽对应的特殊奖池，只会出发一次
	std::map<uint32_t, uint32_t>::iterator m_stFirstDrawTensToPondIdMapIter;

    std::map<uint32_t, uint32_t> m_stActCntCommonDrawPondFloatMap;  //活动10连 奖浮动奖池映射	 通用
    std::map<uint32_t, uint32_t> m_stActOneCommonDrawPondFloatMap;  //活动单抽 浮动奖池映射	通用

    std::map<uint32_t, uint32_t> m_stActCntDrawPondFloatMap;  //活动10连 奖浮动奖池映射
    std::map<uint32_t, uint32_t> m_stActOneDrawPondFloatMap;  //活动单抽 浮动奖池映射

	std::map<uint32_t, uint32_t> m_stActCntFMonthDrawPondFloatMap;  //活动10连 奖浮动奖池映射 首月
	std::map<uint32_t, uint32_t> m_stActOneFMonthDrawPondFloatMap;  //活动单抽 浮动奖池映射	首月

    std::map<uint32_t, uint32_t> m_stWebLotteryDrawPondFloadMap;//网页抽奖浮动奖池

    std::map<uint32_t, uint32_t>::iterator m_stPondIdMapIter;
    uint32_t m_dwActDrawMaxNumOfGodCard;                   //活动抽奖保底的最大次数,达到这个次数必出神将
	// free condition
    DT_LOTTERY_PRICE m_stPrice;              //价格

    PKGMETA::SSPKG m_stSsPkg;


	uint32_t m_dwGoldFreeCountByDay;
	uint32_t m_dwGoldFreeIntervalSec;
	uint32_t m_dwDiamondFreeInterval1Sec;
	uint32_t m_dwDiamondFreeInterval2Sec;
    uint32_t m_dwAwakeEquipFreeInterval1Sec;
    uint32_t m_dwAwakeEquipFreeInterval2Sec;

	int m_iDailyResetTime;

	bool m_bCheckFree;
	DT_ITEM m_stDefaultItem;				// 默认奖励物品，出错替换使用

	int32_t m_dwDiscount;                      // 每日首抽折扣

    uint32_t m_dwActDarwTimeId;             //活动抽奖开放时间
    uint32_t m_dwActDarwLvLimit;                   //活动抽奖等级限制
    uint32_t m_dwActDarwVipLimit;                  //抽动抽奖Vip限制
	uint32_t m_dwActFMonthDarwActId;		//活动抽奖(魂匣) 首月  活动ID
public:
	bool Init();
	void HandleLotteryInfo(PlayerData* pstData, PKGMETA::SCPKG& rstScPkg);
    int HandleLotteryDrawByNormal(PlayerData* pstData, uint8_t bDrawType, uint8_t bIsFree, SC_PKG_LOTTERY_DRAW_RSP& rstScPkgBodyRsp);

    //觉醒商店抽奖
    int HandleAwakeEquipLottery(PlayerData* pstData, uint8_t bDrawType, uint8_t bIsFree, SC_PKG_LOTTERY_DRAW_RSP& rstScPkgBodyRsp);

    //  活动(魂匣)抽奖
    int HandleLotteryDrawByAct(PlayerData* pstData, uint8_t bDrawType, SC_PKG_LOTTERY_DRAW_RSP& rstScPkgBodyRsp);
    int HandleLotteryDrawByActBase(PlayerData* pstData, uint8_t bDrawType, uint32_t dwDiamond, uint32_t dwDrawCnt,
        std::map<uint32_t, uint32_t>& rFloatMap, uint32_t (&dwActDrawPondIdTen)[MAX_LOTTERY_CNT_NUM], OUT DT_SYNC_ITEM_INFO& rstSyncItemInfo);
	void UpdatePlayerData(PlayerData* pstData);
	int DrawLotteryByPond(PlayerData* pstData, uint32_t dwPondId, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach, bool bIsMerge = false);
    int InitPlayerData(PlayerData* pstData);
	int TestUnit(uint32_t dwPondId, int iTestTimes);

    //  重置玩家保存的 活动抽奖次数等
    void DrawActReset(PlayerData* pstData);

	//	根据抽奖类型获取参数
	int GetParaFromDrawType(uint8_t bDrawType, OUT int* piDrawCount, OUT uint8_t* pbConsumeType, OUT int32_t* pdiConsumeNum, OUT uint32_t* pdwTicketId, OUT uint32_t* pdwConsumeId = 0);
private:
	bool _InitDrawType();


	bool _InitRule();
	bool _InitFreeCond();
    bool _InitNewLottery();
	int _Clear();
    void _RandDiaDrawPond();
	int _CheckEnoughMoney(PlayerData* pstData, uint32_t dwNeedGold, uint32_t dwNeedDiamond);
	int _CheckFreeCond(PlayerData* pstData, uint8_t bDrawType);
	bool _CheckRule(PlayerData* pstData, uint8_t bDrawType, uint32_t& rdwPondId);
	bool _CheckRuleGold(PlayerData* pstData, uint32_t& rdwPondId);
	bool _CheckRuleDiamond(PlayerData* pstData, uint32_t& rdwPondId);
	bool _CheckRuleDiamondCnt(PlayerData* pstData, uint32_t& rdwPondId);
	int _DrawTypeConvPondId(uint32_t dwDrawType);
	int _DrawLotteryTest(uint32_t dwLotteryId);

	int _DrawLottery(PlayerData* pstData, uint32_t dwLotteryId, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach, bool bIsMerge);
	int _DrawLotteryByPond(PlayerData* pstData, uint32_t dwPondId, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach, bool bIsMerge = false);

	void _ChgBonusSequence(DT_SYNC_ITEM_INFO& rstRewardItemInfo);
	bool _CheckConsume(PlayerData* pstData, int iCount, uint8_t bConsumeType, int32_t& riConsumeNum, uint32_t dwTicketId, int32_t& rstTicketNum, uint32_t dwConsumeId = 0);
	void _RandDrawPond(PlayerData* pstData, uint32_t(&rstDrawPondIdTen)[MAX_LOTTERY_CNT_NUM], bool bRuleEffect);

    void _SendLotteryMessage(PlayerData* pstData, uint32_t dwApproach, uint32_t dwGenenralId);

//     //单抽
//     void _LotteryDrawByActOnce(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo);
//     //十连
//     void _LotteryDrawByActBatch(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo);
};

