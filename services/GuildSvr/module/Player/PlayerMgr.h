#pragma once
#include "Player.h"
#include "singleton.h"
#include "common_proto.h"
#include "mempool.h"
#include <map>
#include "list_i.h"
#include "../../thread/PlayerDBThread.h"
#include "../../transaction/GuildTransFrame.h"

using namespace std;
using namespace PKGMETA;

struct PlayerNode
{
    Player m_oPlayer;
    TLISTNODE m_stTimeListNode;//时间链表节点，用于置换
    TLISTNODE m_stDirtyListNode; //脏数据链表节点，用于回写数据库
};


class PlayerMgr : public TSingleton<PlayerMgr>
{
public:
    static const int PLAYER_UPT_NUM_PER_SECOND = 10;

private:
    typedef map<uint64_t, Player*> PlayerIdMap_t;

public:
    PlayerMgr();
    ~PlayerMgr();

    bool Init(GUILDSVRCFG * pstConfig);
    void Update();
    void Fini();

    Player* GetPlayer(uint64_t ullPlayerId, TActionToken ullTokenId=0);

    //将Player加入DirtyList
    void AddToDirtyList(Player* poPlayer);

    uint64_t GetUptTimestamp() { return m_ullLastUptTimeStamp; }

private:
    Player* NewPlayer(DT_GUILD_PLAYER_DATA& rstPlayerInfo);
    void _HandleThreadMsg();
    void _HandleThreadGetMsg(DT_GUILD_PLAYER_DATA& rstPlayerInfo, int iErrNo, uint64_t ullActionId);

    void AddPlayerToMap(Player* poPlayer);
    void DelPlayerFromMap(Player* poPlayer);

    void _SavePlayer(Player* poPlayer);

    //将Player移至TimeList的链表头，当此Player被访问时，调用此函数
    void Move2TimeListFirst(Player* poPlayer);

    //将DirtyList中的数据写入数据库
    void _WriteDirtyToDB();

    void _UpdateTimeStamp();

private:
    int m_iWorkThreadNum;//工作线程数
    PlayerDBThread* m_astWorkThreads;

    TLISTNODE m_stTimeListHead; // 时间链表头,用于LRU算法

    int m_iDirtyNodeNum; //脏数据个数
    int m_iDirtyNodeMax; //脏数据上限
    int m_iWriteTimeVal; //回写的时间间隔
    uint64_t m_ullLastWriteTimestamp;//上次回写脏数据的时间
    TLISTNODE m_stDirtyListHead; // 脏数据链表头,用于回写数据库
     
    int m_iCurSize;
    int m_iMaxSize;
    CMemPool<PlayerNode> m_oPlayerPool;
    CMemPool<PlayerNode>::UsedIterator m_oUptIter;

    PlayerIdMap_t m_IdToPlayerMap;
    PlayerIdMap_t::iterator m_IdToPlayerMapIter;

    DT_GUILD_PLAYER_DB_REQ   m_stDBReqPkg;
    DT_GUILD_PLAYER_DB_RSP   m_stDBRspPkg;

    bool m_bUptFlag;
    int m_iApplyUptTime; //申请列表重置的时间
    uint64_t m_ullLastUptTimeStamp;
};
