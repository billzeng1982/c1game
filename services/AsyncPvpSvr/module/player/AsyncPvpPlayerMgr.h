#pragma once

#include "../../AsyncPvpSvr.h"
#include "AsyncPvpPlayer.h"
#include "../../thread/PlayerDBThread.h"
#include "singleton.h"
#include "list_i.h"
#include "mempool.h"
#include <map>

using namespace std;
using namespace PKGMETA;

class AsyncPvpPlayerMgr : public TSingleton<AsyncPvpPlayerMgr>
{
private:
    const static int PLAYER_INIT_NUM = 3000;
    const static int PLAYER_DELTA_NUM = 1000;
    const static int PLAYER_UPT_NUM_PER_FRAME = 10;

    struct PlayerNode
    {
        AsyncPvpPlayer m_oPlayer;
        TLISTNODE m_stTimeListNode; //时间链表节点，用于置换
        TLISTNODE m_stDirtyListNode; //脏数据链表节点，用于回写数据库
    };

    typedef map<uint64_t, AsyncPvpPlayer*> PlayerIdMap_t;

public:
    AsyncPvpPlayerMgr(){}
    ~AsyncPvpPlayerMgr(){}

    bool Init(ASYNCPVPSVRCFG * pstConfig);
    void Update();
    void Fini();

    //获得玩家数据
    AsyncPvpPlayer* GetPlayer(uint64_t ullPlayerId, uint64_t ullActionId = 0);

    //新建玩家(异步接口,需传入ActionId,当创建成功后,会唤醒相应的Action)
    int CreatePlayer(DT_ASYNC_PVP_PLAYER_SHOW_DATA& rstShowData, uint64_t ullActionId);

    //删除玩家
    void DelPlayer(uint64_t ullPlayerId);

    //更新玩家数据
    void UpdatePlayer(DT_ASYNC_PVP_PLAYER_SHOW_DATA& rstShowData);

    //将公会加入DirtyList(当外部修改Player的数据时，需要调用此接口)
    void AddToDirtyList(AsyncPvpPlayer* poPlayer);

protected:
    AsyncPvpPlayer* _NewPlayer(DT_ASYNC_PVP_PLAYER_WHOLE_DATA& rstPlayerData);

    //生成假玩家
    AsyncPvpPlayer* _GenFakePlayer(uint64_t ullPlayerId);

    //处理从线程消息
    void _HandleThreadMsg();
    void _HandleCreateMsg(DT_ASYNC_PVP_PLAYER_WHOLE_DATA& rstPlayerData, int iErrNo, uint64_t ullActionId);
    void _HandleGetMsg(DT_ASYNC_PVP_PLAYER_WHOLE_DATA& rstPlayerData, int iErrNo, uint64_t ullActionId);

    void AddPlayerToMap(AsyncPvpPlayer* poPlayer);
    void DelPlayerFromMap(AsyncPvpPlayer* poPlayer);

    //将Player移至TimeList的链表头，当此Player被访问时，调用此函数
    void _Move2TimeListFirst(AsyncPvpPlayer* poPlayer);

    void _SavePlayer(AsyncPvpPlayer* poPlayer);

    //将脏数据回写到数据库
    void _WriteDirtyToDB();

private:
    //从线程相关参数
    int m_iWorkThreadNum;//线程数
    PlayerDBThread* m_astWorkThreads;

    // 时间链表头,用于LRU算法
    TLISTNODE m_stTimeListHead;

    //脏数据相关参数
    int m_iDirtyNodeNum; //脏数据个数
    int m_iDirtyNodeMax; //脏数据上限
    int m_iWriteTimeVal; //回写的时间间隔
    uint64_t m_ullLastWriteTimestamp;//上次回写脏数据的时间
    TLISTNODE m_stDirtyListHead; // 脏数据链表头,用于回写数据库*/

    //内存池相关参数
    int m_iCurSize;
    int m_iMaxSize;
    CMemPool<PlayerNode> m_oPlayerPool;
    CMemPool<PlayerNode>::UsedIterator m_oUptIter;

    PlayerIdMap_t::iterator m_oIter;
    PlayerIdMap_t m_oPlayerMap;

    DT_ASYNC_PVP_PLAYER_DB_REQ     m_stDBReqPkg;
    DT_ASYNC_PVP_PLAYER_DB_RSP     m_stDBRspPkg;
};
