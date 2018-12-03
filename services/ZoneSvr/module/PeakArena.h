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

	//�����λ�仯(bIsWin:0�䣬1Ӯ��2ƽ)
	void HandleELOSettle(PlayerData* pstData, uint8_t bIsWin);

	//�����ֽ���
	void HandleRewardSettle(PlayerData* pstData, uint8_t bIsWin, DT_SYNC_ITEM_INFO& rstSyncItemInfo);

	//��ȡ��Ծ�Ƚ���
	int RecvActiveReward(PlayerData* pstData, CS_PKG_PEAK_ARENA_GET_ACTIVE_REWARD_REQ& rstReq, SC_PKG_PEAK_ARENA_GET_ACTIVE_REWARD_RSP& rstRsp);

	//��ȡ����
	int RecvOutput(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo);

	//��ȡ����
	int BuyTimes(PlayerData* pstData, uint8_t bTimes, DT_SYNC_ITEM_INFO& rstSyncItemInfo);

	//��ʯ���ٲ���
	int SpeedUpOutput(PlayerData* pstData, SC_PKG_PEAK_ARENA_SPEED_UP_OUTPUT_RSP& rstRsp);

	//��������
	void UpdateRank(PlayerData* pstData);

	//��ȡ��ǰ�Ĺ���id
	uint32_t GetRuleId();

	// ��Ծ�Ȳ���buff�ӳɸĳ�ֱ�����ã������ۼӵķ�ʽ, ����ת��������
	void AdaptActiveRewardBuff( PlayerData* pstData );

private:
	bool _InitBasic();
	bool _InitScore();
	bool _InitSeasonAndRule();

	//���������б�������������㷨��֤��ͬ���ϲ����Ĺ�����������ͬ��
	//��������ʼʱ����Ϊ����㷨����ʼ������ӣ�������֤��ͬ���ϲ������������������ͬ��
	void _GenRuleList();

	void _ResetPlayerData(PlayerData* pstData);

	void _AddOutput(PlayerData* pstData, uint16_t wNum);

	int _RecvOneReward(PlayerData* pstData, uint32_t dwId, SC_PKG_PEAK_ARENA_GET_ACTIVE_REWARD_RSP& rstRsp);

	void _SettleSeason();

	// ͨ����Ծֵ��ȡ�۷�Ļ�Ծ��������
	RESPEAKARENAACTIVEREWARD* _GetActiveRewardByActiveVal(uint16_t wActiveValue);

private:
	MapScore2Id_t m_oScore2IdMap;

	//��ʤ������Ҫ����ʤ��
	uint8_t m_bStreakTimes;

	//ÿ�յ��ֽ��㽱������
	uint8_t m_bSettleRewardLimit;
	uint8_t m_bRewardTimesBuyLimit;
	uint8_t m_bRewardTimesBuyCost;

	//ÿ�յ��ֽ�����ˢ��ʱ��
	int m_iUpdateTime;
	uint64_t m_ullLastUptTimestamp;
	bool m_bUptFlag;

	//�������
	uint16_t m_wOutputInterval;

	//��������Ʒ�����ID
	uint8_t m_OutputType[MAX_PEAK_ARENA_OUTPUT_NUM];
	uint32_t m_OutputId[MAX_PEAK_ARENA_OUTPUT_NUM];

	//������ʼʱ��ͽ���ʱ��
	uint64_t m_ullSeasonStartTime;
	uint64_t m_ullSeasonEndTime;

	//���������·�
	uint8_t m_bSeasonLastMonth;

	//����
	uint8_t m_RuleList[];

	//��ʯ���������ز���
	uint64_t m_ullSpeedUpLastTime;
	uint32_t m_dwSpeedUpDiamand;
	uint32_t m_dwSpeedUpOutputNum[MAX_PEAK_ARENA_OUTPUT_NUM];
};
