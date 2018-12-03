#pragma once
#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "ov_res_public.h"
#include "player/PlayerData.h"

using namespace PKGMETA;

struct Type2Id
{
    uint8_t m_bType;
    uint32_t m_dwId;
};

class Tactics : public TSingleton<Tactics>
{
public:
    Tactics() {}
    ~Tactics() {}
    int Init();
public:
    void Add(PlayerData* pstData, CS_PKG_TACTICS_ADD_REQ& rstReq, SC_PKG_TACTICS_ADD_RSP& rstRsp);
    void LvUp(PlayerData* pstData, CS_PKG_TACTICS_LV_UP_REQ& rstReq, SC_PKG_TACTICS_LV_UP_RSP& rstRsp);

    DT_ITEM_TACTICS* GetTacticsItem(PlayerData* pstData, uint8_t bType);
    uint32_t GetTacticsId(PlayerData* pstData, uint8_t bType);

    //当前等级下的最大阵法数目
    uint8_t GetTacticsNum(PlayerData* pstData);

private:
    DT_ITEM_TACTICS m_stTactics;
    RESCONSUME* m_pResConsume;

    uint8_t m_bCount;
    Type2Id m_astType2Id[MAX_TACTICS_NUM];
    Type2Id m_stType2Id;
};