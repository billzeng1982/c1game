#pragma once

#include <vector>
#include <map>
#include <list>
#include "object.h"
#include "define.h"
#include "vector3.h"
#include "common_proto.h"
#include "MyTdrBuf.h"
#include "ss_proto.h"
#include "cs_proto.h"
#include "ov_res_public.h"
#include "../module/skill/FilterManager.h"

using namespace PKGMETA;
class Player;
class FightObj;
class FightPlayer;
class Troop;
class City;
class Barrier;
class Tower;
class Catapult;

#define DUNGEON_DELAY_DEAD_INTERVAL	(2500)

typedef std::map<uint8_t, FightObj*> MapId2FightObj_t;
typedef std::list<FightObj*> ListFightObj_t;

class Dungeon: public IObject
{
public:
	//对时次数
	const static int TimeSyncCnt = 10;

	const static int TimeOut4WaitConnect = 10*1000;
	const static int TimeOut4ChooseGeneral = 31*1000;
	const static int TimeOut4ChooseMSkill = 21*1000;
	const static int CntOut4Choose = 8;
	const static int TimeOut4WaitLoading = 60*1000;
	const static int TimeOut4Op = 30*1000;
	const static int TimeOut4LockAmbush = 30*1000;
	const static int TimeOut4LockDead = 30*1000;
	const static int TimeOut4LockSkill = 60*1000;
	const static int TimeOut4LockSolo = 60*1000;
	const static int TimeOut4LockMSkill = 60*1000;
	const static int TimeOut4OfflineSettle = 3*1000;

	static int ChooseOrder[8];

public:
	int m_iTimeLeft4WaitConnect;
    int m_iTimeLeft4WaitLoading;
    int m_iTimeLeft4Choose;
    int m_iCntLeft4Choose;
    int m_iTimeLeft4Op;
    int m_iTimeLeft4Fight;
    int m_iTimeLeft4LockAmbush;
    int m_iTimeLeft4LockDead;
    int m_iTimeLeft4LockSkill;
    int m_iTimeLeft4LockSolo;
    int m_iTimeLeft4LockMSkill;
    int m_iTimeLeft4Ed;

    uint64_t m_ullDelayDeadLastTime;

public:
    enum LOCKSCREEN_TYPE
    {
        LOCKSCREEN_TYPE_NONE = 0,
        LOCKSCREEN_TYPE_SKILL,
        LOCKSCREEN_TYPE_SOLO,
        LOCKSCREEN_TYPE_DEAD,
    };

    enum TROOP_FIGHT_STATE
    {
        TROOP_FIGHT_STATE_NORMAL = 0,
        TROOP_FIGHT_STATE_RETREAT = 1,
        TROOP_FIGHT_STATE_DEAD = 2,
    };

    struct CsPkgInfo
    {
        bool m_bLast;
        uint32_t m_dwSessionId;
        MyTdrBuf m_stCsSendBuf;

        CsPkgInfo(size_t uBufferSize):m_stCsSendBuf(uBufferSize)
        {
        }
    };
    typedef std::list<CsPkgInfo*> ListCsPkg_t;

public:
	static int GeneralCmp(const void *pstFirst, const void *pstSecond);
	static int MSkillCmp(const void *pstFirst, const void *pstSecond);

public:
    Dungeon();
    virtual ~Dungeon();
    virtual void Clear();
private:
    void _Construct();
    void _ClearListCsPkgWaitDeal();

public:
    static Dungeon* Get();
    static void Release(Dungeon* pObj);

public:
    bool Init(DT_FIGHT_DUNGEON_INFO* pDungeonInfo);
    virtual void Update(int iDeltaTime); // ms

public:
    int GetState() { return m_iState; }
    void SetState(int iState) { m_iState = iState; }

	//生成可选武将列表
	void GenChooseGeneral();

	//从可选武将列表中查找某个武将
	int FindGeneral(uint32_t dwGeneralId);

	void DelGeneral(uint32_t dwGeneralId);

	//从可选武将列表中查找某个武将
	int FindMSkill(uint8_t bMSkillId);

	//初始化单个部队
	void InitOneTroop(FightPlayer* poFightPlayer, DT_TROOP_INFO& rstToopInfo);

private:
    void _InitTroopInfo(FightPlayer* poFightPlayer, DT_FIGHT_PLAYER_INFO& rPlayerInfo);

public:
    // 广播报文到客户端
    void Broadcast(PKGMETA::SCPKG* pstScPkg, Player* poPlayerExclude = NULL, char cSessCmd = PKGMETA::CONNSESSION_CMD_INPROC);
    void BroadcastWithoutAck(PKGMETA::SCPKG* pstScPkg, Player* poPlayerExclude = NULL, char cSessCmd = PKGMETA::CONNSESSION_CMD_INPROC);

    // 通过Group找Player
    Player* GetPlayerByGroup(int8_t chGroup);
    int8_t GetGroupOpposite(int8_t chGroup);

    // 通过Player*找Group
    int8_t GetGroupByPlayerPtr(Player* poPlayer);

    // 设置副本战场玩家信息
    void SetFightPlayerByUin(uint64_t ullUin, Player* poPlayer);

    // 通过Group找战场玩家
    FightPlayer* GetFightPlayerByGroup(int8_t chGroup);

    // 通过Id找战场玩家
    FightPlayer* GetFightPlayerById(uint8_t bId);

	//通过玩家Uin找到战场玩家
	FightPlayer* GetFightPlayerByUin(uint64_t ullUin);

    // 通过Id找城墙
    City* GetCityById(uint8_t bId);

    // 通过Id找部队
    Troop* GetTroopById(uint8_t bId);

    // 通过阵营找部队
    ListFightObj_t* GetTroopListByGroup(int8_t chGroup);

    // 通过Id找栅栏
    Barrier* GetBarrierById(uint8_t bId);

    // 通过Id找瞭望塔
    Tower* GetTowerById(uint8_t bId);

    // 通过Id找投石车
    Catapult* GetCatapultById(uint8_t bId);

    // 通过DT_FIGHTOBJ找FightObj
    FightObj* GetFightObj(DT_FIGHTOBJ& stFightObj);

    // 开战判断
    bool FightReady(Player* poPlayer);

    // 单挑结算，返回是否结算成功，与胜利部队
    bool SoloSettle(PKGMETA::DT_SOLO_SETTLE_INFO* pSoloSettleInfo, int8_t& chWinnerGroup /*out value*/);

    // 是否有死亡部队
    bool HasTroopDead();

    // 加入死亡部队
    void AddTroopDead(Troop* poTroop);

    // 删除死亡部队
    void DelTroopDead(Troop* poTroop);

    // 清空死亡部队
    void ClearTroopDead();

    bool IsAllTroopDead(int8_t chGroup, FightObj* poTroopExclude = NULL);

    // 是否有伏兵部队
    bool HasTroopAmbush();

    // 加入伏兵部队
    void AddTroopAmbush(Troop* poTroop);

    // 删除伏兵部队
    void DelTroopAmbush(Troop* poTroop);

    // 清空伏兵部队
    void ClearTroopAmbush();

    // 清空副本中的玩家
    void ClearPlayer();

    // 添加待处理的包
    void PushCsPkgWaitDeal(uint32_t dwSessionId, PKGMETA::CSPKG& roPkg);

    // 处理待处理的包
    void DealCsPkgWaitDeal();

    // 士气处理
    void UpdateMorale(int iDeltaTime);

    void ChgMorale(int8_t chGroup, float fMorale);

    // 检测攻城频率
    bool CheckAttackFreq(DT_HP_INFO& rHpInfo);

public:
    int m_iTimeSyncCnt;				// 对时次数

    uint32_t m_dwDungeonId;			// 副本ID
    int m_iState;					// 副本状态

	uint8_t m_bTerrainId;			//地形Id

    int m_iDungeonTime;				// 副本时间, ms
    uint64_t m_ullTimestamp;		// 副本创建时间戳

    uint8_t m_bMatchType;			// 匹配类型
	uint8_t m_bFakeType;			// 假匹配类型

	uint8_t m_bMaxPlayer;			//最大玩家数
	uint8_t m_bOnlinePlayerCnt;     //当前在线玩家数量

	uint8_t m_bChooseGroup;         //当前正在选人的玩家阵营
	uint8_t m_bChooseCount;         //当前已选的武将数

	uint8_t m_bGeneralCount;
	DT_RANK_GENERAL_INFO m_GeneralList[MAX_NUM_GCARD_FOR_CHOOSE];

	uint8_t m_bMSkillCount;
	DT_ITEM_MSKILL m_MSkillList[MAX_NUM_ROLE_MSKILL];

public:
    // 战场信息
    Vector3 m_stArenaSize;

    ListFightObj_t m_listFightPlayer;

    ListFightObj_t m_listCity;

    ListFightObj_t m_listTroopDown;
    ListFightObj_t m_listTroopUp;

    ListFightObj_t m_listTower;
    ListFightObj_t m_listBarrier;
    ListFightObj_t m_listCatapult;

private:
    // 战场玩家
    MapId2FightObj_t m_mapId2FightPlayer;

    // 城墙信息
    MapId2FightObj_t m_mapId2City;

    // 部队信息
    MapId2FightObj_t m_mapId2Troop;

    // 栅栏信息
    MapId2FightObj_t m_mapId2Barrier;

    // 瞭望塔信息
    MapId2FightObj_t m_mapId2Tower;

    // 投石车信息
    MapId2FightObj_t m_mapId2Catapult;

    // 死亡部队列表，该列表中不会有重复的部队，添加都在尾部，删除都在头部
    ListFightObj_t m_listTroopDead;

    // 伏兵部队列表，该列表中不会有重复的部队，添加都在尾部，删除都在头部
    ListFightObj_t m_listTroopAmbush;

public:
    //防作弊信息
    DT_LAST_ATTACK_TIME_INFO m_stLastAttackTimeInfo;

public:
    FilterManager m_oFilterManager;		// 过滤器管理

private:
    ListCsPkg_t m_listCsPkgWaitDeal;	// 缓存待处理的报文
};
