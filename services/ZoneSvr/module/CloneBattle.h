#pragma once

#include "define.h"
#include "singleton.h"
#include "player/PlayerMgr.h"
#include <map>
#include "../gamedata/GameDataMgr.h"




class CloneBattle : public TSingleton<CloneBattle>
{
private:
    const static int SPECIAL_ATTR_VALUE = 5;
    const static uint32_t CONST_GCARD_SKILL_DETA = 5;
    const static int CONST_RESBASIC_ID_LV_LIMIT = 9000;
    const static int CONST_RESBASIC_ID_BASE_INFO = 9001;
    const static int CONST_RESPRIMAIL_ID_FO_REWARD = 20;     //团队奖励ID
    const static int CONST_RESCOMMONREWARD_ID_TEAM_REWARD = 1004;     //团队奖励ID
public:
    CloneBattle() {}
    ~CloneBattle() {}

    bool Init();
    //玩家登陆时更新数据
    void InitPlayerData(PlayerData& roPData);

    //检查战斗条件
    int CheckFight(PlayerData& roPData);

    //检查加入条件
    int CheckLvLimit(PlayerData& roPData);

    //初始化成员信息
    int InitMatInfo(PlayerData& roPData,  uint32_t dwGCardId, OUT PKGMETA::DT_CLONE_BATTLE_MATE_INFO& rstMateInfo);


    //广播给队友
    void BroadcastMate(PKGMETA::SS_PKG_CLONE_BATTLE_BROADCAST_NTF& rstNtf);

    //发送团队奖励
    void SendTeamReward(PKGMETA::DT_CLONE_BATTLE_TEAM_INFO& rstTeamInfo);

    //系统解散队伍通知
    void HandleSysDissovleTeam(PKGMETA::DT_CLONE_BATTLE_TEAM_INFO& rstTeamInfo);

    //CloneBattle系统信息更新
    void UptSysInfo(PKGMETA::SS_PKG_CLONE_BATTLE_UPT_SYSINFO_NTF& rstNtf);

    //检查挑战券
    bool IsHaveTicket(PlayerData& roPData);

    //消耗挑战券
    int ConsumeTicket(PlayerData& roPData, PKGMETA::DT_SYNC_ITEM_INFO& rstSyncItem);

    //检查是否拥有Boss对应的武将
    bool IsHaveBossGCard(PlayerData& roPData, uint8_t bBossType);

    //开启奖励箱子
    int OpenRewardBox(PlayerData& roPData, uint32_t dwBossId, uint8_t bSelect, PKGMETA::DT_SYNC_ITEM_INFO& rstSyncItem);

    //获取BossId
    uint32_t GetBossId(uint8_t bBossType) { return m_stBossInfo.m_BossId[bBossType]; }

    //获取上次更新时间
    uint64_t GetLastUptTimesMs() { return m_ullLastUptTimeMs; }
private:

    PKGMETA::SSPKG m_stSsPkg;
    PKGMETA::SCPKG m_stScPkg;
    PKGMETA::DT_CLONE_BATTLE_BOSS_INFO  m_stBossInfo;
    uint64_t m_ullLastUptTimeMs;        //克隆战最新刷新时间 由CloneBattleSvr更新   毫秒
    uint32_t m_dwTicketId;              //挑战券ID
    uint32_t m_dwWinNumLimit;           //胜利次数限制
    uint32_t m_dwLvLimit;               //功能等级限制
};
