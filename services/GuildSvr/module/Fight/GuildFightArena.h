#pragma once
#include "../Guild/Guild.h"
#include "ss_proto.h"
#include "object.h"
#include <map>

using namespace PKGMETA;
using namespace std;

class FightPlayer;
class GuildFightPoint;

// 公会战  战场
class GuildFightArena : public IObject
{
private:
    typedef map<uint16_t, GuildFightPoint*> MapId2Point_t;
    typedef map<uint64_t, FightPlayer*> MapId2Player_t;

public:
    GuildFightArena() {}
    virtual ~GuildFightArena() {}

    bool Init(uint64_t GuildList[], int iGuildCnt, uint8_t bAgainstId);

    void Clear();

    virtual void Update(int iDeltaTime);

    //战场刚创建时，有一段开战准备时间，准备时间过后，调用此函数切换到正式开战状态
    void Start();

    //状态同步，广播，发给战场内的所有玩家
    void Broadcast();

    //加入战场请求，rstArenaInfo为战场的全部信息
    int Join(SS_PKG_GUILD_FIGHT_ARENA_JOIN_REQ& rstSsPkgReq, DT_GUILD_FIGHT_ARENA_INFO& rstArenaInfo, uint64_t& ullTimeStamp);

    //退出战场请求
    int Quit(uint64_t ullPlayerId);

    //战场内移动请求，返回值小于0为错误码，返回值大于0为移动需要的时间
    int Move(uint64_t ullPlayerId, uint16_t wDstPoint);

    //通过Id找Player
    FightPlayer* GetPlayer(uint64_t ullPlayerId);

    //通过Id找据点
    GuildFightPoint* GetPoint(uint16_t wPointId);

    //获得积分
    void GainScore(uint8_t bCamp, uint16_t wPointId, uint32_t dwScore);

	// 添加战场状态同步消息
	void AddStateSync(DT_GUILD_FIGHT_ARENA_PLAYER_INFO& rstArenaPlayerInfo);

    //更改军团统计信息
    void ChgStatisInfo(uint8_t bCamp, int iType, int iValue);

    //结算
    uint64_t Settle();

	//结算个人积分排名并发奖励
	void SettleRank();

private:
    bool _InitMap(); //初始化地图
    bool _InitPlayer(); //初始化地图
    bool _InitGuild(uint64_t GuildList[], int iGuildCnt); //初始化对阵双方军团
    bool _InitBasePara(uint8_t bAgainstId);//初始化基本参数

	void _SettleRank(uint8_t bCount, FightPlayer* PlayerList[]);

public:
    //战场ID,由GuildFightMgr分配,全局唯一,用于标示战场
    uint8_t m_bAgainstId;

    //当前战场状态,分为准备状态,开战状态
    uint8_t m_iState;

    //开战时间
    uint64_t m_ullStartTimeMs;

    //获胜阵营和公会id
    uint8_t m_bWinCamp;
    uint64_t m_ullWinGuild;

    //获胜所需积分
    uint32_t m_dwWinScore;

	//积分加快需要占领的据点数
	uint16_t m_wGainScorePoint1;
	uint16_t m_wGainScorePoint2;

    //占领所有据点后积分加倍的倍率
    uint16_t m_wGainScoreRate1;
	uint16_t m_wGainScoreRate2;

    // 据点信息
	MapId2Point_t m_mapId2Point;
    MapId2Point_t::iterator m_mapId2PointIter;

    // 战场玩家信息
	MapId2Player_t m_mapId2Player;
    MapId2Player_t::iterator m_mapId2PlayerIter;

    //对阵军团信息
    DT_GUILD_FIGHT_ARENA_GUILD_INFO m_GuildInfoList[MAX_NUM_ONE_GUILD_FIGHT_JOIN_GUILD];

    //对阵军团统计信息
    DT_GUILD_FIGHT_STATIS_INFO m_GuildStatisList[MAX_NUM_ONE_GUILD_FIGHT_JOIN_GUILD];

    //战场玩家列表，用于军团战广播
    DT_GUILD_FIGHT_ONE_CAMP_PLAYER_LIST m_GuildPlayerList[MAX_NUM_ONE_GUILD_FIGHT_JOIN_GUILD];

    //当前时间,单位毫秒
    uint64_t m_ullTimeMs;

    //加入战场冷却时间
    uint64_t m_ullJoinCoolTimeMs;

    //新加入公会后，能参加公会战最少需要经过的时间
    uint64_t m_ullLeastPassTimeMs;

    SSPKG m_stSsPkg;

    //广播消息
    DT_GUILD_FIGHT_SYN_MSG m_stSynMsg;
};