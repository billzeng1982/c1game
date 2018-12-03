#include "AsyncPvpFightMgr.h"
#include "GameTime.h"
#include "AsyncPvpRank.h"
#include "AsyncPvpSvrMsgLayer.h"

using namespace PKGMETA;

bool AsyncPvpFightMgr::Init()
{
    if (!m_oDungeonPool.Init(DUNGEON_INIT_NUM, DUNGEON_DELTA_NUM))
    {
        LOGERR_r("AsyncPvpFightMgr init pool failed");
        return false;
    }

    m_ullLastUptTimestamp = CGameTime::Instance().GetCurrSecond();

    TLIST_INIT(&m_stTimeListHead);

    return true;
}


void AsyncPvpFightMgr::Update()
{
    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();
    if (ullCurTime == m_ullLastUptTimestamp)
    {
        return;
    }
    m_ullLastUptTimestamp = ullCurTime;

    DT_ASYNC_PVP_FIGHT_RECORD stRecord;

    //结算已经超时的dungeon
    while (!TLIST_IS_EMPTY(&m_stTimeListHead))
    {
        TLISTNODE* pstNode = TLIST_NEXT(&m_stTimeListHead);
        Dungeon* poDungeon = container_of(pstNode, Dungeon, m_stTimeListNode);

        if (ullCurTime - poDungeon->m_ullTimestamp > FIGHT_TIME)
        {
            //超时结算
            this->Settle(poDungeon->m_ullRecordNo, PLAYER_GROUP_UP, stRecord);
        }
        else
        {
            break;
        }
    };
}


int AsyncPvpFightMgr::Create(AsyncPvpPlayer* poAttacker, AsyncPvpPlayer* poDefender, uint64_t* pRecordNo)
{
    if (poAttacker->IsInFight())
    {
        return ERR_SELF_IN_FIGHT;
    }

    if (poDefender->IsInFight())
    {
        return ERR_OPPONENT_IN_FIGHT;
    }

    Dungeon* poDungeon = m_oDungeonPool.Get();

    poDungeon->m_ullRecordNo = this->_GenRecordNo();
    if (poDungeon->m_ullRecordNo == 0)
    {
        return ERR_SYS;
    }
    poDungeon->m_ullTimestamp = CGameTime::Instance().GetCurrSecond();
    poDungeon->poAttacker = poAttacker;
    poDungeon->poDefender = poDefender;
    TLIST_INIT(&poDungeon->m_stTimeListNode);
    TLIST_INSERT_PREV(&m_stTimeListHead, &poDungeon->m_stTimeListNode);

    m_oDungeonMap.insert(Id2DungeonMap_t::value_type(poDungeon->m_ullRecordNo, poDungeon));

    *pRecordNo = poDungeon->m_ullRecordNo;

    //设置为正在战斗状态
    poAttacker->SetInFight(true);
    poDefender->SetInFight(true);

    return ERR_NONE;
}


int AsyncPvpFightMgr::Settle(uint64_t ullRecordNo, uint8_t bWinGroup, DT_ASYNC_PVP_FIGHT_RECORD& rstRecord)
{
    m_oIter = m_oDungeonMap.find(ullRecordNo);
    if (m_oIter == m_oDungeonMap.end())
    {
        return ERR_NOT_FOUND;
    }
    Dungeon* poDungeon = m_oIter->second;

    if (bWinGroup == PLAYER_GROUP_DOWN)
    {
        rstRecord.m_ullWinPlayerId = poDungeon->poAttacker->GetPlayerId();
    }
    else
    {
        rstRecord.m_ullWinPlayerId = poDungeon->poDefender->GetPlayerId();
    }

    rstRecord.m_ullRecordNo = ullRecordNo;
    rstRecord.m_ullTimestamp = poDungeon->m_ullTimestamp;

    poDungeon->poAttacker->GetBaseData(rstRecord.m_astPlayerList[0]);
    poDungeon->poDefender->GetBaseData(rstRecord.m_astPlayerList[1]);

    //将战报加入玩家战斗记录
    
    poDungeon->poDefender->AddRecord(rstRecord);
    poDungeon->poAttacker->AddRecord(rstRecord);

    //当进攻方获胜且进攻方排名低于防守方排名时，交换两者的名次
    if ((bWinGroup == PLAYER_GROUP_DOWN) &&
       poDungeon->poAttacker->GetRank() > poDungeon->poDefender->GetRank())
    {
        AsyncPvpRank::Instance().SwapRank(poDungeon->poAttacker, poDungeon->poDefender);

        //当交换名次时，被挑战者的对阵表需要刷新
        if (poDungeon->poDefender->GetPlayerId() > MAX_NUM_ASYNC_PVP_FAKE_PLAYER_UIN)
        {
            m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_REFRESH_OPPONENT_REQ;
            m_stSsPkg.m_stHead.m_ullUin = poDungeon->poDefender->GetPlayerId();
            SS_PKG_ASYNC_PVP_REFRESH_OPPONENT_REQ& rstSsPkgReq = m_stSsPkg.m_stBody.m_stAsyncpvpRefreshOpponentReq;
            rstSsPkgReq.m_ullUin = poDungeon->poDefender->GetPlayerId();
            rstSsPkgReq.m_bIsAuto = 1;
            AsyncPvpSvrMsgLayer::Instance().DealSvrPkg(m_stSsPkg);
        }
    }

    poDungeon->poAttacker->SetInFight(false);
    poDungeon->poDefender->SetInFight(false);

    //释放内存
    TLIST_DEL(&poDungeon->m_stTimeListNode);
    m_oDungeonMap.erase(m_oIter);
    m_oDungeonPool.Release(poDungeon);

    return ERR_NONE;
}


uint64_t AsyncPvpFightMgr::_GenRecordNo()
{
    static uint64_t ullLastGenTime = CGameTime::Instance().GetCurrSecond();

    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();

    if (ullLastGenTime != ullCurTime)
    {
        ullLastGenTime = ullCurTime;
        m_bSeq = 0;
    }
    else
    {
        if (++m_bSeq >= MAX_NUM_CREATE_ONE_SEC)
        {
            return 0;
        }
    }

    //ID生成算法
    // 30位(时间戳,单位:秒)  |  8位(当前时间戳内的序号)
    uint64_t ullTimestamp = ullLastGenTime - BASE_TIMESTAMP;

    uint64_t ullRecordNo = ullTimestamp << 8;
    ullRecordNo |= m_bSeq ;

    return ullRecordNo;
}

