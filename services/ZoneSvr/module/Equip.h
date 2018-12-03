#pragma once
#include "define.h"
#include "common_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include "cs_proto.h"
#include "../gamedata/GameDataMgr.h"
#include <map>
using namespace PKGMETA;
using namespace std;

//装备分类：1-武器，2-饰品/兵书，3-坐骑，4-帽子，5-衣服，6-鞋子
typedef struct
{
    DT_ITEM_EQUIP* pstEquip;
    RESEQUIP* pstResEquip;
    int iReqExp;
    int iGainExp;
}EQUIP_EXP_INFO;

class Equip : public TSingleton<Equip>
{
public:
	Equip();
	virtual ~Equip(){}

private:
	DT_ITEM_EQUIP m_stEquip;
    uint32_t m_ExpCardId[MAX_NUM_EQUIP_EXPCARD_TYPE];
    uint32_t m_ExpCardExp[MAX_NUM_EQUIP_EXPCARD_TYPE];
    //uint32_t m_ExpCardNum[MAX_NUM_EQUIP_EXPCARD_TYPE];
    //map<uint32_t,uint32_t> m_PropUsed;

public:
	bool Init();
	int Add(PlayerData* pstData, DT_ITEM_EQUIP* pstItem);
	int Del(PlayerData* pstData, uint32_t dwSeq);
	DT_ITEM_EQUIP* Find(PlayerData* pstData, uint32_t dwSeq);

	int LvUp(PlayerData* pstData, CS_PKG_EQUIP_LV_UP_REQ& rstEquipLvUpReq, SC_PKG_EQUIP_LV_UP_RSP& rstEquipLvUpRsp);
    //int TotalLvUp(PlayerData* pstData, uint32_t dwGeneralId, DT_SYNC_ITEM_INFO& rstSyncItemInfo);
    int PhaseUp(PlayerData* pstData, uint32_t dwSeq, SC_PKG_EQUIP_PHASE_UP_RSP& rstEquipPhaseUpRsp);
	int PhaseUpTotal(PlayerData* pstData, uint32_t GeneralId, SC_PKG_EQUIP_PHASE_UP_RSP& rstEquipPhaseUpRsp);
    int UpStar(PlayerData* pstData, uint32_t dwSeq, DT_SYNC_ITEM_INFO& rstSyncItemInfo);
    int Recycle(PlayerData* pstData, uint32_t dwGeneralId, OUT DT_SYNC_ITEM_INFO& rstRewardItemInfo, OUT uint32_t* dwGoldUsed, uint32_t dwRatio);
    int _Recycle(PlayerData* pstData, uint32_t dwSeq, OUT uint32_t& dwGoldUsed, OUT map<uint32_t,uint32_t>& PropUsed, OUT uint32_t& dwTotalExp);   //装备回退

private:
    //double _GetUpStarProbability(uint8_t BaseClassScore, uint8_t EatClassScore, uint8_t BaseStar, uint8_t EatStar);
    //int _CalEquipLv(DT_ITEM_EQUIP* pstEquip, RESEQUIP* pstResEquip, uint32_t dwGainExp);
    //void _CalConsume(PlayerData* pstData, int iTotalExp, int iRestExp, DT_SYNC_ITEM_INFO& rstSyncItemInfo);
    //void _TotalLvUp(EQUIP_EXP_INFO* pstEquipList, int iTotalExp, int* pRestExp, int iNum, int* pRestNum);
};


