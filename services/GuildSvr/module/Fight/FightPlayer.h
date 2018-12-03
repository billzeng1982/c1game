#pragma once
#include "common_proto.h"
#include "object.h"

using namespace PKGMETA;

// 公会战
class GuildFightArena;
class GuildFightPoint;

class FightPlayer : public IObject
{
public:
    FightPlayer(){}
    virtual ~FightPlayer(){}

    bool Init(GuildFightArena* poGuildFightArena, DT_GUILD_FIGHT_PLAYER_INFO& rstPlayerInfo, uint8_t bCamp, uint64_t ullTimeMs,
				uint16_t wHeadIcon, uint16_t wHeadFrame, uint16_t wHeadTitle);

    void Clear();

    virtual void Update(int iDeltaTime);

    //移动请求，返回值小于0为错误码，大于0为移动需要的时间
    int Move(uint16_t wDstPoint);

    //匹配结束后的结算(记录匹配结果，统计杀敌数，增加积分等)
    void MatchSettle(uint64_t ullWinnerId);

    //伤害据点(记录据点伤害，增加积分等)
    void DamagePoint(uint16_t wDamageValue);

    //复活
    void Revive();

    //状态
    int GetState();
    void SetState(int iNewState);

public:
    //玩家所属军团
    uint64_t m_ullGuildId;

    //玩家所属的战场
    GuildFightArena* m_poGuildFightArena;

    //玩家所在据点
    GuildFightPoint* m_poGuildFightPoint;

    //玩家信息
    DT_GUILD_FIGHT_ARENA_PLAYER_INFO m_stPlayerInfo;

    //当前时间
    uint64_t m_ullTimeMs;

    //玩家对据点造成伤害的周期
    uint32_t m_dwDamageCycleMs;

    //伤害值
    uint16_t m_wDamageValue;

    //复活冷却时间
    uint32_t m_dwReviveCoolTimeMs;

    //复活点
    uint16_t m_wRevivePoint;

    //赢得军团战匹配后获得的积分
    uint16_t m_wMatchWinScore;

    //移动的目的地
    uint16_t m_wDstPointId;

	//赢得军团战匹配后获得的个人积分
	uint16_t m_wWinPersonScore;

	//攻击据点后获得的个人积分
	uint16_t m_wDamageScore;

    //移动的剩余时间
    int m_iTimeLeftMove;

    //造成伤害的剩余时间
    int m_iTimeLeftDamage;

    //复活剩余时间
    int m_iTimeLeftRevive;

    //上次PVP匹配的结果
    int m_iFightPvpResult;

    //无敌时间 常量
    int m_iIniGodLeftTimeMs;

    //无敌剩余时间 毫秒
    int m_iGodLeftTimeMs;
};