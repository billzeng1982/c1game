#pragma once

#include "define.h"
#include "singleton.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "GameTimerMgr_PQ.h"
#include "LogMacros.h"
#include <map>
#include "../gamedata/GameDataMgr.h"
#include "list_i.h"


using namespace std;
using namespace PKGMETA;

class MatchInfo;

struct Section
{
    int m_iLow;
    int m_iHigh;

    bool operator < (const Section& a) const
    {
        if (m_iHigh < a.m_iLow)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool operator == (const Section& a) const
    {
        if ((a.m_iLow >= m_iLow) && (a.m_iHigh <= m_iHigh))
        {
            return true;
        }
        else if ((m_iLow >= a.m_iLow) && (m_iHigh <= a.m_iHigh))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

#define FAKE_PLAYER_MAX_NUM 50
typedef std::map<uint64_t, int> MapUin2FakePlayer_t;
struct FakeNode : public IObject
{
    int m_iCount;
    DT_FIGHT_PLAYER_INFO m_astPlayerList[FAKE_PLAYER_MAX_NUM];
    MapUin2FakePlayer_t m_oMapPlayer;
};

class MatchBase
{
public:
    const static int DEFAULT_MATCH_TIMER_NUM = 100;
    const static int MATCH_TIMER_CHECK_TIMES = 90;

    typedef map<uint32_t, MatchInfo*> MapKey2Info_t;
    typedef map<Section, RESMATCH*> MapSection2Info_t;
    typedef map<Section, FakeNode*> MapSection2Fake_t;

public:
    static void ReleaseTimer(GameTimer* pTimer);
    static void _ReleaseMatchInfo(MatchInfo* pstInfo, MatchBase* pstMgr);

public:
    MatchBase();
    virtual ~MatchBase();

    bool Init();

    //派生类重写此方法做一些自己的初始化
    virtual bool AppInit() { return true; };

    void Update();

    virtual void AppUpdate() {}

    //Msg主动匹配
    bool MatchStartHandle(DT_FIGHT_PLAYER_INFO& rstSelfInfo);

    //定时器匹配触发匹配
    bool MatchStartTimer(MatchInfo& rstInfo, uint32_t dwWaitCount);

    //取消匹配
    int MatchCancelHandle(uint64_t m_ullUin, uint32_t m_dwScore);

protected:
    bool _InitBasic();

    bool _CheckFakeMatch(MatchInfo& rstMatchInfo);

    bool _FakeMatch(DT_FIGHT_PLAYER_INFO& rstPlayerInfo);

    void _SendFakeMatchStartRsp(DT_FIGHT_PLAYER_INFO& rstSelfInfo, uint8_t m_bMatchAIType);

    void _SendCreateDungeonReq(DT_FIGHT_PLAYER_INFO& rstSelfInfo, DT_FIGHT_PLAYER_INFO& rstOpponentInfo);

    void _SaveFightPlayerInfo(DT_FIGHT_PLAYER_INFO& rstPlayerInfo);

    void _GetFakePlayerInfo(DT_FIGHT_PLAYER_INFO& rstSelfInfo, DT_FIGHT_PLAYER_INFO& rstOpponentInfo, uint8_t& bMatchAIType);

protected:
    //通用匹配池(存放玩家,key为玩家的匹配所用战力)
    MapKey2Info_t m_oMapScore;

    //积分匹配区段(读配置档)
    MapSection2Info_t m_oMapSection;

    //缓存匹配成功玩家的数据，用于假匹配时提供真实玩家的数据
    MapSection2Fake_t m_oMapFakePlayer;

    //定时器管理
    GameTimerMgr_PQ m_oTimerMgr;

    //匹配类型:排位赛、休闲赛、每日挑战赛等等
    uint8_t m_bMatchType;

    //地图个数
    uint8_t m_bTerrainNum;

    SSPKG m_stSsPkg;
};


