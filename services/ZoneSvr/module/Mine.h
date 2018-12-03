#pragma once

#include "define.h"
#include "ss_proto.h"
#include "singleton.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../gamedata/GameDataMgr.h"
#include "player/PlayerMgr.h"


class Mine : public TSingleton<Mine>
{
public:
    static const uint32_t BASIC_MINE_EXPLORE_PARA = 9304;
    static const uint32_t BASIC_MINE_CHALLENGE_PARA = 9305;
    static const uint32_t BASIC_MINE_REVENGE_PARA = 9306;
    static const uint32_t BASIC_MINE_MINING_BEGIN_TIME = 9312;
    static const uint32_t BASIC_MINE_MINING_END_TIME = 9313;
	static const uint32_t BASIC_MINE_SEASON  = 9314;
    static const uint32_t BASIC_MINE_GCARD_AP = 9308;
public:
    Mine() {};
    ~Mine() {};
    
    //  初始化
    bool Init();

    void UpdateServer();

    //  读取数据党
    bool LoadGameData();
    
    //  功能是否开启
    bool IsOpen();

    void UpdatePlayer(PlayerData& rstData);

    //  检查探索次数是否有效
    bool IsExploreCountEnough(DT_ROLE_MINE_INFO& rstMineInfo);

    //  检查挑战次数是否有效
    bool IsChallengeCountEnough(DT_ROLE_MINE_INFO& rstMineInfo);

    //  检查复仇次数是否有效
    bool IsRevengeCountEnough(DT_ROLE_MINE_INFO& rstMineInfo);

    //  检查武将挑战体力是否足够
    bool IsGCardAPEnough(PlayerData& rstData);

    //  获取驻守部队信息
    int GetTroopInfo(PlayerData& rstData, uint8_t bMSId, uint32_t* pdwTroopFormation, OUT PKGMETA::SS_PKG_MINE_DEAL_ORE_REQ& rstReq);

    //  调查矿
    int InvestigateOre(PlayerData& rstData, DT_MINE_ORE_INFO& rstOreInfo, OUT PKGMETA::DT_SYNC_ITEM_INFO& rstSyncItem);


    //  从物资中心领取奖励
    int GetAwardByAwardLog(PlayerData& rstData, uint8_t bAwardCount, DT_MINE_AWARD* pastAwardList, SC_PKG_MINE_GET_AWARD_RSP& rstRsp);


    //  购买探索次数
    int BuyExploreCount(PlayerData& rstData, OUT DT_SYNC_ITEM_INFO& rstSyncTiem);

    //  购买挑战次数
    int BuyChallengeCount(PlayerData& rstData, OUT DT_SYNC_ITEM_INFO& rstSyncTiem);

    //  购买复仇次数
    int BuyRevengeCount(PlayerData& rstData, OUT DT_SYNC_ITEM_INFO& rstSyncTiem);

    //  消耗挑战次数
    void ConsumeChallengeCount(DT_ROLE_MINE_INFO& rstMineInfo) { rstMineInfo.m_bChallengeCount++; }

    //  消耗探索次数
    void ConsumeExploreCount(DT_ROLE_MINE_INFO& rstMineInfo) { rstMineInfo.m_bExploreCount++; }

    //  消耗复仇次数
    void ConsumeRevengeCount(DT_ROLE_MINE_INFO& rstMineInfo) { rstMineInfo.m_bRevengeCount++; }

    //  消耗武将体力
    void ConsumeGCardAP(PlayerData& rstData);


    //  设置武将状态 bUseTag=true#使用|false#没使用
    void SetGCardState(PlayerData& rstData, DT_MINE_ORE_INFO& rstOreInfo, bool bUseTag);

    void SendNtfToClient(uint8_t bNtfType, SC_PKG_MINE_INFO_NTF& rstNtf);

	//	清理武将状态
	void RefreshGCardState(PlayerData& rstData, uint8_t bOreNum, DT_MINE_ORE_INFO* pstOreInfo);
	
	bool IsLvEnough(PlayerData& rstData) { return rstData.GetLv() >= m_dwLimitLv; }

	float GetTeamLiRate() { return m_fTeamLiRate; }
	//	设置占领武将
// 	int SetGCardOccupy(PlayerData& rstData, uint8_t bOreNum, DT_MINE_ORE_INFO* pstOreInfo);
// 
// 	bool IsGCardOccupy(PlayerData& rstData, uint32_t ullGardId);
private:
    uint32_t m_dwExploreCountConsumeId;         //探索次数购买 ConsumeId
    uint32_t m_dwChallengeCountConsumeId;       //挑战次数购买 ConsumeId    
    uint32_t m_dwRevengeCountConsumeId;         //复仇次数购买 ConsumeId

    uint8_t m_bNormalExploreCountMax;
    uint8_t m_bNormalChallengeCountMax;
    uint8_t m_bNormalRevengeCountMax;
    uint32_t m_dwSettleBeinSecOfDay;			//相对于每天凌晨多少秒
    uint32_t m_dwSettleEndSecOfDay;				//相对于每天凌晨多少秒
	

    uint32_t m_dwMaxGCardAP;

    uint32_t m_dwDaySettleTime;    //每天结算点
    uint64_t m_ullCurSettleTime;    //当前结算时间戳

    uint64_t m_ullOpenBeginTime;    //功能开启时间	绝对时间（秒）
    uint64_t m_ullOpenEndTime;      //功能结束时间	绝对时间（秒）

	uint64_t m_ullSeasonEndTime;		//赛季开放时间  
	uint64_t m_ullSeasonStartTime;		//赛季结束时间

	uint32_t m_dwLimitLv;			//等级限制
	float m_fTeamLiRate;		//战力倍数
};

