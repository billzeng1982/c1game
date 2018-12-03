#pragma once
#include "singleton.h"
#include "cs_proto.h"
#include "ss_proto.h"
#include "common_proto.h"
#include "player/Player.h"
#include "GeneralCard.h"
#include "../gamedata/GameDataMgr.h"
#include "../cfg/ZoneSvrCfgDesc.h"
using namespace PKGMETA;

class LevelRank : public TSingleton<LevelRank>
{
public:
    LevelRank();
    virtual ~LevelRank();
    bool Init(ZONESVRCFG* p_stConfig);
    bool Fini();
    // void MakeFakeData();

    void UpdateRecord(PlayerData* pstData, CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsPkgBodyReq);
    int GetRecord(CS_PKG_FIGHT_LEVEL_RECORD_REQ& rstCsPkgBodyReq, SC_PKG_FIGHT_LEVEL_RECORD_RSP& rstScPkgBodyRsp);
	void SetRecordBlob(SS_PKG_LEVEL_RECORD_READ_RSP& rstLevelRecordRsp);
	void UpdateServer();

private:
    void _SetNextRecommendIndex(DT_FIGHT_LEVEL_RECORD_BIG_ITEM& stItem);
    void _UpdateFastRecord(DT_FIGHT_LEVEL_RECORD_BIG_ITEM& stItem, CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsPkgBodyReq, DT_ROLE_MAJESTY_INFO& rstPlayerInfo);
    void _UpdateRecommendRecord(DT_FIGHT_LEVEL_RECORD_BIG_ITEM& stItem, CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsPkgBodyReq, DT_ROLE_MAJESTY_INFO& rstPlayerInfo);
    bool _GetRecordDataFromDB();
    bool _SetRecordDataToDB();


public:
    DT_FIGHT_LEVEL_RECORD_INFO m_stRecordInfo;
    uint8_t m_bRecommendIndex; //表示下一条将被替换的推荐阵容
    bool m_bIsUpdated;
    uint32_t m_dwEfficiency;//战力
	time_t m_ullLastTm;//最近更新时间戳
	bool m_bIsUpdateServer;//是否定时回写数据库
	ZONESVRCFG* m_pstConfig;

private:
	PKGMETA::SSPKG 	m_stSsPkg;
};