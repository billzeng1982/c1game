#pragma once

#include "define.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"

using namespace PKGMETA;

class SkillPoint : public TSingleton<SkillPoint>
{
public:
    SkillPoint() {}
    ~SkillPoint() {}

    bool Init();
    bool InitPlayerData(PlayerData* pstData); // �ǳ�ʼ����½����
	void UpdatePlayerData(PlayerData* pstData); // ˢ���������

	int PurchaseSP(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo);

public:
    bool IsEnough(PlayerData* pstData, uint16_t wValue);
    uint32_t Add(PlayerData* pstData, int32_t iValue);

private:
    //�ظ����ʱ�䣬����VIP�ȼ�,ʱ��������ͬ
    uint32_t m_dwUpdateInterval[MAX_VIP_LEVEL+1];
	//���ܵ����ޣ�����VIP�ȼ��仯
	uint16_t m_wTopSPLimit[MAX_VIP_LEVEL+1];
	//���ܵ�ɹ������������VIP�ȼ��仯
	uint16_t m_wBuySPLTimes[MAX_VIP_LEVEL+1];
	//����sp����
	int m_iSPGetIndex;
	int m_iDiamondCostIndex;
};
