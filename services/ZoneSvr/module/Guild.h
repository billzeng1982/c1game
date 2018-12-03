#pragma once
#include "define.h"
#include "cs_proto.h"
#include "ss_proto.h"
#include "singleton.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../gamedata/GameDataMgr.h"
#include "player/Player.h"
#include "player/PlayerMgr.h"
#include "player/PlayerData.h"
#include <map>
#include "Lottery.h"

using namespace std;
using namespace PKGMETA;


typedef struct
{
    uint8_t bLevel;
    RESGUILDTASK* pstResGuildTask;
}TaskNode;

struct BossSection
{
    uint32_t m_dwLow;
    uint32_t m_dwHight;
    uint32_t m_dwResId;
};

struct BossSectionCmp
{
    BossSectionCmp(uint32_t key):m_dwkey(key) {}
    bool operator()(const BossSection& mc)
    {
        return mc.m_dwLow <= m_dwkey && m_dwkey <= mc.m_dwHight;
    }
private:
    uint32_t m_dwkey;
};


class Guild : public TSingleton<Guild>
{
public:
    Guild(){};
    virtual ~Guild(){};
    bool Init();
    bool LoadBossGameData();
public:
    int CheckCreateGuild(PlayerData* pstData);
    int CheckJoinGuild(PlayerData* pstData);
    void UpdateGuidContribution(PlayerData* pstData);
    void AfterCreateGuild(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo);
    void RefreshMemberInfo(PlayerData* pstData, uint8_t bIsOnline = 1);
    void InitMemberExtraInfo(PlayerData* pstData, DT_GUILD_MEMBER_EXTRA_INFO& rstGuildMemberExtraInfo, uint8_t bIsOnline);
    void InitMemberInfo(PlayerData* pstData, DT_ONE_GUILD_MEMBER& rstGuildMemberInfo);
    int AddGuildContribution(PlayerData* pstData, int iNum, uint32_t dwApproach);
    int AddGuidFund(PlayerData* pstData, int iNum, uint8_t bType, uint8_t bDonateType);
	int AddGuildVitality(PlayerData* pstData, int iNum, uint8_t bDonateType);

    int UpdateServer();
    void UpdatePlayerData(PlayerData* pstData);
    void InitPlayerData(PlayerData* pstData);

    int ResetMarketOrTask(PlayerData* pstData, CS_PKG_GUILD_RESET_MARKET_OR_TASK_REQ& rstReq, SC_PKG_GUILD_RESET_MARKET_OR_TASK_RSP& rstRsp);     //商店或人物刷新
    int SettleGuilTask(PlayerData* pstData, CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsSettle, SC_PKG_FIGHT_PVE_SETTLE_RSP& rSettleRsp);
    int SettleBoss(PlayerData* pstData, CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsSettle, SC_PKG_FIGHT_PVE_SETTLE_RSP& rSettleRsp);
    ////军团商店购买
    //int PurChase(PlayerData* pstData, CS_PKG_GUILD_MARKET_PURCHASE_REQ& rstCsPkgReq, SC_PKG_GUILD_MARKET_PURCHASE_RSP& rstScPkgRsp);
    //军团捐赠
    int Donate(PlayerData* pstData, CS_PKG_GUILD_DONATE_REQ& rstCsPkgReq, SC_PKG_GUILD_DONATE_RSP& rstScPkgRsp);
    //领取军团活动奖励
    int ActivityRecvReward(PlayerData* pstData, uint32_t dwId, SC_PKG_GUILD_ACTIVITY_REWARD_RSP& rstScPkgRsp);

    void GetGuildFightApplyList(uint64_t ullGuildId, SC_PKG_GUILD_FIGHT_GET_APPLY_LIST_RSP& rstGetApplyListRsp);
    void SetGuildFightApplyList(DT_GUILD_FIGHT_APPLY_LIST_INFO& rstGuildFightApplyList);

    void GetGuildFightAgainstInfo(DT_GUILD_FIGHT_AGAINST_INFO& rstGuildFightAgainstInfo);
    void SetGuildFightAgainstInfo(DT_GUILD_FIGHT_AGAINST_INFO& rstGuildFightAgainstInfo);

    //**************Boss
    void GuildBossSettleUpdate(PlayerData* pstData, DT_GUILD_PKG_UPDATE_GUILD_BOSS& rstUpdateGuildBoss);
    void GuildBossGetKilledAward(PlayerData* pstData);
    void GuildBossReset(PlayerData* pstData);
    bool CheckBossFightNum(PlayerData* pstData, uint8_t bNum);
    void SendBossNtfMsg(PlayerData* pstData, DT_GUILD_BOSS_INFO& m_stBossInfo);
    int CheckFightBoss(PlayerData* pstData, uint32_t m_dwLevelId);
    int SendGuildBossCompetitorInfo(DT_GUILD_BOSS_COMPETITOR_INFO& m_stCompetitorInfo);
    void GuildBossGetSingleReward(PlayerData* pstData, uint8_t bBossId);
    uint8_t m_szGuildBossAwardList[MAX_GUILD_BOSS_NUM];
    //uint8_t m_szGUildBossFightTimes[MAX_GUILD_MEMBER_MAX_NUM];

	//检查战斗
	int ExpeditionFightRequest(PlayerData* pstData);
	
	

	//设置远征的阵容
	int SetExpeditionArray(PlayerData* pstData, CS_PKG_GUILD_EXPEDITION_SET_BATTLE_ARRAY_REQ& rstReq);

	//上传阵容信息
	int UploadDefendFightInfo(PlayerData* pstData);

	//处理战斗结果
	int ExpeditionFightResult(PlayerData* pstData, CS_PKG_GUILD_EXPEDITION_FIGHT_RESULT_REQ& rstCSReq);

	//战斗发奖
	int SettleExpeditionFight(PlayerData* pstData, SS_PKG_GUILD_EXPEDITION_FIGHT_RESULT_RSP& rstRsp, DT_SYNC_ITEM_INFO& rstSyncInfo);

	//获取玩家远征角色信息
	void GetRoleExpeditionInfo(PlayerData* pstData, OUT DT_ROLE_GUILD_EXPEDITION_INFO& rstExpeditionInfo);

	//开始匹配
	int ExpeditionMatch(PlayerData* pstData);

	bool IsExpeditionOpen();

	//军团俸禄
	int DrawSalary(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncInfo,  uint8_t bSalaryIdentity);

	void UpdateSocietyBuffs(DT_ROLE_GUILD_INFO& rstGuildInfo, DT_GUILD_PKG_UPDATE_GUILD_SOCIETY& rstUpdateGuildSociety);
private:
    bool _InitGuildTask();
    //bool _InitGuildMarket();
    bool _InitPara();
    bool _InitGuildDonate();
    bool _InitGuildBoss();
    void SendTaskNtfMsg(PlayerData* pstData);
    //void SendMarketNtfMsg(PlayerData* pstData);
    int _ResetTask(PlayerData* pstData, CS_PKG_GUILD_RESET_MARKET_OR_TASK_REQ& rstReq, SC_PKG_GUILD_RESET_MARKET_OR_TASK_RSP& rstRsp);
    //int _ResetMarket(PlayerData* pstData, CS_PKG_GUILD_RESET_MARKET_OR_TASK_REQ& rstReq, SC_PKG_GUILD_RESET_MARKET_OR_TASK_RSP& rstRsp);
    RESGUILDTASK* GetTaskResByLevel(uint8_t bLevel);
    void GenerateLevelIdList(PlayerData* pstData);
    //void RefreshMarket(PlayerData* pstData);

    int _GoldDonate(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncInfo, uint8_t bType);
    int _DiamandDonate(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncInfo, uint8_t bType);
	int _VipDonate(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncInfo, uint8_t bType);

private:
    SSPKG m_stSsPkg;
    SCPKG m_stScPkg;
    RandomNode m_stRandomNode;



    uint16_t m_wCreateLowestLv;      //允许创建军团的最低等级
    uint16_t m_wFreeCreateLowestLv;  //免费创建军团的最低等级
    uint32_t m_dwJoinGuildVal;       //允许再次加入或者创建的间隔
    uint32_t m_dwQuitKeepRatio;      //退出后保留百分比
    uint16_t m_wCreateConsume;       //创建公会的消耗
    uint8_t m_bMarketInitNum;        //军团商店初始物品数

    //军团任务刷新相关变量
    int m_iTaskUptTime;
    bool m_bTaskUptFlag;
    uint64_t m_ullTaskLastUptTime;

    //军团练兵场普通栏位个数
    uint8_t m_bLevelSlotNum;
    //军团练兵场vip栏位个数
    uint8_t m_bVipSlotNum;

    //军团捐赠相关
    int m_iDonateMaxTimes;

    int m_iGoldDonateConsume;
    int m_iGoldDonateFund;
    int m_iGoldDonateContribute;
    int m_iGoldDonateVitality;

    int m_iDiamandDonateConsume;
    int m_iDiamandDonateFund;
    int m_iDiamandDonateContribute;
    int m_iDiamandDonateVitality;

	int m_iVipDonateConsume;
	int m_iVipDonateFund;
	int m_iVipDonateContribute;
	int m_iVipDonateVitality;

	int m_iExpeditionMaxFightNum;	//远征战斗最大次数
	int m_iExpeditionRecoverSec;	//远征恢复战斗次数的时间(秒)
	int m_iExpeditionServerOpenDay;	//远征开服几天后开启
	int m_iExpeditionWinDamage;		//胜利伤害
	int m_iExpeditionLoseDamage;	//失败伤害

    //军团任务相关
    map<uint32_t, uint32_t> m_LevelToRewardMap;
    map<uint32_t, uint32_t> ::iterator m_LevelToRewardMapIter;
    uint8_t   m_TaskNodeCount;
    TaskNode  m_TaskNodeList[MAX_GUILD_TASKNODE_NUM];

    uint8_t   RandomList[MAX_GUILD_LEVELS_NUM];

    map<uint32_t, RandomNodeList> m_Type2BonusMap;
    map<uint32_t, RandomNodeList>::iterator m_Type2BonusMapIter;

    //军团战报名列表
    DT_GUILD_FIGHT_APPLY_LIST_INFO  m_stGuildFightApplyList;

    //军团战对阵表
    DT_GUILD_FIGHT_AGAINST_INFO m_stGuildFightAgainstInfo;

    //军团Boss
    int m_iGuildBossFightNumUptHour;
    uint64_t m_ullGuildBossFightNumUptTime;
    bool m_bGuildBossFightNumUpteFlag;
    uint32_t m_BossHp[MAX_GUILD_BOSS_NUM];
    std::map<uint32_t, uint32_t> m_FLevelToBossIdMap;   // <副本ID, BossId>
    list<BossSection> m_oBossRewardMap[MAX_GUILD_BOSS_NUM];
};

