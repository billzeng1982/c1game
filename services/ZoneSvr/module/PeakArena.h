#pragma once

#include "singleton.h"
#include "define.h"
#include "cs_proto.h"
#include "Player.h"
#include <map>

using namespace std;
using namespace PKGMETA;

#define MAX_PEAK_ARENA_SCORE 999999

class PeakArena : public TSingleton<PeakArena>
{
public:
	struct ScoreNode
	{
		uint32_t m_dwLow;
		uint32_t m_dwHigh;

		bool operator < (const ScoreNode& a) const
		{
			return m_dwHigh < a.m_dwLow;
		}
	};

	typedef map<ScoreNode, uint32_t> MapScore2Id_t;

public:
	PeakArena() {};
	~PeakArena() {};

	bool Init();

	void UpdateServer();

	void UpdatePlayerData(PlayerData* pstData);

	//处理段位变化(bIsWin:0输，1赢，2平)
	void HandleELOSettle(PlayerData* pstData, uint8_t bIsWin);

	//处理单局奖励
	void HandleRewardSettle(PlayerData* pstData, uint8_t bIsWin, DT_SYNC_ITEM_INFO& rstSyncItemInfo);

	//领取活跃度奖励
	int RecvActiveReward(PlayerData* pstData, CS_PKG_PEAK_ARENA_GET_ACTIVE_REWARD_REQ& rstReq, SC_PKG_PEAK_ARENA_GET_ACTIVE_REWARD_RSP& rstRsp);

	//领取产出
	int RecvOutput(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo);

	//领取产出
	int BuyTimes(PlayerData* pstData, uint8_t bTimes, DT_SYNC_ITEM_INFO& rstSyncItemInfo);

	//钻石加速产出
	int SpeedUpOutput(PlayerData* pstData, SC_PKG_PEAK_ARENA_SPEED_UP_OUTPUT_RSP& rstRsp);

	//更新排名
	void UpdateRank(PlayerData* pstData);

	//获取当前的规则id
	uint32_t GetRuleId();

	// 活跃度产出buff加成改成直接配置，不再累加的方式, 适配转换老数据
	void AdaptActiveRewardBuff( PlayerData* pstData );

private:
	bool _InitBasic();
	bool _InitScore();
	bool _InitSeasonAndRule();

	//产生规则列表，随机产生，由算法保证不同服上产生的规则序列是相同的
	//用赛季开始时间作为随机算法的起始随机种子，这样保证不同服上产生的随机数序列是相同的
	void _GenRuleList();

	void _ResetPlayerData(PlayerData* pstData);

	void _AddOutput(PlayerData* pstData, uint16_t wNum);

	int _RecvOneReward(PlayerData* pstData, uint32_t dwId, SC_PKG_PEAK_ARENA_GET_ACTIVE_REWARD_RSP& rstRsp);

	void _SettleSeason();

	// 通过活跃值获取巅峰的活跃奖励配置
	RESPEAKARENAACTIVEREWARD* _GetActiveRewardByActiveVal(uint16_t wActiveValue);

private:
	MapScore2Id_t m_oScore2IdMap;

	//连胜奖励需要的连胜数
	uint8_t m_bStreakTimes;

	//每日单局结算奖励上限
	uint8_t m_bSettleRewardLimit;
	uint8_t m_bRewardTimesBuyLimit;
	uint8_t m_bRewardTimesBuyCost;

	//每日单局奖励的刷新时间
	int m_iUpdateTime;
	uint64_t m_ullLastUptTimestamp;
	bool m_bUptFlag;

	//产出间隔
	uint16_t m_wOutputInterval;

	//产出的物品种类和ID
	uint8_t m_OutputType[MAX_PEAK_ARENA_OUTPUT_NUM];
	uint32_t m_OutputId[MAX_PEAK_ARENA_OUTPUT_NUM];

	//赛季开始时间和结束时间
	uint64_t m_ullSeasonStartTime;
	uint64_t m_ullSeasonEndTime;

	//赛季持续月份
	uint8_t m_bSeasonLastMonth;

	//规则
	uint8_t m_RuleList[];

	//钻石购买加速相关参数
	uint64_t m_ullSpeedUpLastTime;
	uint32_t m_dwSpeedUpDiamand;
	uint32_t m_dwSpeedUpOutputNum[MAX_PEAK_ARENA_OUTPUT_NUM];
};
