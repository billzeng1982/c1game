#pragma once
#include "define.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include "player/Player.h"
class Gem : public TSingleton<Gem>
{
public:
    static const uint8_t CONST_DOWN_TYPE_ONE = 0;   //只卸载一颗宝石
    static const uint8_t CONST_DOWN_TYPE_ALL = 1;   //卸载全部宝石
public:
    Gem(){};
    virtual ~Gem() {};
public:
    //void AutoOpenSlot(PlayerData* pstData, DT_ITEM_GCARD* pstGCard);    //自动开第一个孔
public:
    int Up(PlayerData* pstData, IN CS_PKG_GEM_UP_REQ& rstReq);
    int Down(PlayerData* pstData, IN CS_PKG_GEM_DOWN_REQ& rstReq) ;
	int Synthetic(PlayerData* pstData, IN CS_PKG_GEM_SYNTHETIC_REQ& rstReq, OUT SC_PKG_GEM_SYNTHETIC_RSP& rstRsp);

	//修复 宝石
	int Repair(PlayerData* pstData);

private:
    bool _CheckSlotValid(uint8_t bSlot) {return bSlot >= 0 && bSlot < MAX_GEM_SLOT_NUM;}
	void _RepairDown(PlayerData* pstData, DT_ITEM_GCARD& rstGCard, int iSlotIndex);
    //  镶嵌一颗宝石
    int _UpOneGem(PlayerData* pstData, DT_ITEM_GCARD* pstGCard , uint8_t bSlot, uint32_t dwGemSeq);
   
    //  卸载一颗宝石
    int _DownOneGem(PlayerData* pstData, DT_ITEM_GCARD* pstGCard, uint8_t bSlotIndex);
private:
    DT_ITEM m_stTmpItem; //感觉不需要给前台
};
