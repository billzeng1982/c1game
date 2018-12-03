#pragma once

#include <map>
#include "common_proto.h"
#include "ss_proto.h"

/*
 *  军团Boss战的一些功能
 **/

class GuildBoss
{
public:
    GuildBoss() { m_bIsAdapted = false; };
    ~GuildBoss() {};
    bool InitFromDB(PKGMETA::DT_GUILD_BOSS_BLOB& rstBlob, uint64_t ullGuildId, uint16_t wVersion);
    bool PackGuildBossInfo(PKGMETA::DT_GUILD_BOSS_BLOB& rstBlob, uint16_t wVersion);
    void Clear();
    bool InitFromFile();

    //  初始Boss相关信息
    bool NewInit();

    int  GetBossLeftHp(uint64_t ullUin, uint32_t dwFLevelId, OUT uint32_t& dwBossLeftHp);
    int GetBossLeftHp(uint32_t dwBossId);
    //int GetCurBossId();
    bool UnlockNextBoss(uint8_t bGuildLv);
    PKGMETA::DT_GUILD_BOSS_INFO& GetBossInfo() { return m_oBossInfo; };
    //  Boss扣血
    void AddDamage(uint64_t ullUin, uint32_t dwFLevelId, uint32_t dwDamageHp, OUT uint8_t& bKilled, OUT uint32_t& dwBossId);
	// 重置
	void Reset( );
    void SingleReset();

    uint8_t* GetGuildBossAwardStateList() { return m_szGuildBossAwardStateList; }

    //有成员攻击Boss造成伤害后将该成员的伤害增加到伤害展示数组中
    void SetMemDamageList(uint64_t ullUin, uint32_t dwBossId, uint32_t dwDamage);

    //刷新时重置伤害展示数组
    void ResetMemDamageList(uint64_t dwBossId);

    //成员退出公会时删除该成员造成的伤害信息
    void DelMemDamageInfo(uint64_t ullUin, uint32_t dwBossId);

    //获取BossID对应的单个Boss信息
    PKGMETA::DT_GUILD_ONE_BOSS_INFO* GetOneBossInfo(uint32_t dwBossId);

    int FindBossIndex(uint32_t dwBossID);

    void SendSubsectionRwd(uint32_t dwBossId);

    void ResetMemFightTimesToZero();

    void SendDamageRankRwd(uint32_t dwBossId);

    void SetSubRwdList(uint64_t ullUin, uint8_t bCount, PKGMETA::DT_ITEM stItem);

    void DelMemGetSubRwdInfo(uint64_t ullUin);

    void DelMemFightTimesInfo(uint64_t ullUin);

    int AddMemFightTimesInfo(size_t nMemNum, uint64_t ullUin);

    void ResetFightTimesInfo();

private:
	void _ResetBossListFromRes();
	void _Unlock1stBoss();
	// 添加不存在的boss至boss list
	void _AddNewBossFromRes();

private:
    PKGMETA::DT_GUILD_BOSS_INFO m_oBossInfo;
    uint64_t m_ullGuildId;
    //uint32_t m_BossHp[PKGMETA::MAX_GUILD_BOSS_NUM];
    //std::map<uint32_t, uint32_t> m_FLevelToBossIdMap;   // <副本ID, BossId>
    uint8_t m_szGuildBossAwardStateList[PKGMETA::MAX_GUILD_BOSS_NUM];
    PKGMETA::SSPKG m_stSsPkg;

    bool m_bIsAdapted;

    static const int GUILD_BOSS_SUBSECTION_REWARD_MAIL_ID;
    static const int GUILD_BOSS_DAMAGE_RANK_REWARD_MAIL_ID;
};
