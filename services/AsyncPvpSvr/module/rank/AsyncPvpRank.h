#pragma once

#include "singleton.h"
#include "ss_proto.h"
#include <map>
#include "AsyncPvpPlayer.h"

using namespace std;
using namespace PKGMETA;

class AsyncPvpRank : public TSingleton<AsyncPvpRank>
{
private:
    static const int DAILY_RANK_SETTLE_MAIL1_ID = 10006;
    static const int DAILY_RANK_SETTLE_MAIL2_ID = 10031;
    static const int DAILY_RANK_SETTLE_TIME_ID = 7008;

    typedef map<uint64_t, uint64_t> Id2RankMap_t;

public:
    bool Init();

    void Update();

    void Fini();

    //初始化TopList
    void InitTopList();

    //交换排名
    bool SwapRank(AsyncPvpPlayer* poPlayerLeft, AsyncPvpPlayer* poPlayerRight);

    //通过玩家获取对应的排名
    uint32_t GetRankByPlayer(uint64_t ullUin);

    //通过排名获取对应的玩家
    uint64_t GetPlayerByRank(uint32_t dwRank);

    //获取排行榜前N名玩家的数据
    uint8_t GetTop10List(DT_ASYNC_PVP_PLAYER_SHOW_DATA astOpponentList[]);

    //获取最大排名
    uint32_t GetRankMax() { return m_iCount; }

    //发排名奖励
    void SendRankReward(bool bUseBak = false);

    //排行榜更新时，通知到ZoneSvr
    void SendRefreshTopListNtf();

private:
    void _Resize();

    void _SaveToFile();

	void _SendRankReward(int iSize, uint64_t* pUinList);

	void _BackUpRankFile();

private:
    FILE* m_fp;

    int m_iCount;
    int m_iCapacity;

    uint64_t* m_Rank;

    SSPKG m_stSsPkg;

    //脏数据相关参数
    int m_iDirtyNodeMax;
    int m_iWriteTimeVal;

    //每日结算发奖励的时间
    int m_iSettleTime;
    bool m_bSettleFlag;

    //玩家Id对应的排名
    Id2RankMap_t::iterator m_oIter;
    Id2RankMap_t m_oRankMap;

    //排行榜
    bool m_bNtfFlag;
    DT_ASYNC_PVP_PLAYER_SHOW_DATA m_astTopList[MAX_NUM_ASYNC_PVP_TOP_LIST];
};
