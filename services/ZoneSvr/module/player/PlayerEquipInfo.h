#pragma once

#include "common_proto.h"
#include "DynMempool.h"
#include <map>

using namespace PKGMETA;
using namespace std;


class PlayerEquipInfo
{
private:
    static const int EQUIPPOOL_INIT_NUM = 50;
    static const int EQUIPPOOL_DELTA_NUM = 50;

public:
    PlayerEquipInfo(){}
    virtual ~PlayerEquipInfo(){}
    void Init( int iMaxNum = MAX_NUM_ROLE_EQUIP );
    bool PackEquipInfo(DT_ROLE_EQUIP_BLOB& rstBlob, uint16_t wVersion = 0);
    bool InitFromDB(DT_ROLE_EQUIP_BLOB& rstBlob, uint16_t wVersion);
    DT_ITEM_EQUIP* Find(uint32_t dwSeq);
    bool Add(DT_ITEM_EQUIP& rstEquip);
    bool Del(uint32_t dwSeq);
    bool IsNeedUpdate() { return m_bUptFlag; }
    void ResetUptFlag() { m_bUptFlag = false; }
    void SetUptFlag() { m_bUptFlag = true; }
	void Clear();

public:
    int m_iCount;
    int m_iMaxNum;
    bool m_bUptFlag;
    map<uint32_t, DT_ITEM_EQUIP*> m_oSeqToEquipMap;
    map<uint32_t, DT_ITEM_EQUIP*>::iterator m_oSeqToEquipMapIter;
};


