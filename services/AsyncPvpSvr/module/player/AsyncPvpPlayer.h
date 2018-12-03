#pragma once

#include "singleton.h"
#include "common_proto.h"

using namespace PKGMETA;

class AsyncPvpPlayer
{
public:
    //��ʼ�����
    bool InitFromDB(DT_ASYNC_PVP_PLAYER_WHOLE_DATA& rstPlayerData);

    //�������
    void Clear();

    //��ȡ��������
    void GetWholeData(DT_ASYNC_PVP_PLAYER_WHOLE_DATA& rstPlayerData);

    //���չʾ����
    void GetShowData(DT_ASYNC_PVP_PLAYER_SHOW_DATA& rstShowData);

    //����չʾ����
    void UptShowData(DT_ASYNC_PVP_PLAYER_SHOW_DATA& rstShowData);

    //��û�������
    void GetBaseData(DT_ASYNC_PVP_PLAYER_BASE_INFO& rstBaseInfo);

    //ˢ�¶���
    void RefreshOpponentList();

    //��ȡ����(�õ��Ĳ��Ƕ��ֵľ�����Ϣ�����Ƕ��ֵ�Rank)
    uint8_t GetOpponentList(uint32_t astOpponentList[]);

    //�������Ƿ����Լ����б���
    bool CheckOpponent(uint32_t dwOpponent);

    //����ս����¼
    int AddRecord(DT_ASYNC_PVP_FIGHT_RECORD& rstRecord);

    //��ȡս����¼
    uint8_t GetRecordList(DT_ASYNC_PVP_FIGHT_RECORD astRecordList[]);

    //�Ƿ���ս����
    bool IsInFight() { return m_bInFight; }

    //����ս��״̬
    void SetInFight(bool bInFight) { m_bInFight = bInFight; }

    //��ȡ��λ
    uint32_t GetRank() { return m_stShowData.m_stBaseInfo.m_dwRank; }

    //������λ
    void SetRank(uint32_t dwRank) { m_stShowData.m_stBaseInfo.m_dwRank = dwRank; }

    //�õ�Uin
    uint64_t GetPlayerId() { return m_stShowData.m_stBaseInfo.m_ullUin; }

	//��Ĥ�����ý��
	void AddWorshipGold(int32_t iWorshippedGoldOnce, int32_t iWorshippedGoldMax);

	//��ȡ��Ĥ�����ý��
	uint32_t GetWorshipGold();

	void ClearWorshipGold()	{m_iGoldWorshipped = 0;}

private:
    //ɾ��ս����¼
    void _DelRecord();

private:
    DT_ASYNC_PVP_PLAYER_SHOW_DATA m_stShowData;

    //������
    uint8_t m_bOpponentCount;

    //�����б�
    uint32_t m_OpponentList[MAX_NUM_ASYNC_PVP_OPPONENT_MYSQL];

    //¼����
    uint8_t m_bRecordCount;

    //¼���б�
    uint64_t m_RecordList[MAX_NUM_ASYNC_PVP_RECORD];

    //�Ƿ���ս����
    bool m_bInFight;

	//δ��ȡ�ı�Ĥ�����ý��
	int32_t m_iGoldWorshipped;

	////��������ȡ��Ĥ�ݽ����Ŀ
	//int32_t m_iGoldTakenWorshipped;
};
