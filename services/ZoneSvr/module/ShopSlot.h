#pragma once
#include "define.h"
#include "Lottery.h"
#include <map>
#include <set>

using namespace std;
using namespace PKGMETA;

#define MAX_COIN_SHOP_NUM (10)
//֧�ֵĵȼ���λ��������
#define MAX_SLOT_LEVEL_NUN (90)

class ShopSlot
{
public:
    bool Init();

    bool InitSlot(uint8_t bShopType, uint8_t bSlotType, uint16_t mSlotLv, RandomNodeList& rstRdNodeList, bool bIsAddSlotNum);

    RandomNodeList* GetRandomList(uint8_t bShopType, uint8_t bSlotType, uint16_t wRoleLevel);

	//��ȡ�̵�չʾλ����
    uint8_t GetShopSlotNum(uint8_t bShopType);

private:
	//������ƷKey
    uint32_t _GenKey(uint8_t bShopType, uint8_t bSlotType, uint16_t mSlotLv);

private:

    map<uint32_t, RandomNodeList> m_CompositeKey2RandomListMap;
    map<uint32_t, RandomNodeList>::iterator m_CompositeKey2RandomListMapIter;


    uint8_t m_abShopSlotNum[MAX_COIN_SHOP_NUM + 1];
};