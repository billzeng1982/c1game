#pragma once

#include "singleton.h"
#include "common_proto.h"

using namespace PKGMETA;

class AsyncPvpPlayer
{
public:
    //初始化玩家
    bool InitFromDB(DT_ASYNC_PVP_PLAYER_WHOLE_DATA& rstPlayerData);

    //清除数据
    void Clear();

    //获取所有数据
    void GetWholeData(DT_ASYNC_PVP_PLAYER_WHOLE_DATA& rstPlayerData);

    //获得展示数据
    void GetShowData(DT_ASYNC_PVP_PLAYER_SHOW_DATA& rstShowData);

    //更新展示数据
    void UptShowData(DT_ASYNC_PVP_PLAYER_SHOW_DATA& rstShowData);

    //获得基本数据
    void GetBaseData(DT_ASYNC_PVP_PLAYER_BASE_INFO& rstBaseInfo);

    //刷新对手
    void RefreshOpponentList();

    //获取对手(得到的不是对手的具体信息，而是对手的Rank)
    uint8_t GetOpponentList(uint32_t astOpponentList[]);

    //检查对手是否在自己的列表中
    bool CheckOpponent(uint32_t dwOpponent);

    //增加战斗记录
    int AddRecord(DT_ASYNC_PVP_FIGHT_RECORD& rstRecord);

    //获取战斗记录
    uint8_t GetRecordList(DT_ASYNC_PVP_FIGHT_RECORD astRecordList[]);

    //是否在战斗中
    bool IsInFight() { return m_bInFight; }

    //设置战斗状态
    void SetInFight(bool bInFight) { m_bInFight = bInFight; }

    //获取排位
    uint32_t GetRank() { return m_stShowData.m_stBaseInfo.m_dwRank; }

    //设置排位
    void SetRank(uint32_t dwRank) { m_stShowData.m_stBaseInfo.m_dwRank = dwRank; }

    //得到Uin
    uint64_t GetPlayerId() { return m_stShowData.m_stBaseInfo.m_ullUin; }

	//被膜拜所得金币
	void AddWorshipGold(int32_t iWorshippedGoldOnce, int32_t iWorshippedGoldMax);

	//获取被膜拜所得金币
	uint32_t GetWorshipGold();

	void ClearWorshipGold()	{m_iGoldWorshipped = 0;}

private:
    //删除战斗记录
    void _DelRecord();

private:
    DT_ASYNC_PVP_PLAYER_SHOW_DATA m_stShowData;

    //对手数
    uint8_t m_bOpponentCount;

    //对手列表
    uint32_t m_OpponentList[MAX_NUM_ASYNC_PVP_OPPONENT_MYSQL];

    //录像数
    uint8_t m_bRecordCount;

    //录像列表
    uint64_t m_RecordList[MAX_NUM_ASYNC_PVP_RECORD];

    //是否在战斗中
    bool m_bInFight;

	//未领取的被膜拜所得金币
	int32_t m_iGoldWorshipped;

	////当日已领取的膜拜金币数目
	//int32_t m_iGoldTakenWorshipped;
};
