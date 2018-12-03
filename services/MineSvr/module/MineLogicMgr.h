#pragma once

#include <vector>
#include <map>
#include <set>
#include <sys/time.h>
#include "singleton.h"
#include "../gamedata/GameDataMgr.h"
#include "ss_proto.h"
#include "../cfg/MineSvrCfgDesc.h"
#include "MineUidSet.h"
using namespace PKGMETA;

#define UIN_BATCH_COUNT 20
struct CoSettleArgs
{

	uint8_t m_bCount;
	uint64_t m_UinList[UIN_BATCH_COUNT];
};

class MineLogicMgr : public TSingleton<MineLogicMgr>
{
public:
	static const uint32_t BASIC_MINE_NORMAL_ORE_GEN_RULE = 9310;
	static const uint32_t BASIC_MINE_SUPPER_ORE_GEN_RULE = 9311;
    static const uint32_t BASIC_MINE_MINING_BEGIN_TIME = 9312;
    static const uint32_t BASIC_MINE_MINING_END_TIME = 9313;
    static const uint32_t BASIC_MINE_GUARD_TIME = 9303;
    static const uint32_t BASIC_MINE_LOOT_PARA = 9307;
	static const uint32_t BASIC_MINE_DROP_RATE = 9309;
	static const uint32_t BASIC_MINE_SEASON_TIME = 9314;
public:
    typedef std::vector<uint32_t>  ResMineArry_t;
    typedef std::map<uint8_t, ResMineArry_t> ResMineTypeToArry_t;

public:
    MineLogicMgr ();
    ~MineLogicMgr ();
    
	//	初始化
    bool Init(MINESVRCFG* pstConfig);

	//	初始化配置档数据
    bool LoadGameData();

	//	更新
    void Update(bool bIdle);
	
	//创建矿
	int CreateOre();

	void Fini();

	//	获取信息
    int GetInfo(uint64_t ullUin, char* szName, uint32_t dwAddr, uint16_t wLv, uint16_t wIconId, uint32_t dwLeaderValue, OUT PKGMETA::SS_PKG_MINE_GET_INFO_RSP& rstRsp);

    //  探索
    int Explore(uint64_t ullUin, uint32_t dwTeamLi, OUT PKGMETA::SS_PKG_MINE_EXPLORE_RSP& rstRsp);

    //  占领矿
    int Occupy(uint64_t ullUin, PKGMETA::SS_PKG_MINE_DEAL_ORE_REQ& rstReq, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo);

    //  放弃矿
    int Drop(uint64_t ullUin, uint64_t ullOreUid, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo);

    //  修改矿
    int Modify(uint64_t ullUin, uint64_t ullOreUid, PKGMETA::DT_ITEM_MSKILL& rstMSkill, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo);

    //  调查领奖
    int Investigate(uint64_t ullUin, uint64_t ullOreUid, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo);

    //  挑战请求
    int ChallengeRequest(uint64_t ullUin, uint64_t ullOreUid, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo);

	//	复仇请求
	int RevengeRequest(uint64_t ullUin, uint64_t ullOreUid, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo);

    //  挑战
    int Challenge(uint64_t ullUin, uint64_t ullOreUid, uint8_t bIsWin);

    //  抢占矿
    int Grab(uint64_t ullUin, PKGMETA::SS_PKG_MINE_DEAL_ORE_REQ& rstReq, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo);

	//  放弃抢占矿
	int GiveUpGrab(uint64_t ullUin, PKGMETA::SS_PKG_MINE_DEAL_ORE_REQ& rstReq, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo);

    //  获取奖励
    int GetAwardLog(uint64_t ullUin, uint8_t bType, uint8_t bIndex, OUT PKGMETA::SS_PKG_MINE_GET_AWARD_RSP& rstRsp);

	//	复仇
	int Revenge(uint64_t ullUin, uint64_t ullObjUin, uint64_t ullObjUid, uint8_t bIsWin, uint8_t bRevengeLogIndex);

    void SendNtfToClient(uint8_t bNtfType, SSPKG& rstPkg);

	//	当前矿的数量是否充分
	void UpdateOreNum();

private:
	int													m_iDelayCheckOreSec;			//延迟检测矿数量的时间(秒)
    ResMineTypeToArry_t                                 m_MineTypeToArryMap;
    std::set<uint64_t>                                  m_CurrSeasonUinSet;            //当前活动周期的玩家集合

	CoSettleArgs										m_CoSettleArgs;
	MineUidSet											m_oMineUinSet;
	bool												m_bIsSettle;					//是否结算
	bool												m_bIsClear;						//是否赛季清理
	bool												m_bIsCoDoFinished;				//是否赛季清理
	timeval												m_stBeginTv;
	uint64_t											m_ullLastUpdateOreNum;			//上一次更新矿的时间
public:
	int													m_iInitNormalOreNum;			//初始化低级矿数量
	int													m_iInitSupperOreNum;			//初始化高级矿数量
	float												m_fNormalEmptyRate;				//无人低级矿占比
	float												m_fSupperEmptyRate;				//无人高级矿占比
	int													m_iNormalOreAddNum;				//低级矿增量增加数量
	int													m_iSupperOreAddNum;				//高级矿增量增加数量
	uint32_t											m_dwDailyStartSecOfDay;			//相对于每天凌晨多少秒
	uint32_t											m_dwDailyEndSecOfDay;			//相对于每天凌晨多少秒
	uint32_t											m_dwDailySettleAwardSecOfDay;			//相对于每天凌晨多少秒 发奖时间
	uint32_t											m_dwSelectOccupyOutTime;		//高级矿选择占领过期时间 秒
    uint64_t											m_ullNowDailyStartTime;			//当前每天功能开放开始时间
	uint64_t											m_ullNowDailyEndTime;			//当前每天功能开放结束时间

    uint64_t                                            m_dwChallengeGuardTime;         //被挑战获得的护盾时间
    uint64_t                                            m_dwOccupyOreGuardTime;			//占领高级矿获得的护盾时间
    float                                               m_fLootRate;                    //抢夺比率
	float												m_fLootMaxCycleNum;             //抢夺最大资源（N个周期的产出），
    uint32_t                                            m_dwBeLootCount;                //最多被抢次数
	float												m_fLootLowCycleNum;				//被抢次数达到上限后 抢到的资源（N个周期的产出）
    uint64_t                                            m_ullDailySettleAwardTime;      //当前每日奖励结算时间戳
	float												m_fDropRate;					//放弃矿获得的比例
	uint32_t											m_dwDailyMaxProduceTime;		//每天最大产出时间


	uint64_t											m_ullSeasonEndTime;				//赛季开放时间  
	uint64_t											m_ullSeasonStartTime;			//赛季结束时间
	uint8_t												m_bSeasonStartWeekDay;			//赛季开始的周几
	uint8_t												m_bSeasonEndWeekDay;			//赛季结束的周几



};


