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

    //��ʼ��TopList
    void InitTopList();

    //��������
    bool SwapRank(AsyncPvpPlayer* poPlayerLeft, AsyncPvpPlayer* poPlayerRight);

    //ͨ����һ�ȡ��Ӧ������
    uint32_t GetRankByPlayer(uint64_t ullUin);

    //ͨ��������ȡ��Ӧ�����
    uint64_t GetPlayerByRank(uint32_t dwRank);

    //��ȡ���а�ǰN����ҵ�����
    uint8_t GetTop10List(DT_ASYNC_PVP_PLAYER_SHOW_DATA astOpponentList[]);

    //��ȡ�������
    uint32_t GetRankMax() { return m_iCount; }

    //����������
    void SendRankReward(bool bUseBak = false);

    //���а����ʱ��֪ͨ��ZoneSvr
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

    //��������ز���
    int m_iDirtyNodeMax;
    int m_iWriteTimeVal;

    //ÿ�ս��㷢������ʱ��
    int m_iSettleTime;
    bool m_bSettleFlag;

    //���Id��Ӧ������
    Id2RankMap_t::iterator m_oIter;
    Id2RankMap_t m_oRankMap;

    //���а�
    bool m_bNtfFlag;
    DT_ASYNC_PVP_PLAYER_SHOW_DATA m_astTopList[MAX_NUM_ASYNC_PVP_TOP_LIST];
};
