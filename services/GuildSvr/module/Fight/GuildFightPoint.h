#pragma once

#include <list>
#include "object.h"
#include "common_proto.h"
#include "../../gamedata/GameDataMgr.h"

using namespace PKGMETA;
using namespace std;

class GuildFightArena;
class FightPlayer;
struct MatchPair
{
    uint64_t ullStartTimeMs;    //匹配成功的时间 毫秒
    uint64_t ullRed;
    uint64_t ullBlue;
};

// 公会战   两军团对阵   据点
class GuildFightPoint : public IObject
{
private:
    typedef list<FightPlayer*> ListFightPlayer_t;

public:
    GuildFightPoint() {}
    virtual ~GuildFightPoint() {}

    //根据数据档的配置初始化据点
    bool Init(GuildFightArena* poGuildFightArena, RESGUILDFIGHTMAP* pResGuildFightMap, uint64_t ullTimeMs);

    void Clear();

    virtual void Update(int iDeltaTime);

    //获取移动时间
    int GetMoveTime(uint16_t wDstPoint);

    //玩家进入当前据点
    void AddPlayer(FightPlayer* poPlayer);

    //将玩家从当前据点踢出
    void DelPlayer(FightPlayer* poPlayer);

    //玩家对当前据点造成伤害
    void Damage(FightPlayer* poPlayer);

    //处理匹配
    bool DealMatch(FightPlayer* poPlayer);

    //开始战斗
    void StartFight(MatchPair& stMatchPair);

    //无敌退出时匹配
    bool GodExitMatch(FightPlayer* poPlayer);

public:
    //所属战场
    GuildFightArena* m_poGuildFightArena;

    //据点信息,在玩家加入战场时需要将据点信息传给玩家
    DT_GUILD_FIGHT_ARENA_MAP_PONIT_INFO m_stPointInfo;

    //邻接点移动时间,单位毫秒
    uint32_t m_AdjoinMoveTimeMs[MAX_NUM_GUILD_FIGHT_ADJOIN_POINT];

    //得分周期，单位毫秒
    uint32_t m_dwGainScoreCycleMs;

    //得分值
    uint16_t m_wScoreValue;

    //当前时间,单位毫秒
    uint64_t m_ullTimeMs;

    //下次加分时间
    uint64_t m_ullNextUptTime;

    //战斗前等待时间
    int m_iPreMatchTimeMs;

    //当前据点红方阵营玩家列表，保存玩家列表的目的在于方便进行匹配
    ListFightPlayer_t m_listPlayerRed;

    //当前据点蓝方阵营玩家列表
    ListFightPlayer_t m_listPlayerBlue;

    //匹配成功的玩家,进入PreMatch状态
    list<MatchPair> m_listMatcPair;
};
