#pragma once

#include "define.h"
#include "singleton.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "player/PlayerData.h"

using namespace PKGMETA;

class RankMgr : public TSingleton<RankMgr>
{
    static const uint32_t CONST_RANK_LI_UPDATE_TIME = 1800;
public:
    RankMgr() {};
    bool Init();
    int Update();
    void UpdatePlayerData(PlayerData* pstData);

    //  更新武将数量排行榜
    int UpdateGCardCnt(PlayerData* pstData);

    //  更新关卡星星排行榜
    int UpdatePveStar(PlayerData* pstData);

    //  更新战力排行榜
    int UpdateLi(PlayerData* pstData);

    //  更新武将战力排行榜
    int UpdateGCardLi(PlayerData* pstData, uint32_t dwId, bool bIsAdd = true);
    int UpdateGCardLi(PlayerData * pstData, DT_ITEM_GCARD * pstGeneral, bool bIsAdd = true);

    void SetGCardLiRankLastValue(uint32_t dwValue) { m_dwGCardLiRankLastValue = dwValue; }
    void SetGCardLiRankNum(uint32_t dwNum) { m_dwGCardLiRankNum = dwNum; }

	uint32_t GetGCardLiRankLastValue() { return m_dwGCardLiRankLastValue; }
private:

    //  填充排行其他信息
    void _PadOtherRankInfo(PlayerData * pstData, OUT DT_RANK_ROLE_INFO& rstRankInfo);
private:
    uint32_t m_dwRankLimitLow;
    uint64_t m_ullLastUpdateTime;
    SSPKG m_stSsPkg;
    uint32_t m_dwRankCount;
    uint32_t m_dwGCardLiRankNum;            //武将战力排行榜上榜人数
    uint32_t m_dwGCardLiRankLastValue;    //武将战力排行榜最后一名的战力
    uint32_t m_dwMaxNum;
    DT_RANK_ROLE_INFO m_astRankRoleList[MAX_RANK_TOP_NUM];
};

