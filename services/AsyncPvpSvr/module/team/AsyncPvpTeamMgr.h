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
        TLISTNODE m_stTimeListNode; //ʱ������ڵ㣬�����û�
        TLISTNODE m_stDirtyListNode; //����������ڵ㣬���ڻ�д���ݿ�
    };

    typedef map<uint64_t, DT_ASYNC_PVP_PLAYER_TEAM_DATA*> TeamIdMap_t;

public:
    AsyncPvpTeamMgr(){}
    ~AsyncPvpTeamMgr(){}

    bool Init(ASYNCPVPSVRCFG * pstConfig);
    void Update();
    void Fini();

    //��������������
    DT_ASYNC_PVP_PLAYER_TEAM_DATA* GetTeamData(uint64_t ullPlayerId, uint64_t ullActionId = 0);

    //�½������������
    int CreateTeamData(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData, uint64_t ullActionId);

    //���������������
    void UpdateTeamData(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData);

    //�����������б�
    void AddToDirtyList(DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam);

protected:
    DT_ASYNC_PVP_PLAYER_TEAM_DATA* _NewTeam(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData);

    //������߳���Ϣ
    void _HandleThreadMsg();
    void _HandleCreateMsg(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData, int iErrNo, uint64_t ullActionId);
    void _HandleGetMsg(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData, int iErrNo, uint64_t ullActionId);

    void AddTeamToMap(DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam);
    void DelTeamFromMap(DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam);

    void _Move2TimeListFirst(DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam);

    void _SaveTeam(DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam);

    //�������ݻ�д�����ݿ�
    void _WriteDirtyToDB();

private:
    //���߳���ز���
    int m_iWorkThreadNum;//�߳���
    TeamDBThread* m_astWorkThreads;

    // ʱ������ͷ,����LRU�㷨
    TLISTNODE m_stTimeListHead;

    //��������ز���
    int m_iDirtyNodeNum; //�����ݸ���
    int m_iDirtyNodeMax; //����������
    int m_iWriteTimeVal; //��д��ʱ����
    uint64_t m_ullLastWriteTimestamp;//�ϴλ�д�����ݵ�ʱ��
    TLISTNODE m_stDirtyListHead; // ����������ͷ,���ڻ�д���ݿ�*/

    //�ڴ����ز���
    int m_iCurSize;
    int m_iMaxSize;
    CMemPool<TeamNode> m_oTeamPool;
    CMemPool<TeamNode>::UsedIterator m_oUptIter;

    TeamIdMap_t::iterator m_oIter;
    TeamIdMap_t m_oTeamMap;

    DT_ASYNC_PVP_TEAM_DB_REQ     m_stDBReqPkg;
    DT_ASYNC_PVP_TEAM_DB_RSP     m_stDBRspPkg;
};
