#pragma once
#include <map>
#include <string>
#include "define.h"
#include "singleton.h"
#include "mempool.h"
#include "functors.h"
#include "list_i.h"
#include "Guild.h"
#include "../../thread/GuildDBThread.h"
#include "../../transaction/GuildTransFrame.h"
#include "GameTime.h"

using namespace PKGMETA;
using namespace std;

#define BASE_TIMESTAMP 1472659200

struct GuildNode
{
    Guild m_oGuild;
    TLISTNODE m_stTimeListNode; //时间链表节点，用于置换
    TLISTNODE m_stDirtyListNode; //脏数据链表节点，用于回写数据库
};

class GuildMgr : public TSingleton<GuildMgr>
{
public:
    static const int GUILD_UPT_NUM_PER_FRAME = 10;
    static const int MAX_NUM_CREATE_ONE_SEC = 16; //一秒内最多可以创建的公会数

private:
    typedef hash_map_t<const char*, Guild*, __gnu_cxx::hash<const char*>, eqstr> GuildNameMap_t;
    typedef map<uint64_t, Guild*> GuildIdMap_t;

public:
    GuildMgr(){};
    virtual ~GuildMgr(){};

    bool Init(GUILDSVRCFG * pstConfig);
    void Update();
    void Fini();

    //创建公会,ullTokenId表明是哪个Action执行此函数
    int CreateGuild(const char* pszName, TActionToken ullTokenId);

    //通过id查找公会, ullTokenId表明是哪个Action执行此函数
    Guild* GetGuild(uint64_t ullGuildId, TActionToken ullTokenId =0);

    //通过名字查找公会
    Guild* GetGuild(const char* pszName, TActionToken ullTokenId =0);

    //删除公会
    bool DeleteGuild(uint64_t ullGuildId);

    //将公会加入DirtyList
    void AddToDirtyList(Guild* poGuild);

    int RefreshGuildList(uint8_t& rbGuildCount, DT_GUILD_BRIEF_INFO* pstGuildList);

    void SetRefreshCompetitorFlag() { m_bRefreshCompetitorFlag = false; }

    bool GetRefreshCompetitorFlag() { return m_bRefreshCompetitorFlag; }

    DT_GUILD_RANK_INFO m_astGuildRankList[MAX_LEN_ROLE_GUILD];

    uint16_t GetRank(uint64_t ullGuildId);

    uint16_t GetRankNum();

    DT_GUILD_RANK_INFO* GetGuildByRank(uint16_t wGuildRank);
	//获取每日刷新时间(小时)
	uint8_t GetDailyUpdateHour() { return m_bFundUptHour; }
protected:
    Guild* NewGuild(DT_GUILD_WHOLE_DATA& rstGuildInfo);

    void _UpdateGuild();

    void _HandleThreadMsg();
    void _HandleCreateMsg(DT_GUILD_WHOLE_DATA& rstGuildInfo, int iErrNo, uint64_t ullActionId);
    void _HandleGetMsg(DT_GUILD_WHOLE_DATA& rstGuildInfo, int iErrNo, uint64_t ullActionId);
	void _HandleGetByNameMsg(DT_GUILD_WHOLE_DATA& rstGuildInfo, int iErrNo, uint64_t ullActionId);

    void AddGuildToMap(Guild* poGuild);
    void DelGuildFromMap(Guild* poGuild);

    //将Guild移至TimeList的链表头，当此Guild被访问时，调用此函数
    void Move2TimeListFirst(Guild* poGuild);

    void _SaveGuild(Guild* poGuild);

    void _WriteDirtyToDB();

    //生成军团ID
    int GenerateGuildId(uint64_t& ullGuildId);

    void SetGuildBossSingleUptTime();
    void SetGuildBossUptTime();

	
private:
    //数据库线程的数量
    int m_iWorkThreadNum;
    GuildDBThread* m_astWorkThreads;

    int m_iRefreshTimeVal; //刷新的时间间隔
    uint64_t m_ullLastUptTimestamp;//上次刷新时间

    TLISTNODE m_stTimeListHead; // 时间链表头,用于LRU算法

    int m_iDirtyNodeNum; //脏数据个数
    int m_iDirtyNodeMax; //脏数据上限
    int m_iWriteTimeVal; //回写的时间间隔
    uint64_t m_ullLastWriteTimestamp;//上次回写脏数据的时间
    TLISTNODE m_stDirtyListHead; // 脏数据链表头,用于回写数据库

    int m_iMaxSize;
    int m_iCurSize;
    CMemPool<GuildNode> m_GuildPool;
    CMemPool<GuildNode>::UsedIterator m_oUptIter; //update iterator;

    GuildNameMap_t m_NameToGuildMap;
    GuildNameMap_t::iterator m_NameToGuildMapIter;
    GuildIdMap_t m_IdToGuildMap;
    GuildIdMap_t::iterator m_IdToGuildMapIter;

    DT_GUILD_DB_REQ   m_stDBReqPkg;
    DT_GUILD_DB_RSP   m_stDBRspPkg;

    uint8_t m_szBossUptWeekday[3] ;
    uint8_t m_bBossUptHour ;
    uint64_t m_ullBossUptLastUptTime;  //秒
    uint64_t m_ullBossSingleUptLastUptTime;
    bool  m_bBossUptFlag;
    bool m_bBossSingleUptFlag;
    bool m_bRefreshCompetitorFlag;
	uint8_t m_bFundUptHour ;
	uint64_t m_ullFundUptLastUptTime;  //秒，公会每日资金上限
	bool m_bFundUptFlag;

    //以下三项全部用于生成公会Id

    uint16_t m_wSvrId; //服务器ID
    uint64_t m_ullLastGenerateTime;//上次生成Id的时间，秒
    uint32_t m_dwSeq; //一秒内的序号
};

