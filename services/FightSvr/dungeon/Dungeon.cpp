#include "LogMacros.h"
#include "GameTime.h"
#include "FakeRandom.h"
#include "Dungeon.h"
#include "DungeonMgr.h"
#include "DungeonLogic.h"
#include "DungeonStateMachine.h"
#include "../framework/GameObjectPool.h"
#include "../framework/FightSvrMsgLayer.h"
#include "../gamedata/GameDataMgr.h"
#include "../player/Player.h"
#include "../player/PlayerMgr.h"
#include "ChooseMgr.h"
#include "FightCheck.h"
#include "../player/PlayerLogic.h"
#include "../module/fightobj/FightPlayer.h"

using namespace PKGMETA;

int Dungeon::ChooseOrder[8] = {2, 1, 2, 2, 2, 2, 2, 1};

int Dungeon::GeneralCmp(const void *pstFirst, const void *pstSecond)
{
    DT_RANK_GENERAL_INFO* pstItemFirst = (DT_RANK_GENERAL_INFO*)pstFirst;
    DT_RANK_GENERAL_INFO* pstItemSecond = (DT_RANK_GENERAL_INFO*)pstSecond;

    int iResult = pstItemFirst->m_dwGeneralID - pstItemSecond->m_dwGeneralID;
    return iResult;
}

int Dungeon::MSkillCmp(const void *pstFirst, const void *pstSecond)
{
    DT_ITEM_MSKILL* pstItemFirst = (DT_ITEM_MSKILL*)pstFirst;
    DT_ITEM_MSKILL* pstItemSecond = (DT_ITEM_MSKILL*)pstSecond;

    int iResult = pstItemFirst->m_bId - pstItemSecond->m_bId;
    return iResult;
}

Dungeon::Dungeon()
{
    this->_Construct();
}

Dungeon::~Dungeon()
{
    this->Clear();
}

void Dungeon::Clear()
{
    this->_Construct();
    IObject::Clear();
}

void Dungeon::_Construct()
{
    MapId2FightObj_t::iterator iter;
    iter = m_mapId2FightPlayer.begin();
    for (; iter != m_mapId2FightPlayer.end(); iter++)
    {
        FightPlayer::Release((FightPlayer*)iter->second);
    }
    m_mapId2FightPlayer.clear();
    m_listFightPlayer.clear();

    iter = m_mapId2City.begin();
    for (; iter != m_mapId2City.end(); iter++)
    {
        City::Release((City*)iter->second);
    }
    m_mapId2City.clear();
    m_listCity.clear();

    iter = m_mapId2Troop.begin();
    for (; iter != m_mapId2Troop.end(); iter++)
    {
        Troop::Release((Troop*)iter->second);
    }
    m_mapId2Troop.clear();

    m_listTroopDown.clear();
    m_listTroopUp.clear();

    m_listTroopDead.clear();
    m_listTroopAmbush.clear();

    iter = m_mapId2Barrier.begin();
    for (; iter != m_mapId2Barrier.end(); iter++)
    {
        Barrier::Release((Barrier*)iter->second);
    }
    m_mapId2Barrier.clear();
    m_listBarrier.clear();

    iter = m_mapId2Tower.begin();
    for (; iter != m_mapId2Tower.end(); iter++)
    {
        Tower::Release((Tower*)iter->second);
    }
    m_mapId2Tower.clear();
    m_listTower.clear();

    iter = m_mapId2Catapult.begin();
    for (; iter != m_mapId2Catapult.end(); iter++)
    {
        Catapult::Release((Catapult*)iter->second);
    }
    m_mapId2Catapult.clear();
    m_listCatapult.clear();

    m_iTimeSyncCnt = 0;

    m_dwDungeonId = 0;
    m_bTerrainId = 0;

    m_iState = DUNGEON_STATE_NONE;

    m_iDungeonTime = 0;

    m_stArenaSize = Vector3::zero();

    m_bGeneralCount = 0;
    m_bChooseGroup = PLAYER_GROUP_UP;
    m_bChooseCount = 0;
    m_bOnlinePlayerCnt = 0;

    // 过滤器重置
    m_oFilterManager.Clear();

    _ClearListCsPkgWaitDeal();

    m_ullDelayDeadLastTime = 0;
}

void Dungeon::_ClearListCsPkgWaitDeal()
{

    ListCsPkg_t::iterator iter = m_listCsPkgWaitDeal.begin();
    if (!m_listCsPkgWaitDeal.empty())
    {
        LOGWARN("Don't clear the m_listCsPkgWaitDeal, size=<%d> fix it!", (int)m_listCsPkgWaitDeal.size());
    }
    for (; iter != m_listCsPkgWaitDeal.end(); /*iter++*/)
    {

        delete (*iter);
        (*iter) = NULL;
        iter = m_listCsPkgWaitDeal.erase(iter);
    }
    //m_listCsPkgWaitDeal.clear();

}

Dungeon* Dungeon::Get()
{
    return GET_GAMEOBJECT(Dungeon, GAMEOBJ_DUNGEON);
}

void Dungeon::Release( Dungeon* pObj )
{
    RELEASE_GAMEOBJECT(pObj);
}

//TODO:具体的生成规则
void Dungeon::GenChooseGeneral()
{
    ChooseMgr::Instance().GetGeneralChooseList(m_bGeneralCount, m_GeneralList);

    ChooseMgr::Instance().GetMSkillChooseList(m_bMSkillCount, m_MSkillList);
}

int Dungeon::FindGeneral(uint32_t dwGeneralId)
{
    DT_RANK_GENERAL_INFO stGeneralInfo;
    stGeneralInfo.m_dwGeneralID = dwGeneralId;

    int iEqual = 0;
    int iIndex = MyBSearch(&stGeneralInfo, &m_GeneralList, m_bGeneralCount, sizeof(DT_RANK_GENERAL_INFO), &iEqual, Dungeon::GeneralCmp);
    if (!iEqual)
    {
        return -1;
    }

    return iIndex;
}

void Dungeon::DelGeneral(uint32_t dwGeneralId)
{
    DT_RANK_GENERAL_INFO stGeneralInfo;
    stGeneralInfo.m_dwGeneralID = dwGeneralId;

    size_t nmemb = m_bGeneralCount;
    MyBDelete(&stGeneralInfo, &m_GeneralList, &nmemb, sizeof(DT_RANK_GENERAL_INFO), Dungeon::GeneralCmp);
    m_bGeneralCount = (uint8_t)nmemb;
}

int Dungeon::FindMSkill(uint8_t bMSkillId)
{
    DT_ITEM_MSKILL stMSkillInfo;
    stMSkillInfo.m_bId = bMSkillId;

    int iEqual = 0;
    int iIndex = MyBSearch(&stMSkillInfo, &m_MSkillList, m_bMSkillCount, sizeof(DT_ITEM_MSKILL), &iEqual, Dungeon::MSkillCmp);
    if (!iEqual)
    {
        return -1;
    }

    return iIndex;
}


void Dungeon::InitOneTroop(FightPlayer* poFightPlayer, DT_TROOP_INFO& rstToopInfo)
{
    DT_FIGHT_PLAYER_INFO& rPlayerInfo = poFightPlayer->m_stPlayerInfo;

    Troop* poTroop = Troop::Get();
    if (rPlayerInfo.m_chGroup == PLAYER_GROUP_DOWN)
    {
        // 回城卡片状态
        rstToopInfo.m_stInitPos.m_iPosX = 0;
        rstToopInfo.m_stInitPos.m_iPosY = -2000;
    }
    else
    {
        // 回城卡片状态
        rstToopInfo.m_stInitPos.m_iPosX = 0;
        rstToopInfo.m_stInitPos.m_iPosY = 10000;
    }

    poTroop->Init(this, poFightPlayer, rstToopInfo);
    m_mapId2Troop.insert(MapId2FightObj_t::value_type(poTroop->m_bId, poTroop));

    // 栅栏
    // 至多三个栅栏
    if (MAX_BARRIER_NUM > poFightPlayer->m_dwBarrierCount && poTroop->HasSkill(SKILL_BARRIER))
    {
        Barrier* poBarrier = Barrier::Get();

        poBarrier->Init(this, poFightPlayer, poTroop->m_bId, poTroop->m_poResGeneral->m_bBarrierHp);
        m_mapId2Barrier.insert(MapId2FightObj_t::value_type(poBarrier->m_bId, poBarrier));
        m_listBarrier.push_back(poBarrier);
        ++poFightPlayer->m_dwBarrierCount;
    }

    if (rPlayerInfo.m_chGroup == PKGMETA::PLAYER_GROUP_DOWN)
    {
        m_listTroopDown.push_back(poTroop);
    }
    else if (rPlayerInfo.m_chGroup == PKGMETA::PLAYER_GROUP_UP)
    {
        m_listTroopUp.push_back(poTroop);
    }

}

void Dungeon::_InitTroopInfo(FightPlayer* poFightPlayer, DT_FIGHT_PLAYER_INFO& rPlayerInfo)
{
    //初始化部队
    for (int j=0; j < rPlayerInfo.m_bTroopNum; j++)
    {
        DT_TROOP_INFO& rTroopInfo = rPlayerInfo.m_astTroopList[j];
        rTroopInfo.m_bId = MAX_TROOP_NUM * rPlayerInfo.m_chGroup + j + 1;

        InitOneTroop(poFightPlayer, rTroopInfo);
    }
}

bool Dungeon::Init(DT_FIGHT_DUNGEON_INFO* pDungeonInfo)
{
    // 对时次数初始化
    m_iTimeSyncCnt = TimeSyncCnt;

    // 匹配类型
    m_bMatchType = pDungeonInfo->m_bMatchType;
    m_bFakeType = pDungeonInfo->m_bFakeType;

    m_bMaxPlayer = m_bFakeType == MATCH_FAKE_NONE ? 2 : 1;

    //统帅值增加士气
    RESBASIC* pResBasicMorale = CGameDataMgr::Instance().GetResBasicMgr().Find(9402);
    float fLeaderValueMorale = pResBasicMorale->m_para[0];
    int iGroup = -1;

    if (m_bFakeType == MATCH_FAKE_NONE)
    {
        // 副本时间初始化
        ResBasicMgr_t& rResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
        RESBASIC* poBasicTime = (m_bMatchType == MATCH_TYPE_DAILY_CHALLENGE) ?
    		rResBasicMgr.Find(DAILY_CHALLENGE_PARA) : rResBasicMgr.Find(BASIC_TIME);

        if (poBasicTime != NULL)
        {
            m_iDungeonTime = (int)(poBasicTime->m_para[0] * 1000);
        }
        else
        {
            LOGERR("poBasicTime is null");
            return false;
        }
        m_iTimeLeft4Fight = m_iDungeonTime;

        // 副本信息初始化
        RESBASIC* poBasicArenaSize = rResBasicMgr.Find(BASIC_ARENA_SIZE);
        m_stArenaSize.set(poBasicArenaSize->m_para[0], poBasicArenaSize->m_para[1], poBasicArenaSize->m_para[2]);

        RESBASIC* poBasicTerrainInfo = rResBasicMgr.Find(BASIC_TERRAIN_INFO);
        pDungeonInfo->m_bTerrainId = (uint8_t)CFakeRandom::Instance().Random(1, (uint32_t)poBasicTerrainInfo->m_para[0]);
        m_bTerrainId = pDungeonInfo->m_bTerrainId;
        pDungeonInfo->m_ullTimeStamp = CGameTime::Instance().GetCurrTimeMs();
        this->m_ullTimestamp = pDungeonInfo->m_ullTimeStamp;

        //统帅值增加士气
        if (pDungeonInfo->m_astFightPlayerList[0].m_dwLeaderValue > pDungeonInfo->m_astFightPlayerList[1].m_dwLeaderValue)
        {
            iGroup = 0;
        }
        else if (pDungeonInfo->m_astFightPlayerList[0].m_dwLeaderValue < pDungeonInfo->m_astFightPlayerList[1].m_dwLeaderValue)
        {
            iGroup = 1;
        }

        // 投石车信息
        Catapult* poCatapult = Catapult::Get();
        poCatapult->Init(this, 1);
        m_mapId2Catapult.insert(MapId2FightObj_t::value_type(poCatapult->m_bId, poCatapult));
        m_listCatapult.push_back(poCatapult);

        // 防作弊信息
        m_stLastAttackTimeInfo.m_bCount = 0;

    }

    for (int i=0; i<pDungeonInfo->m_bFightPlayerNum; i++)
    {
        DT_FIGHT_PLAYER_INFO& rPlayerInfo = pDungeonInfo->m_astFightPlayerList[i];
        // 战场玩家
        FightPlayer* poFightPlayer = FightPlayer::Get();
        poFightPlayer->Init(this, rPlayerInfo);
        m_mapId2FightPlayer.insert(MapId2FightObj_t::value_type(poFightPlayer->m_bId, poFightPlayer));
        m_listFightPlayer.push_back(poFightPlayer);

        if (m_bFakeType == MATCH_FAKE_NONE)
        {
            if (iGroup == -1 || i == iGroup)
            {
                poFightPlayer->ChgMorale(fLeaderValueMorale);
            }

            // 部队信息
            _InitTroopInfo(poFightPlayer, rPlayerInfo);

            // 城墙信息
            City* poCity = City::Get();
            poCity->Init(this, poFightPlayer);
            m_mapId2City.insert(MapId2FightObj_t::value_type(poCity->m_bId, poCity));
            m_listCity.push_back(poCity);

            // 瞭望塔信息
            int iCnt = 2;
            for (int i=0; i<iCnt; i++)
            {
                Tower* poTower = Tower::Get();
                poTower->Init(this, poFightPlayer, rPlayerInfo.m_chGroup * iCnt + i + 1);
                m_mapId2Tower.insert(MapId2FightObj_t::value_type(poTower->m_bId, poTower));
                m_listTower.push_back(poTower);
            }
        }
    }

    // 主动切到副本第一个状态
    DungeonStateMachine::Instance().ChangeState(this, DUNGEON_STATE_WAIT_CONNECT);

    LOGRUN("Dungeon init finished, DungeonId(%u), timestamp(%lu), MatchType(%d)",
            m_dwDungeonId, m_ullTimestamp, m_bMatchType);

    return true;
}

void Dungeon::Update(int iDeltaTime)
{
    if (m_iState >= DUNGEON_STATE_WAIT_LOADING)
    {
        // 对时协议
        if (m_iTimeSyncCnt-- > 0)
        {
            DungeonLogic::Instance().SyncServerTime(this);
        }
    }

    // 状态机Update
    DungeonStateMachine::Instance().Update(this, iDeltaTime);

    ListFightObj_t::iterator iter = m_listTroopDown.begin();
    for (; iter!=m_listTroopDown.end(); iter++)
    {
        (*iter)->Update(iDeltaTime);
    }

    iter = m_listTroopUp.begin();
    for (; iter!=m_listTroopUp.end(); iter++)
    {
        (*iter)->Update(iDeltaTime);
    }
}

void Dungeon::Broadcast(PKGMETA::SCPKG* pstScPkg, Player* poPlayerExclude, char cSessCmd)
{
    MapId2FightObj_t::iterator iter = m_mapId2FightPlayer.begin();
    for (; iter != m_mapId2FightPlayer.end(); iter++)
    {
        FightPlayer* poFightPlayer = (FightPlayer*)iter->second;
        Player* poPlayer = poFightPlayer->m_poPlayer;
        if (poPlayer != NULL)
        {
            if (poPlayerExclude == NULL || poPlayer != poPlayerExclude)
            {
                FightSvrMsgLayer::Instance().SendToClient(poPlayer, pstScPkg, cSessCmd);
            }
        }
    }
}

void Dungeon::BroadcastWithoutAck(PKGMETA::SCPKG* pstScPkg, Player* poPlayerExclude, char cSessCmd)
{
    MapId2FightObj_t::iterator iter = m_mapId2FightPlayer.begin();
    for (; iter != m_mapId2FightPlayer.end(); iter++)
    {
        FightPlayer* poFightPlayer = (FightPlayer*)iter->second;
        Player* poPlayer = poFightPlayer->m_poPlayer;
        if (poPlayer != NULL)
        {
            if (poPlayerExclude == NULL || poPlayer != poPlayerExclude)
            {
                FightSvrMsgLayer::Instance().SendToClientWithoutAck(poPlayer, pstScPkg, cSessCmd);
            }
        }
    }
}

int8_t Dungeon::GetGroupOpposite(int8_t chGroup)
{
    if (chGroup == PLAYER_GROUP_DOWN)
    {
        return PLAYER_GROUP_UP;
    }
    else if (chGroup == PLAYER_GROUP_UP)
    {
        return PLAYER_GROUP_DOWN;
    }

    return -1;
}

int8_t Dungeon::GetGroupByPlayerPtr(Player* poPlayer)
{
    MapId2FightObj_t::iterator iter = m_mapId2FightPlayer.begin();
    for (; iter != m_mapId2FightPlayer.end(); iter++)
    {
        FightPlayer* poFightPlayer = (FightPlayer*)iter->second;
        if (poFightPlayer->m_poPlayer == poPlayer)
        {
            return poFightPlayer->m_chGroup;
        }
    }

    return -1;
}

Player* Dungeon::GetPlayerByGroup(int8_t chGroup)
{
    Player* ret = NULL;
    MapId2FightObj_t::iterator iter = m_mapId2FightPlayer.begin();
    for (; iter != m_mapId2FightPlayer.end(); iter++)
    {
        FightPlayer* poFightPlayer = (FightPlayer*)iter->second;
        if (poFightPlayer->m_chGroup == chGroup)
        {
            ret = poFightPlayer->m_poPlayer;
            break;
        }
    }

    return ret;
}

void Dungeon::SetFightPlayerByUin(uint64_t ullUin, Player* poPlayer)
{
    FightPlayer* poFightPlayer = this->GetFightPlayerByUin(ullUin);
    if (poFightPlayer)
    {
        if (poPlayer && !poFightPlayer->m_poPlayer)
        {
            m_bOnlinePlayerCnt++;
        }

        if (!poPlayer && poFightPlayer->m_poPlayer)
        {
            m_bOnlinePlayerCnt--;

        }

        poFightPlayer->m_poPlayer = poPlayer;
    }
}

FightPlayer* Dungeon::GetFightPlayerByGroup(int8_t chGroup)
{
    return this->GetFightPlayerById(chGroup);
}

FightPlayer* Dungeon::GetFightPlayerById(uint8_t bId)
{
    MapId2FightObj_t::iterator it = m_mapId2FightPlayer.find(bId);
    if (it != m_mapId2FightPlayer.end())
    {
        return (FightPlayer*)it->second;
    }

    return NULL;
}

FightPlayer* Dungeon::GetFightPlayerByUin(uint64_t ullUin)
{
    FightPlayer* poFightPlayer;
    ListFightObj_t::iterator iter = m_listFightPlayer.begin();
    for (; iter!=m_listFightPlayer.end(); iter++)
    {
        poFightPlayer = (FightPlayer*)(*iter);
        if (poFightPlayer->m_ullUin == ullUin)
        {
            return poFightPlayer;
        }
    }

    return NULL;
}


City* Dungeon::GetCityById(uint8_t bId)
{
    MapId2FightObj_t::iterator it = m_mapId2City.find(bId);
    if (it != m_mapId2City.end())
    {
        return (City*)it->second;
    }

    return NULL;
}

Troop* Dungeon::GetTroopById(uint8_t bId)
{
    MapId2FightObj_t::iterator it = m_mapId2Troop.find(bId);
    if (it != m_mapId2Troop.end())
    {
        return (Troop*)it->second;
    }

    return NULL;
}

ListFightObj_t* Dungeon::GetTroopListByGroup(int8_t chGroup)
{
    if (chGroup == PKGMETA::PLAYER_GROUP_DOWN)
    {
        return &m_listTroopDown;
    }
    else if (chGroup == PKGMETA::PLAYER_GROUP_UP)
    {
        return &m_listTroopUp;
    }
    else
    {
        return NULL;
    }
}

Barrier* Dungeon::GetBarrierById(uint8_t bId)
{
    MapId2FightObj_t::iterator it = m_mapId2Barrier.find(bId);
    if (it != m_mapId2Barrier.end())
    {
        return (Barrier*)it->second;
    }

    return NULL;
}

Tower* Dungeon::GetTowerById(uint8_t bId)
{
    MapId2FightObj_t::iterator it = m_mapId2Tower.find(bId);
    if (it != m_mapId2Tower.end())
    {
        return (Tower*)it->second;
    }

    return NULL;
}

Catapult* Dungeon::GetCatapultById(uint8_t bId)
{
    MapId2FightObj_t::iterator it = m_mapId2Catapult.find(bId);
    if (it != m_mapId2Catapult.end())
    {
        return (Catapult*)it->second;
    }

    return NULL;
}

FightObj* Dungeon::GetFightObj(DT_FIGHTOBJ& stFightObj)
{
    FightObj* poRet = NULL;
    switch (stFightObj.m_chType)
    {
    case FIGHTOBJ_PLAYER:
        {
            poRet = this->GetFightPlayerById(stFightObj.m_bId);
            break;
        }
    case FIGHTOBJ_WALL:
        {
            poRet = this->GetCityById(stFightObj.m_bId);
            break;
        }
    case FIGHTOBJ_TROOP:
        {
            poRet = this->GetTroopById(stFightObj.m_bId);
            break;
        }
    case FIGHTOBJ_TOWER:
        {
            poRet = this->GetTowerById(stFightObj.m_bId);
            break;
        }
    case FIGHTOBJ_BARRIER:
        {
            poRet = this->GetBarrierById(stFightObj.m_bId);
            break;
        }

    default:
        break;
    }

    return poRet;
}

bool Dungeon::FightReady(Player* poPlayer)
{
    size_t ulReadyCnt = 0;
    MapId2FightObj_t::iterator iter = m_mapId2FightPlayer.begin();
    for (; iter != m_mapId2FightPlayer.end(); iter++)
    {
        FightPlayer* poFightPlayer = (FightPlayer*)iter->second;
        if (poFightPlayer->m_poPlayer == poPlayer)
        {
            poFightPlayer->m_bReady = true;
        }

        if (poFightPlayer->m_bReady)
        {
            ulReadyCnt++;
        }
    }

    if (ulReadyCnt == m_mapId2FightPlayer.size())
    {
        return true;
    }

    return false;
}

bool Dungeon::SoloSettle(PKGMETA::DT_SOLO_SETTLE_INFO* pSoloSettleInfo, int8_t& chWinnerGroup)
{
#if 0
    // 判断是否已收到对手的单挑按键成绩
    if (m_chSoloSettleGroup == PLAYER_GROUP_NONE && pSoloSettleInfo != NULL)
    {
        if (pSoloSettleInfo == NULL)
        {
            return false;
        }
        else
        {
            // 保存当前成绩
            if (pSoloSettleInfo->m_bScoreCnt > MAX_SOLO_SCORE_NUM)
            {
                LOGERR("m_bScoreCnt > MAX_SOLO_SCORE_NUM");
                return false;
            }

            m_chSoloSettleGroup = pSoloSettleInfo->m_stSoloObj.m_chGroup;
            m_stSoloSettleInfo = *pSoloSettleInfo;

            return false;
        }
    }
    else
    {
        chWinnerGroup = PLAYER_GROUP_NONE;
        if (pSoloSettleInfo == NULL)
        {
            for (int i=0; i<m_stSoloSettleInfo.m_bScoreCnt; i++)
            {
                if (m_stSoloSettleInfo.m_szScoreList[i] > 0)
                {
                    chWinnerGroup = m_chSoloSettleGroup;
                    break;
                }
            }
        }
        else
        {
            // 比较双方成绩，分别发送己方成绩给对方
            // 胜3 平1 负0
            int iWinScoreA = 0; // 当前网络发起者玩家
            int iWinScoreB = 0;

            for (int i=0; i<m_stSoloSettleInfo.m_bScoreCnt; i++)
            {
                if (m_stSoloSettleInfo.m_szScoreList[i] == pSoloSettleInfo->m_szScoreList[i])
                {
                    iWinScoreA += 1;
                    iWinScoreB += 1;
                }
                else if (m_stSoloSettleInfo.m_szScoreList[i] > pSoloSettleInfo->m_szScoreList[i])
                {
                    iWinScoreA += 3;
                }
                else
                {
                    iWinScoreB += 3;
                }
            }

            if (iWinScoreA > iWinScoreB)
            {
                chWinnerGroup = m_chSoloSettleGroup;
            }
            else if (iWinScoreA < iWinScoreB)
            {
                chWinnerGroup = pSoloSettleInfo->m_stSoloObj.m_chGroup;
            }
        }

        // 清空保存的单挑结算信息
        m_chSoloSettleGroup = PLAYER_GROUP_NONE;

        return true;
    }
#endif
    return true;
}

bool Dungeon::HasTroopDead()
{
    return !m_listTroopDead.empty();
}

void Dungeon::AddTroopDead(Troop* poTroop)
{
    if (poTroop == NULL)
    {
        return;
    }

    // 应该不会同时存在两个一样的，可以判断一下
#if 1
    ListFightObj_t::iterator iter = std::find(m_listTroopDead.begin(), m_listTroopDead.end(), poTroop);
    if (iter != m_listTroopDead.end())
    {
        LOGERR("troop id<%d> dead twice.", poTroop->m_bId);
    }
#endif

    m_listTroopDead.push_back(poTroop);

    if (m_iState == DUNGEON_STATE_FIGHT)
    {
        // 切到死亡状态
        DungeonStateMachine::Instance().ChangeState(this, DUNGEON_STATE_LOCK_DEAD);
    }
}

void Dungeon::DelTroopDead(Troop* poTroop)
{
    if (m_listTroopDead.size() > 0)
    {
        if (poTroop == m_listTroopDead.front())
        {
            m_listTroopDead.pop_front();
        }
    }

    if (m_listTroopDead.empty())
    {
        if (m_iState == DUNGEON_STATE_LOCK_DEAD)
        {
            // 切到正常战斗
            DungeonStateMachine::Instance().ChangeState(this, DUNGEON_STATE_FIGHT);
        }
    }
}

void Dungeon::ClearTroopDead()
{
    m_listTroopDead.clear();
}

bool Dungeon::IsAllTroopDead(int8_t chGroup, FightObj* poTroopExclude /*= NULL*/)
{
    MapId2FightObj_t::iterator iter = m_mapId2Troop.begin();
    for (; iter != m_mapId2Troop.end(); iter ++)
    {
        if (iter->second->m_chGroup == chGroup && iter->second != poTroopExclude && iter->second->m_iHpCur > 0)
        {
            return false;
        }
    }

    return true;
}

bool Dungeon::HasTroopAmbush()
{
    return !m_listTroopAmbush.empty();
}

void Dungeon::AddTroopAmbush(Troop* poTroop)
{
    if (poTroop == NULL)
    {
        return;
    }

    // 应该不会同时存在两个一样的，可以判断一下
#if 1
    ListFightObj_t::iterator iter = std::find(m_listTroopAmbush.begin(), m_listTroopAmbush.end(), poTroop);
    if (iter != m_listTroopAmbush.end())
    {
        LOGERR("troop id<%d> ambush twice.", poTroop->m_bId);
    }
#endif

    m_listTroopAmbush.push_back(poTroop);

    if (m_iState == DUNGEON_STATE_FIGHT)
    {
        // 切到伏兵状态
        DungeonStateMachine::Instance().ChangeState(this, DUNGEON_STATE_LOCK_AMBUSH);
    }
}

void Dungeon::DelTroopAmbush(Troop* poTroop)
{
    if (m_listTroopAmbush.size() > 0)
    {
        if (poTroop == m_listTroopAmbush.front())
        {
            m_listTroopAmbush.pop_front();
        }
    }

    if (m_listTroopAmbush.empty())
    {
        if (m_iState == DUNGEON_STATE_LOCK_AMBUSH)
        {
            // 切到正常战斗
            DungeonStateMachine::Instance().ChangeState(this, DUNGEON_STATE_FIGHT);
        }
    }
}

void Dungeon::ClearTroopAmbush()
{
    m_listTroopAmbush.clear();
}

void Dungeon::ClearPlayer()
{
    MapId2FightObj_t::iterator iter = m_mapId2FightPlayer.begin();
    for (; iter != m_mapId2FightPlayer.end(); iter++)
    {
        FightPlayer* poFightPlayer = (FightPlayer*)iter->second;
        Player* poPlayer = poFightPlayer->m_poPlayer;
        if (poPlayer != NULL)
        {
            // 该玩家还在本副本中
            PlayerMgr::Instance().Delete(poPlayer);
        }
    }
}

void Dungeon::PushCsPkgWaitDeal(uint32_t dwSessionId, PKGMETA::CSPKG& roPkg)
{
    CsPkgInfo* pstScPkgInfo =new CsPkgInfo(DungeonMgr::Instance().GetWaitDealBufferLen());
    pstScPkgInfo->m_bLast = false;
    pstScPkgInfo->m_dwSessionId = dwSessionId;
    TdrError::ErrorType iRet = roPkg.pack(pstScPkgInfo->m_stCsSendBuf.m_szTdrBuf, pstScPkgInfo->m_stCsSendBuf.m_uSize);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        return;
    }

    m_listCsPkgWaitDeal.push_back(pstScPkgInfo);
}

void Dungeon::DealCsPkgWaitDeal()
{
    // 待处理包链表为空，直接返回
    if (m_listCsPkgWaitDeal.empty())
    {
        return;
    }

    // 有可能待处理的报文也没法立即处理，需要继续缓存
    // 记录该次处理的最后一个报文
    CsPkgInfo& rPkg = *m_listCsPkgWaitDeal.back();
    rPkg.m_bLast = true;

    bool bLast;
    PKGMETA::CONNSESSION stSession;
    PKGMETA::CSPKG stCsPkg;
    ListCsPkg_t::iterator iter = m_listCsPkgWaitDeal.begin();
    for (; iter != m_listCsPkgWaitDeal.end(); /*iter++*/)
    {
        stCsPkg.unpack((*iter)->m_stCsSendBuf.m_szTdrBuf, (*iter)->m_stCsSendBuf.m_uSize);
        bLast = (*iter)->m_bLast;
        stSession.m_dwSessionId = (*iter)->m_dwSessionId;

        IMsgBase* poMsgHandler = FightSvrMsgLayer::Instance().GetClientMsgHandler(stCsPkg.m_stHead.m_wMsgId);
        if (poMsgHandler != NULL)
        {
            poMsgHandler->HandleClientMsg(&stSession, stCsPkg, NULL);
            LOGRUN("handle delay pkg. MsgId<%d>", stCsPkg.m_stHead.m_wMsgId);
        }

        // 处理一个，删一个，没处理成功的插入到后面
        delete (*iter);
        *iter = NULL;
        iter = m_listCsPkgWaitDeal.erase(iter);

        if (bLast)
        {
            // 处理完该次循环的最后一个包
            break;
        }
    }
}

void Dungeon::UpdateMorale(int iDeltaTime)
{
    MapId2FightObj_t::iterator iter = m_mapId2FightPlayer.begin();
    for (; iter != m_mapId2FightPlayer.end(); iter++)
    {
        iter->second->Update(iDeltaTime);
    }
}

void Dungeon::ChgMorale(int8_t chGroup, float fMorale)
{
    FightPlayer* poFightPlayer = GetFightPlayerById(chGroup);

    if (poFightPlayer != NULL)
    {
        poFightPlayer->ChgMorale(fMorale);
    }
}

bool Dungeon::CheckAttackFreq(DT_HP_INFO& rHpInfo)
{
    FightObj* poSource = this->GetFightObj(rHpInfo.m_stSourceObj);
    FightObj* poTarget = this->GetFightObj(rHpInfo.m_stTargetObj);
    if(poSource == NULL || poTarget == NULL)
    {
        LOGERR("poSource or poTarget is NULL");
        return true;
    }

    Troop* poTroop = this->GetTroopById(poSource->m_bId);
    if (poTroop == NULL)
    {
        LOGERR("poTroop is null.");
        return true;
    }
    // 设置最短攻击间隔
    FightCheck::Instance().SetMinDetaTime(poTroop->m_poResArmy->m_fAttackCityCD*1000);

    if(!FightCheck::Instance().CheckCityWallAttFreq(poSource, poTarget, rHpInfo.m_nValueChgType, m_stLastAttackTimeInfo))
    {
        FightCheck::Instance().SendKickPlayerNtf(poSource->m_poFightPlayer->m_ullUin, poSource->m_poFightPlayer->m_iZoneSvrProcId);
        PlayerLogic::Instance().PlayerLogout(poSource->m_poFightPlayer->m_poPlayer, PLAYER_LOGOUT_BY_CONN);
        return false;
    }

    return true;
}

