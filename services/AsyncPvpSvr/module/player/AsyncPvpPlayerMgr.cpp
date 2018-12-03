#include "AsyncPvpPlayerMgr.h"
#include "AsyncPvpTransFrame.h"
#include "GameDataMgr.h"

using namespace PKGMETA;

bool AsyncPvpPlayerMgr::Init(ASYNCPVPSVRCFG * pstConfig)
{
    if (pstConfig == NULL)
    {
       return false;
    }

    //��ʼ�����������
    m_iDirtyNodeNum = 0;
    m_iDirtyNodeMax = pstConfig->m_iDirtyNodeMax;
    m_iWriteTimeVal = pstConfig->m_iWriteTimeVal;
    m_ullLastWriteTimestamp = CGameTime::Instance().GetCurrSecond();
    TLIST_INIT(&m_stDirtyListHead);

    //��ʼ��mempool���
    m_iCurSize = 0;
    m_iMaxSize = pstConfig->m_iPlayerMax;
    if (m_oPlayerPool.CreatePool(m_iMaxSize) < 0)
    {
       LOGERR_r("Create Player pool MaxNum(%d) failed.", m_iMaxSize);
       return false;
    }
    m_oPlayerPool.RegisterSlicedIter(&m_oUptIter);

    //��ʼ�������߳�
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
           LOGERR_r("Init thread<%d> failed", i);
           return false;
       }
    }

    //��ʼ��ʱ������
    TLIST_INIT(&m_stTimeListHead);

    m_oPlayerMap.clear();

    return true;
}


void AsyncPvpPlayerMgr::Update()
{
    this->_HandleThreadMsg();

    this->_WriteDirtyToDB();
}


AsyncPvpPlayer* AsyncPvpPlayerMgr::_NewPlayer(DT_ASYNC_PVP_PLAYER_WHOLE_DATA& rstPlayerData)
{
    AsyncPvpPlayer* poPlayer = NULL;
    PlayerNode* pstPlayerNode = NULL;

    //mempool���п���ʱ
    if (m_iCurSize < m_iMaxSize)
    {
        pstPlayerNode = m_oPlayerPool.NewData();
        if (NULL == pstPlayerNode)
        {
            LOGERR_r("Get PlayerNode from PlayerPool failed, CurPlayer=%d, Pool UsedSize=%d",
                      m_iCurSize, m_oPlayerPool.GetUsedNum());
            return NULL;
        }

        //��ʼ��Player
        poPlayer = &(pstPlayerNode->m_oPlayer);
        poPlayer->InitFromDB(rstPlayerData);

        //����ʱ������ͷ
        TLIST_INSERT_NEXT(&m_stTimeListHead, &(pstPlayerNode->m_stTimeListNode));
        m_iCurSize++;
    }
    //mempool��û�п���ʱ����Ҫ�û����û�����LRU�㷨�����������ʹ�õĽڵ��д�����ݿ�
    else
    {
        //ȡʱ������β�ڵ�
        TLISTNODE* pstSwapNode = TLIST_PREV(&m_stTimeListHead);
        pstPlayerNode = container_of(pstSwapNode, PlayerNode, m_stTimeListNode);

        //���û���PlayerNode��Ҫ��д���ݿ�
        poPlayer = &(pstPlayerNode->m_oPlayer);
        this->_SavePlayer(poPlayer);

        //���û���PlayerNode��Ҫ��Map��ɾ��
        this->DelPlayerFromMap(poPlayer);

        //��ʼ���µ�Player
        poPlayer->InitFromDB(rstPlayerData);

        //���˽ڵ��ʱ������β�Ƶ�����ͷ
        TLIST_DEL(pstSwapNode);
        TLIST_INSERT_NEXT(&m_stTimeListHead, pstSwapNode);
    }

    //����Map
    this->AddPlayerToMap(poPlayer);

    return poPlayer;
}


AsyncPvpPlayer* AsyncPvpPlayerMgr::GetPlayer(uint64_t ullPlayerId, uint64_t ullActionId)
{
    m_oIter = m_oPlayerMap.find(ullPlayerId);
    if (m_oIter != m_oPlayerMap.end())
    {
        //��Player������,����ʱ������ͷ
        this->_Move2TimeListFirst(m_oIter->second);
        return m_oIter->second;
    }

    //����ң�����ȥ���ݿ���ѯ��ֱ�Ӵ���
    if (ullPlayerId <= MAX_NUM_ASYNC_PVP_FAKE_PLAYER_UIN)
    {
        return this->_GenFakePlayer(ullPlayerId);
    }

    if (ullActionId == 0)
    {
        return NULL;
    }

    //�ڴ���û���ҵ��������ݿ⴦���̷߳���Ϣ
    m_stDBReqPkg.m_bOpType = DB_GET;
    m_stDBReqPkg.m_ullActionId = ullActionId;
    m_stDBReqPkg.m_stData.m_stShowdata.m_stBaseInfo.m_ullUin = ullPlayerId;
    m_stDBReqPkg.m_stData.m_stShowdata.m_bCount = 0;
    m_stDBReqPkg.m_stData.m_bOpponentCount = 0;
    m_stDBReqPkg.m_stData.m_bRecordCount = 0;
    m_astWorkThreads[ullPlayerId % m_iWorkThreadNum].SendReqPkg(m_stDBReqPkg);

    return NULL;
}


int AsyncPvpPlayerMgr::CreatePlayer(DT_ASYNC_PVP_PLAYER_SHOW_DATA& rstShowData, uint64_t ullActionId)
{
    //���map�Ƿ����������
    m_oIter = m_oPlayerMap.find(rstShowData.m_stBaseInfo.m_ullUin);
    if (m_oIter != m_oPlayerMap.end())
    {
        return ERR_ALREADY_EXISTED;
    }

    //�ڴ���û���ҵ��������ݿ⴦���̷߳���Ϣ
    m_stDBReqPkg.m_bOpType = DB_CREATE;
    m_stDBReqPkg.m_ullActionId = ullActionId;
    DT_ASYNC_PVP_PLAYER_WHOLE_DATA& rstPlayerData = m_stDBReqPkg.m_stData;
    rstPlayerData.m_stShowdata = rstShowData;
    rstPlayerData.m_bOpponentCount = 0;
    rstPlayerData.m_bRecordCount = 0;

    m_astWorkThreads[rstShowData.m_stBaseInfo.m_ullUin % m_iWorkThreadNum].SendReqPkg(m_stDBReqPkg);

    return ERR_NONE;
}


void AsyncPvpPlayerMgr::DelPlayer(uint64_t ullPlayerId)
{
    return;
}


void AsyncPvpPlayerMgr::UpdatePlayer(DT_ASYNC_PVP_PLAYER_SHOW_DATA& rstShowData)
{
    //����ң�������
    if (rstShowData.m_stBaseInfo.m_ullUin <= MAX_NUM_ASYNC_PVP_FAKE_PLAYER_UIN)
    {
        return;
    }

    m_oIter = m_oPlayerMap.find(rstShowData.m_stBaseInfo.m_ullUin);
    if (m_oIter != m_oPlayerMap.end())
    {
        //���ڴ��У������ڴ�����
        AsyncPvpPlayer* poPlayer = m_oIter->second;
        poPlayer->UptShowData(rstShowData);
    }
    else
    {
        m_stDBReqPkg.m_bOpType = DB_UPDATE_PART;
        m_stDBReqPkg.m_stData.m_stShowdata = rstShowData;
        m_astWorkThreads[rstShowData.m_stBaseInfo.m_ullUin % m_iWorkThreadNum].SendReqPkg(m_stDBReqPkg);
    }
}


void AsyncPvpPlayerMgr::Fini()
{
    //�������Ϣ��д���ݿ�
    AsyncPvpPlayer* poPlayer = NULL;
    m_oUptIter.Begin();
    for (int i=0; !m_oUptIter.IsEnd(); m_oUptIter.Next(), i++)
    {
        if (i % PLAYER_UPT_NUM_PER_FRAME == 0)
        {
            MsSleep(10);
        }

        PlayerNode* pstPlayerNode = m_oUptIter.CurrItem();
        if (NULL == pstPlayerNode)
        {
            LOGERR_r("In AsyncPvpPlayerMgr fini, pstPlayerNode is null");
            break;
        }

        poPlayer = &(pstPlayerNode->m_oPlayer);
        this->_SavePlayer(poPlayer);
    }

    //�ȴ��߳̽���
    for( int i = 0; i < m_iWorkThreadNum; i++ )
    {
        m_astWorkThreads[i].FiniThread();
    }

    return;
}


void AsyncPvpPlayerMgr::_HandleThreadMsg()
{
    for (int i=0; i < m_iWorkThreadNum; i++)
    {
        PlayerDBThread& rstWorkThread = m_astWorkThreads[i];
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
                LOGERR_r("AsyncPvpPlayerMgr handle thread msg error, msg type(%d) error", m_stDBRspPkg.m_bOpType);
                break;
        }
    }
}


void AsyncPvpPlayerMgr::_HandleCreateMsg(DT_ASYNC_PVP_PLAYER_WHOLE_DATA& rstPlayerData, int iErrNo, uint64_t ullActionId)
{
    AsyncPvpPlayer* poPlayer = NULL;

    if (iErrNo != ERR_NONE)
    {
        LOGERR_r("Create Player(%lu) failed", rstPlayerData.m_stShowdata.m_stBaseInfo.m_ullUin);
    }
    else
    {
        poPlayer = this->_NewPlayer(rstPlayerData);
        //����Ҹտ�ʼ��Ҫˢ��һ�ζ����б�
        poPlayer->RefreshOpponentList();
    }

    AsyncPvpTransFrame::Instance().AsyncActionDone(ullActionId, poPlayer, sizeof(poPlayer));
}


void AsyncPvpPlayerMgr::_HandleGetMsg(DT_ASYNC_PVP_PLAYER_WHOLE_DATA& rstPlayerData, int iErrNo, uint64_t ullActionId)
{
    AsyncPvpPlayer* poPlayer = this->GetPlayer(rstPlayerData.m_stShowdata.m_stBaseInfo.m_ullUin);

    if (iErrNo != ERR_NONE)
    {
        LOGERR_r("Get Player(%lu) failed, iRet=%d", rstPlayerData.m_stShowdata.m_stBaseInfo.m_ullUin, iErrNo);
    }
    else if (poPlayer == NULL)
    {
        poPlayer = this->_NewPlayer(rstPlayerData);
    }

    AsyncPvpTransFrame::Instance().AsyncActionDone(ullActionId, poPlayer, sizeof(poPlayer));
}


void AsyncPvpPlayerMgr::_SavePlayer(AsyncPvpPlayer* poPlayer)
{
    if (poPlayer == NULL)
    {
        LOGERR_r("poPlayer is NULL. Save Player failed.");
        return;
    }
    if (poPlayer->GetPlayerId() <= MAX_NUM_ASYNC_PVP_FAKE_PLAYER_UIN)
    {
        return;
    }

    m_stDBReqPkg.m_bOpType = DB_UPDATE;
    poPlayer->GetWholeData(m_stDBReqPkg.m_stData);
    m_astWorkThreads[poPlayer->GetPlayerId() % m_iWorkThreadNum].SendReqPkg(m_stDBReqPkg);
}


AsyncPvpPlayer* AsyncPvpPlayerMgr::_GenFakePlayer(uint64_t ullPlayerId)
{
    ResFakePlayerMgr_t& rstResFakePlayerMgr = CGameDataMgr::Instance().GetResFakePlayerMgr();
    RESASYNCPVPFAKEPLAYER* pResFakePlayer = rstResFakePlayerMgr.Find(ullPlayerId);
    assert(pResFakePlayer);

    DT_ASYNC_PVP_PLAYER_WHOLE_DATA stPlayerData;
    stPlayerData.m_bRecordCount = 0;
    stPlayerData.m_bOpponentCount = 0;

    DT_ASYNC_PVP_PLAYER_SHOW_DATA& rstShowData = stPlayerData.m_stShowdata;
    rstShowData.m_stBaseInfo.m_ullUin = ullPlayerId;
    rstShowData.m_stBaseInfo.m_bLv = pResFakePlayer->m_bLevel;
    rstShowData.m_stBaseInfo.m_dwLi = pResFakePlayer->m_dwLi;
    rstShowData.m_stBaseInfo.m_wHeadIconId = pResFakePlayer->m_wHeadIconId;
    rstShowData.m_stBaseInfo.m_wHeadFrameId = pResFakePlayer->m_wHeadFrameId;
    rstShowData.m_stBaseInfo.m_dwWinCnt = 0;
    StrCpy(rstShowData.m_stBaseInfo.m_szName, pResFakePlayer->m_szName, MAX_NAME_LENGTH);
    rstShowData.m_dwMSkillId = 1;
    rstShowData.m_bMSkillLevel = 1;
    rstShowData.m_bCount = pResFakePlayer->m_bTroopNum;
    for (uint8_t i = 0; i < rstShowData.m_bCount; i++)
    {
        rstShowData.m_astGeneralList[i].m_dwGeneralID = pResFakePlayer->m_troopList[i];
        rstShowData.m_astGeneralList[i].m_bStar = 3;
        rstShowData.m_astGeneralList[i].m_bGrade = 3;
        rstShowData.m_astGeneralList[i].m_bLv = pResFakePlayer->m_bLevel;
    }
	rstShowData.m_bVipLv = 0;
	rstShowData.m_bIsWorshipped = 0;
    rstShowData.m_bTacticsType = 0;
    rstShowData.m_bTacticsLevel = 0;
    rstShowData.m_dwLeaderValue = 0;

    return this->_NewPlayer(stPlayerData);
}


void AsyncPvpPlayerMgr::AddPlayerToMap(AsyncPvpPlayer* poPlayer)
{
    m_oIter = m_oPlayerMap.find(poPlayer->GetPlayerId());
    if (m_oIter == m_oPlayerMap.end())
    {
        m_oPlayerMap.insert(PlayerIdMap_t::value_type(poPlayer->GetPlayerId(), poPlayer));
    }
    else
    {
        LOGERR_r("Add Player to IdMap failed, PlayerId=(%lu)", poPlayer->GetPlayerId());
    }
}


void AsyncPvpPlayerMgr::DelPlayerFromMap(AsyncPvpPlayer* poPlayer)
{
    poPlayer->Clear();

    m_oIter = m_oPlayerMap.find(poPlayer->GetPlayerId());
    if (m_oIter != m_oPlayerMap.end())
    {
        m_oPlayerMap.erase(m_oIter);
    }
    else
    {
        LOGERR_r("Del Player from IdMap failed, PlayerId=(%lu) ", poPlayer->GetPlayerId());
    }
}


void AsyncPvpPlayerMgr::AddToDirtyList(AsyncPvpPlayer* poPlayer)
{
    //��������ݲ��û�д
    if (poPlayer->GetPlayerId() <= MAX_NUM_ASYNC_PVP_FAKE_PLAYER_UIN)
    {
        return;
    }

    PlayerNode* pstPlayerNode = container_of(poPlayer, PlayerNode, m_oPlayer);
    TLISTNODE* pstListNode = &(pstPlayerNode->m_stDirtyListNode);

    //�Ѿ�����DirtyList,�����ظ�����
    if (!TLIST_IS_EMPTY(pstListNode))
    {
        return;
    }

    //����DirtyList��ͷ
    TLIST_INSERT_NEXT(&m_stDirtyListHead, pstListNode);
    m_iDirtyNodeNum++;

    LOGRUN_r("Add Player to DirtyList, PlayerId=(%lu), Time=(%lu), Player MemAddr(%p) ",
                poPlayer->GetPlayerId(), CGameTime::Instance().GetCurrSecond(), poPlayer);
}


void AsyncPvpPlayerMgr::_WriteDirtyToDB()
{
    TLISTNODE* pstPos = NULL;
    TLISTNODE* pstNext = NULL;
    TLISTNODE* pstHead = &(m_stDirtyListHead);

    PlayerNode* pstPlayerNode = NULL;
    AsyncPvpPlayer* poPlayer = NULL;

    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();

    if (m_iDirtyNodeNum >= m_iDirtyNodeMax ||
        ullCurTime - m_ullLastWriteTimestamp >= (uint64_t)m_iWriteTimeVal)
    {
        m_ullLastWriteTimestamp = ullCurTime;

        TLIST_FOR_EACH_SAFE(pstPos, pstNext, pstHead)
        {
            pstPlayerNode = TLIST_ENTRY(pstPos, PlayerNode, m_stDirtyListNode);
            poPlayer = &(pstPlayerNode->m_oPlayer);
            this->_SavePlayer(poPlayer);

            LOGRUN_r("Write DirtyList to DB, PlayerId=(%lu), Time=(%lu) ", poPlayer->GetPlayerId(), ullCurTime);

            TLIST_INIT(pstPos);
        }

        TLIST_INIT(pstHead);
        m_iDirtyNodeNum = 0;
    }

}


void AsyncPvpPlayerMgr::_Move2TimeListFirst(AsyncPvpPlayer* poPlayer)
{
    PlayerNode* pstPlayerNode = container_of(poPlayer, PlayerNode, m_oPlayer);
    TLISTNODE* pstListNode = &pstPlayerNode->m_stTimeListNode;

    //���˽ڵ��Ƶ�ʱ������ͷ
    TLIST_DEL(pstListNode);
    TLIST_INSERT_NEXT(&m_stTimeListHead, pstListNode);
}

