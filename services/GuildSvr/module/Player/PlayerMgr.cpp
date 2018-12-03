#include "PlayerMgr.h"
#include "common_proto.h"
#include "LogMacros.h"
#include "../../transaction/GuildTransFrame.h"
#include "../../gamedata/GameDataMgr.h"
#include "GameTime.h"

using namespace PKGMETA;

PlayerMgr::PlayerMgr()
{
}

PlayerMgr::~PlayerMgr()
{
}

bool PlayerMgr::Init(GUILDSVRCFG * pstConfig)
{
    if (pstConfig == NULL)
    {
        return false;
    }

    //初始化脏数据相关
    m_iDirtyNodeNum = 0;
    m_iDirtyNodeMax = pstConfig->m_iDirtyNodeMax;
    m_iWriteTimeVal = pstConfig->m_iWriteTimeVal * 1000;
    m_ullLastWriteTimestamp = CGameTime::Instance().GetCurrTimeMs();
    TLIST_INIT(&m_stDirtyListHead);

    //初始化时间链表
    TLIST_INIT(&m_stTimeListHead);

    //初始化mempool相关
    m_iCurSize = 0;
    m_iMaxSize = pstConfig->m_iPlayerMaxNum;
    if (m_oPlayerPool.CreatePool(m_iMaxSize) < 0)
    {
        LOGERR_r("Create Player pool[num=%d] failed.", m_iMaxSize);
        return false;
    }
    m_oPlayerPool.RegisterSlicedIter(&m_oUptIter);

    //初始化工作线程
    m_iWorkThreadNum = pstConfig->m_iWorkThreadNum;
    m_astWorkThreads = new PlayerDBThread[m_iWorkThreadNum];
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

    //初始化 更新时间 相关
    ResBasicMgr_t& rstResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* pResBasic = rstResBasicMgr.Find(COMMON_UPDATE_TIME);
    if (pResBasic == NULL)
    {
        LOGERR_r("PlayerMgr Init fail, pResBasic is NULL");
        return false;
    }
    m_iApplyUptTime = (int)(pResBasic->m_para[0]);

    m_ullLastUptTimeStamp = CGameTime::Instance().GetSecOfHourInCurrDay(m_iApplyUptTime) * 1000;
    if (CGameTime::Instance().GetCurrHour() < m_iApplyUptTime)
    {
        m_ullLastUptTimeStamp -= MSSECONDS_OF_DAY;
    }
    m_bUptFlag = false;

    return true;
}

void PlayerMgr::Fini()
{
    //将玩家信息回写到数据库
    m_oUptIter.Begin();
    for (int i=0; !m_oUptIter.IsEnd(); m_oUptIter.Next(), i++)
    {
        if (i % PLAYER_UPT_NUM_PER_SECOND == 0)
        {
            MsSleep(10);
        }

        PlayerNode* pstPlayerNode = m_oUptIter.CurrItem();
        if (NULL == pstPlayerNode)
        {
            LOGERR_r("pstPlayerNode is null");
            break;
        }

        Player* poPlayer = &pstPlayerNode->m_oPlayer;
        _SavePlayer(poPlayer);
    }

    //等待线程结束
    for( int i = 0; i < m_iWorkThreadNum; i++ )
    {
        m_astWorkThreads[i].FiniThread();
    }

    LOGRUN_r("PlayerMgr Fini success");

    return;
}


void PlayerMgr::Update()
{
    //处理线程消息
    _HandleThreadMsg();

    //将脏数据写入数据库
    _WriteDirtyToDB();

    //更新申请列表更新的时间戳
    _UpdateTimeStamp();
}


Player* PlayerMgr::NewPlayer(DT_GUILD_PLAYER_DATA& rstPlayerInfo)
{
    Player* poPlayer = NULL;
    PlayerNode* pstPlayerNode = NULL;
    //当mempool不满时
    if (m_iCurSize< m_iMaxSize)
    {
        pstPlayerNode = m_oPlayerPool.NewData();
        if (NULL == pstPlayerNode)
        {
            LOGERR_r("pstPlayerNode is NULL, Get new PlayerNode from pool failed");
            return NULL;
        }

        poPlayer = &pstPlayerNode->m_oPlayer;
        poPlayer->InitFromDB(rstPlayerInfo);

        //加入时间链表
        TLIST_INSERT_NEXT(&m_stTimeListHead, &pstPlayerNode->m_stTimeListNode);
        m_iCurSize++;
    }
    //当mempool满时，需要置换
    else
    {
        //取时间链表尾节点
        TLISTNODE* pstSwapNode  = TLIST_PREV(&m_stTimeListHead);
        pstPlayerNode = container_of(pstSwapNode, PlayerNode, m_stTimeListNode);

        //被置换的节点需要回写数据库
        poPlayer = &pstPlayerNode->m_oPlayer;
        _SavePlayer(poPlayer);

        //被置换的节点需要从Map中删除
        this->DelPlayerFromMap(poPlayer);

        //初始化新的node
        poPlayer->InitFromDB(rstPlayerInfo);

        //将此节点从时间链表尾移到链表头
        TLIST_DEL(pstSwapNode);
        TLIST_INSERT_NEXT(&m_stTimeListHead, pstSwapNode);
    }

    //加入map
    AddPlayerToMap(poPlayer);

    return poPlayer;
}

void PlayerMgr::_HandleThreadMsg()
{
    for (int i=0; i < m_iWorkThreadNum; i++)
    {
        PlayerDBThread& rstWorkThread = m_astWorkThreads[i];
        int iRecvBytes = rstWorkThread.Recv(CThreadFrame::MAIN_THREAD);
        if (iRecvBytes < 0)
        {
            LOGERR_r("Main Thread Recv Failed, iRecvBytes=%d", iRecvBytes);
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
            LOGERR_r("unpack pkg failed! errno : %d", iRet);
            continue;
        }

        switch(m_stDBRspPkg.m_bType)
        {
            case GUILD_DB_GET:
                this->_HandleThreadGetMsg(m_stDBRspPkg.m_stData, m_stDBRspPkg.m_nErrNo, m_stDBRspPkg.m_ullActionId);
                break;
            default:
                break;
        }
    }
}


void PlayerMgr::_HandleThreadGetMsg(DT_GUILD_PLAYER_DATA& rstPlayerInfo, int iErrNo, uint64_t ullActionId)
{
    assert(ullActionId != 0);

    uint64_t ullPlayerId = rstPlayerInfo.m_stBaseInfo.m_ullUin;
    Player* poPlayer = GetPlayer(ullPlayerId);

    if (iErrNo != ERR_NONE)
    {
        LOGERR_r("Get Player(%lu) failed", ullPlayerId);
    }
    else if(poPlayer==NULL)
    {
        poPlayer = NewPlayer(rstPlayerInfo);
    }

    GuildTransFrame::Instance().AsyncActionDone(ullActionId, poPlayer, sizeof(DT_GUILD_PLAYER_DATA));

    return;
}

void PlayerMgr::_UpdateTimeStamp()
{
    int iHour = CGameTime::Instance().GetCurrHour();

    if ((iHour == m_iApplyUptTime) && m_bUptFlag)
    {
        m_bUptFlag = false;
        m_ullLastUptTimeStamp = CGameTime::Instance().GetCurrTimeMs();
    }
    else if (iHour != m_iApplyUptTime)
    {
        m_bUptFlag = true;
    }
}

Player* PlayerMgr::GetPlayer(uint64_t ullPlayerId, TActionToken ullTokenId)
{
    //先在内存中找
    m_IdToPlayerMapIter = m_IdToPlayerMap.find(ullPlayerId);
    if (m_IdToPlayerMapIter != m_IdToPlayerMap.end())
    {
        Move2TimeListFirst(m_IdToPlayerMapIter->second);
        return m_IdToPlayerMapIter->second;
    }

    if (ullTokenId == 0)
    {
        return NULL;
    }

    //内存中没有找到则向数据库处理线程发消息
    bzero(&m_stDBReqPkg, sizeof(m_stDBReqPkg));
    m_stDBReqPkg.m_bType = GUILD_DB_GET;
    m_stDBReqPkg.m_ullActionId = ullTokenId;
    m_stDBReqPkg.m_stData.m_stBaseInfo.m_ullUin= ullPlayerId;
    m_astWorkThreads[ullPlayerId % m_iWorkThreadNum].SendReqPkg(m_stDBReqPkg);

    return NULL;
}

void PlayerMgr::Move2TimeListFirst(Player* poPlayer)
{
    PlayerNode* pstPlayerNode = container_of(poPlayer, PlayerNode, m_oPlayer);
    TLISTNODE* pstNode = &pstPlayerNode->m_stTimeListNode;

    //将此节点移到时间链表头
    TLIST_DEL(pstNode);
    TLIST_INSERT_NEXT(&m_stTimeListHead, pstNode);
}

void PlayerMgr::_SavePlayer(Player* poPlayer)
{
    m_stDBReqPkg.m_bType = GUILD_DB_UPDATE;
    
    if (poPlayer->PackPlayerData(m_stDBReqPkg.m_stData, 0) == false)
    {
        LOGERR_r("Guild<%lu> Player<%lu> pack failded.", poPlayer->GetGuildId(), poPlayer->GetPlayerId());
        return;
    }
    m_astWorkThreads[poPlayer->GetPlayerId() % m_iWorkThreadNum].SendReqPkg(m_stDBReqPkg);
}

void PlayerMgr::AddToDirtyList(Player* poPlayer)
{
    PlayerNode* pstPlayerNode = container_of(poPlayer, PlayerNode, m_oPlayer);
    TLISTNODE* pstNode = &pstPlayerNode->m_stDirtyListNode;

    //已经加入DirtyList,不能重复加入
    if (!TLIST_IS_EMPTY(pstNode))
    {
        return;
    }

    //加入DirtyList表头
    TLIST_INSERT_NEXT(&m_stDirtyListHead, pstNode);
    m_iDirtyNodeNum++;

    LOGRUN_r("Add Player to DirtyList, PlayerId=(%lu), Time=(%lu) ",
                poPlayer->GetPlayerId(), CGameTime::Instance().GetCurrSecond());
}

void PlayerMgr::_WriteDirtyToDB()
{
    TLISTNODE* pstPos = NULL;
    TLISTNODE* pstNext = NULL;
    TLISTNODE* pstHead = &(m_stDirtyListHead);

    PlayerNode* pstPlayerNode = NULL;
    Player* poPlayer = NULL;

    uint64_t ullCurTime = CGameTime::Instance().GetCurrTimeMs();

    if (m_iDirtyNodeNum >= m_iDirtyNodeMax ||
        ullCurTime - m_ullLastWriteTimestamp >= (uint64_t)m_iWriteTimeVal)
    {
        m_ullLastWriteTimestamp = ullCurTime;

        TLIST_FOR_EACH_SAFE(pstPos, pstNext, pstHead)
        {
            pstPlayerNode = TLIST_ENTRY(pstPos, PlayerNode, m_stDirtyListNode);
            poPlayer = &pstPlayerNode->m_oPlayer;
            _SavePlayer(poPlayer);

            LOGRUN_r("Write DirtyList to DB, PlayerId=(%lu), Time=(%lu) ",
                      poPlayer->GetPlayerId(), ullCurTime/1000);

            TLIST_INIT(pstPos);
        }

        TLIST_INIT(pstHead);
        m_iDirtyNodeNum = 0;
    }

}

void PlayerMgr::AddPlayerToMap(Player* poPlayer)
{
    //加入IdMap
    m_IdToPlayerMapIter = m_IdToPlayerMap.find(poPlayer->GetPlayerId());
    if (m_IdToPlayerMapIter == m_IdToPlayerMap.end())
    {
        m_IdToPlayerMap.insert(PlayerIdMap_t::value_type(poPlayer->GetPlayerId(), poPlayer));
    }
    else
    {
        LOGERR_r("Add Player to IdMap failed, PlayerId=(%lu)", poPlayer->GetPlayerId());
    }
}

void PlayerMgr::DelPlayerFromMap(Player* poPlayer)
{
    m_IdToPlayerMapIter = m_IdToPlayerMap.find(poPlayer->GetPlayerId());
    if (m_IdToPlayerMapIter != m_IdToPlayerMap.end())
    {
       m_IdToPlayerMap .erase(m_IdToPlayerMapIter);
    }
    else
    {
       LOGERR_r("Del Player from IdMap failed, PlayerId=(%lu)", poPlayer->GetPlayerId());
    }
}

