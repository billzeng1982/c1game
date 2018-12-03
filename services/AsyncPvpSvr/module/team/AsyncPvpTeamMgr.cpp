#include "AsyncPvpTeamMgr.h"
#include "AsyncPvpTransFrame.h"
#include "GameDataMgr.h"

using namespace PKGMETA;

bool AsyncPvpTeamMgr::Init(ASYNCPVPSVRCFG * pstConfig)
{
    if (pstConfig == NULL)
    {
       return false;
    }

    //初始化脏数据相关
    m_iDirtyNodeNum = 0;
    m_iDirtyNodeMax = pstConfig->m_iDirtyNodeMax;
    m_iWriteTimeVal = pstConfig->m_iWriteTimeVal;
    m_ullLastWriteTimestamp = CGameTime::Instance().GetCurrSecond();
    TLIST_INIT(&m_stDirtyListHead);

    //初始化mempool相关
    m_iCurSize = 0;
    m_iMaxSize = pstConfig->m_iPlayerMax;
    if (m_oTeamPool.CreatePool(m_iMaxSize) < 0)
    {
       LOGERR_r("Create Player pool[MaxNum=%d] failed.", m_iMaxSize);
       return false;
    }
    m_oTeamPool.RegisterSlicedIter(&m_oUptIter);

    //初始化工作线程
    m_iWorkThreadNum = pstConfig->m_iWorkThreadNum;
    m_astWorkThreads = new TeamDBThread[m_iWorkThreadNum];
    if (!m_astWorkThreads)
    {
       LOGERR_r("Init DBWorkThread failed.");
       return false;
    }
    key_t iShmKey = pstConfig->m_iThreadQBaseShmKey;
    bool bRet = 0;
    for( int i = 0; i < m_iWorkThreadNum; i++ )
    {
       bRet = m_astWorkThreads[i].InitThread(i, pstConfig->m_dwThreadQSize, THREAD_QUEUE_DUPLEX, (void*)pstConfig, &iShmKey);
       if( !bRet )
       {
           LOGERR_r("Init thread <%d> failed", i );
           return false;
       }
    }

    //初始化时间链表
    TLIST_INIT(&m_stTimeListHead);

    m_oTeamMap.clear();

    return true;
}


void AsyncPvpTeamMgr::Update()
{
    this->_HandleThreadMsg();

    this->_WriteDirtyToDB();
}


DT_ASYNC_PVP_PLAYER_TEAM_DATA* AsyncPvpTeamMgr::_NewTeam(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData)
{
    DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam = NULL;
    TeamNode* pstTeamNode = NULL;

    //mempool中有空余时
    if (m_iCurSize < m_iMaxSize)
    {
        pstTeamNode = m_oTeamPool.NewData();
        if (NULL == pstTeamNode)
        {
            LOGERR_r("pstTeamNode is NULL, get TeamNode from TeamPool failed");
            return NULL;
        }

        //初始化Team
        pstTeam = &(pstTeamNode->m_oTeam);
        *pstTeam = rstTeamData;

        //加入时间链表头
        TLIST_INSERT_NEXT(&m_stTimeListHead, &(pstTeamNode->m_stTimeListNode));
        m_iCurSize++;
    }
    //mempool中没有空余时，需要置换，置换采用LRU算法，将最近最少使用的节点回写到数据库
    else
    {
        //取时间链表尾节点
        TLISTNODE* pstSwapNode = TLIST_PREV(&m_stTimeListHead);
        pstTeamNode = container_of(pstSwapNode, TeamNode, m_stTimeListNode);

        //被置换的TeamNode需要回写数据库
        pstTeam = &(pstTeamNode->m_oTeam);
        this->_SaveTeam(pstTeam);

        //被置换的TeamNode需要从Map中删除
        this->DelTeamFromMap(pstTeam);

        //初始化新的Team
        *pstTeam = rstTeamData;

        //将此节点从时间链表尾移到链表头
        TLIST_DEL(pstSwapNode);
        TLIST_INSERT_NEXT(&m_stTimeListHead, pstSwapNode);
    }

    //加入Map
    this->AddTeamToMap(pstTeam);

    return pstTeam;
}


DT_ASYNC_PVP_PLAYER_TEAM_DATA* AsyncPvpTeamMgr::GetTeamData(uint64_t ullPlayerId, uint64_t ullActionId)
{
    m_oIter = m_oTeamMap.find(ullPlayerId);
    if (m_oIter != m_oTeamMap.end())
    {
        this->_Move2TimeListFirst(m_oIter->second);
        return m_oIter->second;
    }

    if (ullActionId == 0)
    {
        return NULL;
    }

    //内存中没有找到则向数据库处理线程发消息
    m_stDBReqPkg.m_bOpType = DB_GET;
    m_stDBReqPkg.m_ullActionId = ullActionId;
    m_stDBReqPkg.m_stData.m_ullUin = ullPlayerId;
    m_stDBReqPkg.m_stData.m_bTroopNum = 0;
    m_astWorkThreads[ullPlayerId % m_iWorkThreadNum].SendReqPkg(m_stDBReqPkg);

    return NULL;
}


void AsyncPvpTeamMgr::UpdateTeamData(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData)
{
    m_oIter = m_oTeamMap.find(rstTeamData.m_ullUin);
    if (m_oIter != m_oTeamMap.end())
    {
        DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam = m_oIter->second;
        *pstTeam = rstTeamData;
    }
    else
    {
        //不在内存中，直接去数据库更新
        this->_SaveTeam(&rstTeamData);
    }
}


int AsyncPvpTeamMgr::CreateTeamData(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData, uint64_t ullActionId)
{
    //检查map是否有重名公会
    m_oIter = m_oTeamMap.find(rstTeamData.m_ullUin);
    if (m_oIter != m_oTeamMap.end())
    {
        return ERR_ALREADY_EXISTED;
    }

    //内存中没有找到则向数据库处理线程发消息
    m_stDBReqPkg.m_bOpType = DB_CREATE;
    m_stDBReqPkg.m_ullActionId = ullActionId;
    m_stDBReqPkg.m_stData = rstTeamData;

    m_astWorkThreads[rstTeamData.m_ullUin % m_iWorkThreadNum].SendReqPkg(m_stDBReqPkg);

    return ERR_NONE;
}


void AsyncPvpTeamMgr::Fini()
{
    //将公会信息回写数据库
    DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam = NULL;
    m_oUptIter.Begin();
    for (int i=0; !m_oUptIter.IsEnd(); m_oUptIter.Next(), i++)
    {
        if (i % TEAM_UPT_NUM_PER_FRAME == 0)
        {
            MsSleep(10);
        }

        TeamNode* pstTeamNode = m_oUptIter.CurrItem();
        if (NULL == pstTeamNode)
        {
            LOGERR_r("In AsyncPvpTeamMgr fini, pstTeamNode is null");
            break;
        }

        pstTeam = &(pstTeamNode->m_oTeam);
        this->_SaveTeam(pstTeam);
    }

    //等待线程结束
    for( int i = 0; i < m_iWorkThreadNum; i++ )
    {
        m_astWorkThreads[i].FiniThread();
    }

    return;
}


void AsyncPvpTeamMgr::_HandleThreadMsg()
{
    for (int i=0; i < m_iWorkThreadNum; i++)
    {
        TeamDBThread& rstWorkThread = m_astWorkThreads[i];
        int iRecvBytes = rstWorkThread.Recv(CThreadFrame::MAIN_THREAD);
        if (iRecvBytes < 0)
        {
            LOGERR_r("Main Thread Recv Msg Failed, iRecvBytes=%d", iRecvBytes);
            continue;
        }
        else if (0 == iRecvBytes)
        {
            continue;
        }

        MyTdrBuf* pstRecvBuf = rstWorkThread.GetRecvBuf(CThreadFrame::MAIN_THREAD);
        TdrError::ErrorType iRet = m_stDBRspPkg.unpack(pstRecvBuf->m_szTdrBuf, pstRecvBuf->m_uPackLen);
        if (iRet != TdrError::TDR_NO_ERROR)
        {
            LOGERR_r("Main Thread unpack pkg failed! errno : %d", iRet);
            continue;
        }

        switch(m_stDBRspPkg.m_bOpType)
        {
            case DB_CREATE:
                this->_HandleCreateMsg(m_stDBRspPkg.m_stData, m_stDBRspPkg.m_nErrNo, m_stDBRspPkg.m_ullActionId);
                break;
            case DB_GET:
                this->_HandleGetMsg(m_stDBRspPkg.m_stData, m_stDBRspPkg.m_nErrNo, m_stDBRspPkg.m_ullActionId);
                break;
            default:
                LOGERR_r("AsyncPvpTeamMgr handle thread msg error, msg type(%d) error", m_stDBRspPkg.m_bOpType);
                break;
        }
    }
}


void AsyncPvpTeamMgr::_HandleCreateMsg(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData, int iErrNo, uint64_t ullActionId)
{
    DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam = NULL;

    if (iErrNo != ERR_NONE)
    {
        LOGERR_r("Create Player(%lu) Team failed", rstTeamData.m_ullUin);
    }
    else
    {
        pstTeam = this->_NewTeam(rstTeamData);
    }

    AsyncPvpTransFrame::Instance().AsyncActionDone(ullActionId, pstTeam, sizeof(pstTeam));
}


void AsyncPvpTeamMgr::_HandleGetMsg(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData, int iErrNo, uint64_t ullActionId)
{
    DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam = this->GetTeamData(rstTeamData.m_ullUin);

    if (iErrNo != ERR_NONE)
    {
        LOGERR_r("Get Player(%lu) Team failed, iRet=%d", rstTeamData.m_ullUin, iErrNo);
    }
    else if (pstTeam == NULL)
    {
        pstTeam = this->_NewTeam(rstTeamData);
    }

    AsyncPvpTransFrame::Instance().AsyncActionDone(ullActionId, pstTeam, sizeof(pstTeam));
}


void AsyncPvpTeamMgr::_SaveTeam(DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam)
{
    if (pstTeam == NULL)
    {
        LOGERR_r("Save Team Failed. pstTeam is null.");
        return;
    }
    m_stDBReqPkg.m_bOpType = DB_UPDATE;
    m_stDBReqPkg.m_stData = *pstTeam;
    m_astWorkThreads[pstTeam->m_ullUin % m_iWorkThreadNum].SendReqPkg(m_stDBReqPkg);
}


void AsyncPvpTeamMgr::AddTeamToMap(DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam)
{
    m_oIter = m_oTeamMap.find(pstTeam->m_ullUin);
    if (m_oIter == m_oTeamMap.end())
    {
        m_oTeamMap.insert(TeamIdMap_t::value_type(pstTeam->m_ullUin, pstTeam));
    }
    else
    {
        LOGERR_r("Add Player Team to IdMap failed, PlayerId=(%lu)", pstTeam->m_ullUin);
    }
}


void AsyncPvpTeamMgr::DelTeamFromMap(DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam)
{
    m_oIter = m_oTeamMap.find(pstTeam->m_ullUin);
    if (m_oIter != m_oTeamMap.end())
    {
        m_oTeamMap.erase(m_oIter);
    }
    else
    {
        LOGERR_r("Del Player Team from IdMap failed, PlayerId=(%lu) ", pstTeam->m_ullUin);
    }
}


void AsyncPvpTeamMgr::AddToDirtyList(DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam)
{
    TeamNode* pstTeamNode = container_of(pstTeam, TeamNode, m_oTeam);
    TLISTNODE* pstListNode = &(pstTeamNode->m_stDirtyListNode);

    //已经加入DirtyList,不能重复加入
    if (!TLIST_IS_EMPTY(pstListNode))
    {
        return;
    }

    //加入DirtyList表头
    TLIST_INSERT_NEXT(&m_stDirtyListHead, pstListNode);
    m_iDirtyNodeNum++;

    LOGRUN_r("Add Player Team to DirtyList, PlayerId=(%lu), Time=(%lu) ", pstTeam->m_ullUin, CGameTime::Instance().GetCurrSecond());
}


void AsyncPvpTeamMgr::_WriteDirtyToDB()
{
    TLISTNODE* pstPos = NULL;
    TLISTNODE* pstNext = NULL;
    TLISTNODE* pstHead = &(m_stDirtyListHead);

    TeamNode* pstTeamNode = NULL;
    DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam = NULL;

    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();

    if (m_iDirtyNodeNum >= m_iDirtyNodeMax ||
        ullCurTime - m_ullLastWriteTimestamp >= (uint64_t)m_iWriteTimeVal)
    {
        m_ullLastWriteTimestamp = ullCurTime;

        TLIST_FOR_EACH_SAFE(pstPos, pstNext, pstHead)
        {
            pstTeamNode = TLIST_ENTRY(pstPos, TeamNode, m_stDirtyListNode);
            pstTeam = &(pstTeamNode->m_oTeam);
            this->_SaveTeam(pstTeam);

            LOGRUN_r("Write DirtyList to DB, Team PlayerId=(%lu), Time=(%lu) ", pstTeam->m_ullUin, ullCurTime);

            TLIST_INIT(pstPos);
        }

        TLIST_INIT(pstHead);
        m_iDirtyNodeNum = 0;
    }

}


void AsyncPvpTeamMgr::_Move2TimeListFirst(DT_ASYNC_PVP_PLAYER_TEAM_DATA* pstTeam)
{
    TeamNode* pstTeamNode = container_of(pstTeam, TeamNode, m_oTeam);
    TLISTNODE* pstListNode = &pstTeamNode->m_stTimeListNode;

    //将此节点移到时间链表头
    TLIST_DEL(pstListNode);
    TLIST_INSERT_NEXT(&m_stTimeListHead, pstListNode);
}

