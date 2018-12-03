#include "ShopSlot.h"




bool ShopSlot::Init()
{
    
    bzero(m_abShopSlotNum, sizeof(m_abShopSlotNum));
	return true;
}

bool ShopSlot::InitSlot(uint8_t bShopType, uint8_t bSlotType, uint16_t mSlotLv, RandomNodeList& rstRdNodeList, bool bIsAddSlotNum)
{
    if (bIsAddSlotNum)
    {
        m_abShopSlotNum[bShopType]++;
        if (m_abShopSlotNum[bShopType] > MAX_SHOP_SLOT_NUM)
        {
            LOGERR("m_abShopSlotNum[bShopType<%d>]<%d> is larger than MAX_SHOP_SLOT_NUM", bShopType, m_abShopSlotNum[bShopType]);
            return false;
        }
    }

    uint32_t dwKey = _GenKey(bShopType, bSlotType, mSlotLv);

    pair< map<uint32_t, RandomNodeList>::iterator, bool > stRet = m_CompositeKey2RandomListMap.insert(map<uint32_t, RandomNodeList>::value_type(dwKey, rstRdNodeList));

    return stRet.second;
}

RandomNodeList* ShopSlot::GetRandomList(uint8_t bShopType, uint8_t bSlotType, uint16_t wRoleLevel)
{
    if (m_CompositeKey2RandomListMap.empty())
    {
        return NULL;
    }
    uint32_t dwKey = _GenKey(bShopType, bSlotType, wRoleLevel);

    m_CompositeKey2RandomListMapIter = m_CompositeKey2RandomListMap.lower_bound(dwKey);

    //1. 返回end，没有比它大的key，需要--，才能取值 
    //2. 找打一个RetKey，但与目标key不相等，需要--
    //3. RetKey == Key 直接得到结果
    if (m_CompositeKey2RandomListMapIter == m_CompositeKey2RandomListMap.end() || m_CompositeKey2RandomListMapIter->first != dwKey)
    {
        m_CompositeKey2RandomListMapIter--;
        return &m_CompositeKey2RandomListMapIter->second;
    }
    else if (m_CompositeKey2RandomListMapIter->first == dwKey)
    {
        return &m_CompositeKey2RandomListMapIter->second;
    }
    else
    {
		LOGERR("RandomList not found. dwKey<%d>, bShopType<%d> bSlotType<%d> wRoleLevel<%d>",
			dwKey, bShopType, bSlotType, wRoleLevel);
		return NULL;
    
    }
}

uint8_t ShopSlot::GetShopSlotNum(uint8_t bShopType)
{
    return m_abShopSlotNum[bShopType];
}

uint32_t ShopSlot::_GenKey(uint8_t bShopType, uint8_t bSlotType, uint16_t mSlotLv)
{
    uint32_t dwKey = 0;

    uint32_t dwTmp = bShopType;
    dwKey += (dwTmp << 24);

    dwTmp = bSlotType;
    dwKey += (dwTmp << 16);

    dwTmp = mSlotLv;
    dwKey += (dwTmp << 8);

    //LOGRUN("bShopType<%d>, bSlotType<%d>, mSlotLv<%d>, dwKey<%d>",bShopType, bSlotType, mSlotLv, dwKey);

    return dwKey;
}


