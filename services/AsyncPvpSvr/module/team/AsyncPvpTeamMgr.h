#pragma once

#include "AsyncPvpSvr.h"
#include "TeamDBThread.h"
#include "singleton.h"
#include "list_i.h"
#include "mempool.h"
#include <map>

using namespace std;
using namespace PKGMETA;

class AsyncPvpTeamMgr : public TSingleton<AsyncPvpTeamMgr>
{
private:
    const static int TEAM_INIT_NUM = 3000;
    const static int TEAM_DELTA_NUM = 1000;
    const static int TEAM_UPT_NUM_PER_FRAME = 10;

    struct TeamNode
    {
        DT_ASYNC_PVP_PLAYER_TEAM_DATA m_oTeam;
        TLISTNODE m_stTimeListNode; //时间链表节点，用于置换
        TLISTNODE m_stDirtyListNode; //脏数据链表节点，用于回写数据库
    };

    typedef map<uint64_t, DT_ASYNC_PVP_PLAYER_TEAM_DATA*> TeamIdMap_t;

public:
    AsyncPvpTeamMgr(){}
    ~AsyncPvpTeamMgr(){}

    bool Init(ASYNCPVPSVRCFG * pstConfig);
    void Update();
    void Fini();

    //获得玩家阵容数据
    DT_ASYNC_PVP_PLAYER_TEAM_DATA* GetTeamData(uint64_t ullPlayerId, uint64_t ullActionId = 0);

    //新建玩家阵容数据
    int CreateTeamData(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData, uint64_t ullActionId);

    //更新玩家阵容数据
    void UpdateTeamData(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData);

    //加入脏数据列表
    void AddToDirtyList(DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam);

protected:
    DT_ASYNC_PVP_PLAYER_TEAM_DATA* _NewTeam(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData);

    //处理从线程消息
    void _HandleThreadMsg();
    void _HandleCreateMsg(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData, int iErrNo, uint64_t ullActionId);
    void _HandleGetMsg(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData, int iErrNo, uint64_t ullActionId);

    void AddTeamToMap(DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam);
    void DelTeamFromMap(DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam);

    void _Move2TimeListFirst(DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam);

    void _SaveTeam(DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam);

    //将脏数据回写到数据库
    void _WriteDirtyToDB();

private:
    //从线程相关参数
    int m_iWorkThreadNum;//线程数
    TeamDBThread* m_astWorkThreads;

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
    CMemPool<TeamNode> m_oTeamPool;
    CMemPool<TeamNode>::UsedIterator m_oUptIter;

    TeamIdMap_t::iterator m_oIter;
    TeamIdMap_t m_oTeamMap;

    DT_ASYNC_PVP_TEAM_DB_REQ     m_stDBReqPkg;
    DT_ASYNC_PVP_TEAM_DB_RSP     m_stDBRspPkg;
};
