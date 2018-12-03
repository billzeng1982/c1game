#include <math.h>
#include "GameTime.h"
#include "LogMacros.h"
#include "../../gamedata/GameDataMgr.h"
#include "../../framework/GameObjectPool.h"
#include "../../dungeon/DungeonLogic.h"
#include "../../dungeon/Dungeon.h"
#include "sys/GameTime.h"
#include "Troop.h"
#include "ChooseMgr.h"

using namespace PKGMETA;

int Troop::GeneralInfoCmp(const void *pstFirst, const void *pstSecond)
{
    DT_GENERAL_BRIEF_INFO* pstGeneralFirst = (DT_GENERAL_BRIEF_INFO*)pstFirst;
    DT_GENERAL_BRIEF_INFO* pstGeneralSecond = (DT_GENERAL_BRIEF_INFO*)pstSecond;

    int iResult = (int)pstGeneralFirst->m_dwId - (int)pstGeneralSecond->m_dwId;
    return iResult;
}

int Troop::SkinCmp(const void *pstFirst, const void *pstSecond)
{
    uint32_t dwFirst = *(uint32_t*)pstFirst;
    uint32_t dwSecond = *(uint32_t*)pstSecond;

    if (dwFirst > dwSecond)
    {
        return 1;
    }
    else if (dwSecond > dwFirst)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

Troop::Troop()
{
    this->_Construct();
}

Troop::~Troop()
{
}

void Troop::Clear()
{
    this->_Construct();
    FightObj::Clear();
}

void Troop::_Construct()
{
    m_dwGeneralId = 0;
    m_bGeneralLevel = 0;
    m_bGeneralStar = 0;
    m_bGeneralPhase = 0;
    m_llCurStampTime = 0;
    m_llLastStampTime = 0;
    m_iArmyType = 0;
    m_fCurSpeedRatio = 1.0f;
    m_oGeneral.Clear();
    m_dwActiveSkillId = 0;
    m_bIsRetreat = false;
    m_bActiveSkillLevel = 1;
    m_poTactics = NULL;
    m_bPassiveSkillCnt = 0;
    m_poResGeneral = NULL;
    m_poResArmy = NULL;
    m_poResGeneralStar = NULL;
    m_poResGeneralPhase = NULL;
    m_poResGeneralLevel = NULL;
    m_poResPeakArenaGeneral = NULL;

    for(int i = 0; i < MAX_SKILL_CHEATS_NUM;i++)
    {
        m_dwActiveCheatsLevel[i] = 0;
    }

    m_bActiveSkillCheatsType = 0;


    // 兵种属性
    m_iArmyId = 0;					// 兵种ID，（同种类兵种，等级不同ID不同，图标不同,模型不同）
    m_iArmyLevel = 0;				// 兵种等级
    m_iArmyPhase = 0;               // 兵种品阶
}

Troop* Troop::Get()
{
    return GET_GAMEOBJECT(Troop, GAMEOBJ_TROOP);
}

void Troop::Release(Troop* pObj)
{
    RELEASE_GAMEOBJECT(pObj);
}

void Troop::Update(int dt)
{
    m_oGeneral.Update(dt);
}

bool Troop::Init(Dungeon* poDungeon, FightPlayer* poFightPlayer, PKGMETA::DT_TROOP_INFO& rstTroopInfo)
{
    FightObj::Init(poDungeon, poFightPlayer, FIGHTOBJ_TROOP, rstTroopInfo.m_bId);

    // 武将信息初始化
    m_dwGeneralId = rstTroopInfo.m_stGeneralInfo.m_dwId;
	m_poResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(m_dwGeneralId);
	if (m_poResGeneral == NULL)
	{
		LOGERR("ResGeneral %u not found.", m_dwGeneralId);
		return false;
	}

    m_bGeneralLevel = rstTroopInfo.m_stGeneralInfo.m_bLevel;
    m_bGeneralStar = rstTroopInfo.m_stGeneralInfo.m_bStar;
    m_bGeneralPhase = rstTroopInfo.m_stGeneralInfo.m_bPhase;

	m_iArmyLevel = rstTroopInfo.m_stGeneralInfo.m_bArmyLv;
	m_iArmyPhase = rstTroopInfo.m_stGeneralInfo.m_bArmyPhase;
	m_iArmyId = m_iArmyPhase * 100 + m_poResGeneral->m_bArmyType;

    if (m_bGeneralPhase > RES_MAX_GENERAL_PHASE)
    {
        m_bGeneralPhase = RES_MAX_GENERAL_PHASE;
        LOGERR("GeneralGrade is larger than max");
    }

    // 配置档读取
    m_poResArmy = CGameDataMgr::Instance().GetResArmyMgr().Find(m_iArmyId);
    if (m_poResArmy == NULL)
    {
        LOGERR("ResArmy %u not found.", m_iArmyId);
        return false;
    }

    m_poResGeneralStar = CGameDataMgr::Instance().GetResGeneralStarMgr().Find(m_dwGeneralId);
    if (m_poResGeneralStar == NULL)
    {
        LOGERR("ResGeneralStar %u not found.", m_dwGeneralId);
        return false;
    }

    m_poResGeneralPhase = CGameDataMgr::Instance().GetResGeneralPhaseMgr().Find(m_dwGeneralId);
    if (m_poResGeneralPhase == NULL)
    {
        LOGERR("ResGeneralPhase %u not found.", m_dwGeneralId);
        return false;
    }

    m_poResGeneralLevel = CGameDataMgr::Instance().GetResGeneralLevelMgr().Find(m_dwGeneralId);
    if (m_poResGeneralLevel == NULL)
    {
        LOGERR("ResGeneralLevel %u not found.", m_dwGeneralId);
        return false;
    }

    if (poDungeon->m_bMatchType == MATCH_TYPE_PEAK_ARENA)
    {
        this->_InitPeakArenaTroopData(rstTroopInfo);
        this->_InitPeakArenaTroopSkill(rstTroopInfo);
    }
    else
    {
        this->_InitNormalTroopData(rstTroopInfo);
        this->_InitNormalTroopSkill(rstTroopInfo);
        this->_InitTactics(poFightPlayer);
    }

    // 初始位置
    m_stCurPos.x = rstTroopInfo.m_stInitPos.m_iPosX / 100.0f;

    if(m_chGroup == PLAYER_GROUP_DOWN)
    {
        m_stCurPos.z = 0;
    }
    else
    {
        m_stCurPos.z = m_poDungeon->m_stArenaSize.z;
    }

    m_stDstPos = m_stCurPos;

    // 初始血量
    m_iHpCur = m_oCurrentRtData.m_arrAttrValue[ATTR_HP];

    // 时间初始化
    m_llLastStampTime = (int64_t)(CGameTime::Instance().GetCurrSecond() * 1000 + CGameTime::Instance().GetCurrMsInSec());

    LOGRUN("DungeonId(%u), Player(%s) Init Troop, General=%u, Id=%d,  Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f, ArmyBase=%f",
            poDungeon->m_dwDungeonId, poFightPlayer->m_szName, m_dwGeneralId, rstTroopInfo.m_bId,
            m_oOriginRtData.m_arrAttrValue[ATTR_HP], m_oOriginRtData.m_arrAttrValue[ATTR_STR],
            m_oOriginRtData.m_arrAttrValue[ATTR_WIT],  m_oOriginRtData.m_arrAttrValue[ATTR_STRDEF],
            m_oOriginRtData.m_arrAttrValue[ATTR_WITDEF], m_oOriginRtData.m_arrAttrValue[ATTR_BASE_NORMALATK],
            m_oOriginRtData.m_arrAttrValue[ATTR_BASE_ARMYSKILL]);

    return true;
}

bool Troop::_InitPeakArenaTroopData(PKGMETA::DT_TROOP_INFO& rstTroopInfo)
{
    m_poResPeakArenaGeneral = CGameDataMgr::Instance().GetResPeakArenaGeneralMgr().Find(m_dwGeneralId);
    if (!m_poResPeakArenaGeneral)
    {
        return false;
    }

    this->_InitPeakArenaRtData(m_oOriginRtData);
    this->_InitPeakArenaRtData(m_oCurrentRtData);

    ChooseMgr::ListSkin_t* poListSkin = ChooseMgr::Instance().GetSkinListByGeneral(m_dwGeneralId);
    if (poListSkin)
    {
         DT_FIGHT_PLAYER_INFO& rstFightPlayerInfo = m_poFightPlayer->m_stPlayerInfo;

         ChooseMgr::ListSkin_t::iterator iter = poListSkin->begin();
         for ( ; iter != poListSkin->end(); iter++)
         {
            uint32_t dwSkinId = (*iter);
            int iEqual = 0;
            int iIndex = MyBSearch(&dwSkinId, rstFightPlayerInfo.m_SkinList, rstFightPlayerInfo.m_wSkinCnt, sizeof(uint32_t), &iEqual, Troop::SkinCmp);
            if (!iEqual)
            {
                continue;
            }
            this->_AddSkinAttr(m_oOriginRtData, dwSkinId);
            this->_AddSkinAttr(m_oCurrentRtData, dwSkinId);
         }
    }

    for (int i=0; i<MAX_ATTR_ADD_NUM; i++)
    {
        rstTroopInfo.m_AttrAddValue[i] = m_oOriginRtData.m_arrAttrValue[i];
    }
}

bool Troop::_InitNormalTroopData(PKGMETA::DT_TROOP_INFO& rstTroopInfo)
{
    // 运行时数据初始化
    this->_InitRtData(m_oOriginRtData);
    this->_InitRtData(m_oCurrentRtData);

    // 武将等级加成
    this->_AddGeneralLevel(m_oOriginRtData);
    this->_AddGeneralLevel(m_oCurrentRtData);

    // 装备加成
    this->_AddEquipAttr(m_oOriginRtData, rstTroopInfo);
    this->_AddEquipAttr(m_oCurrentRtData, rstTroopInfo);

    // 武将升阶加成
    this->_AddGeneralPhase(m_oOriginRtData);
    this->_AddGeneralPhase(m_oCurrentRtData);

    //  缘分加成
    float arrOriginGCardFateAtt[MAX_ATTR_ADD_NUM] = { 0 };
    float arrCurrentGCardFateAtt[MAX_ATTR_ADD_NUM] = { 0 };

    this->_AddGeneralFateAttr(m_oOriginRtData, arrOriginGCardFateAtt, rstTroopInfo);
    this->_AddGeneralFateAttr(m_oOriginRtData, arrCurrentGCardFateAtt, rstTroopInfo);

    //  装备缘分加成
    float arrOriginEquipFateAtt[MAX_ATTR_ADD_NUM] = { 0 };
    float arrCurrentEquipFateAtt[MAX_ATTR_ADD_NUM] = { 0 };

    this->_AddEquipFateAttr(m_oOriginRtData, arrOriginEquipFateAtt, rstTroopInfo);
    this->_AddEquipFateAttr(m_oOriginRtData, arrCurrentEquipFateAtt, rstTroopInfo);

    // 武将星级加成
    this->_AddGeneralStar(m_oOriginRtData);
    this->_AddGeneralStar(m_oCurrentRtData);

    // 宝石加成
    this->_AddGeneralGem(m_oOriginRtData, rstTroopInfo);
    this->_AddGeneralGem(m_oCurrentRtData, rstTroopInfo);

    //  额外系统加成，军师技，亲密度
    this->_AddFeedTrainAtrr(m_oOriginRtData, rstTroopInfo);
    this->_AddFeedTrainAtrr(m_oCurrentRtData, rstTroopInfo);

    m_oOriginRtData.AddAllAttr(arrOriginGCardFateAtt);
    m_oCurrentRtData.AddAllAttr(arrCurrentGCardFateAtt);

    m_oOriginRtData.AddAllAttr(arrOriginEquipFateAtt);
    m_oCurrentRtData.AddAllAttr(arrCurrentEquipFateAtt);

    for (int i=0; i<MAX_ATTR_ADD_NUM; i++)
    {
        rstTroopInfo.m_AttrAddValue[i] = m_oOriginRtData.m_arrAttrValue[i];
    }
}

bool Troop::_InitPeakArenaTroopSkill(PKGMETA::DT_TROOP_INFO& rstTroopInfo)
{
    //是否觉醒
    DT_GENERAL_BRIEF_INFO stTemp;
    stTemp.m_dwId = m_dwGeneralId;
    DT_FIGHT_PLAYER_INFO& rstFightPlayerInfo = m_poFightPlayer->m_stPlayerInfo;

    int iEqual = 0;
    int iIndex = MyBSearch(&stTemp, rstFightPlayerInfo.m_astGeneralList, rstFightPlayerInfo.m_bGeneralCnt, sizeof(DT_GENERAL_BRIEF_INFO), &iEqual, Troop::GeneralInfoCmp);
    if (iEqual)
    {
        DT_GENERAL_BRIEF_INFO& rstGeneralInfo = rstFightPlayerInfo.m_astGeneralList[iIndex];
        rstTroopInfo.m_bIsAwake = rstFightPlayerInfo.m_astGeneralList[iIndex].m_bIsAwake;
    }

    //主动技能初始化
    m_dwActiveSkillId = rstTroopInfo.m_bIsAwake ? m_poResPeakArenaGeneral->m_dwAwakenSkillId : m_poResPeakArenaGeneral->m_dwActiveSkillId;
    m_bActiveSkillLevel = rstTroopInfo.m_bIsAwake ? m_poResPeakArenaGeneral->m_bAwakenSkillLv: m_poResPeakArenaGeneral->m_bActiveSkillLv;

    //被动技能初始化
    m_bPassiveSkillCnt = m_poResPeakArenaGeneral->m_bPassiveSkillCnt;
    for (int i = 0; i < m_bPassiveSkillCnt; i++)
    {
        m_dwPassiveSkillId[i] = m_poResPeakArenaGeneral->m_passiveSkillId[i];
        m_szPassiveSkillLevel[i] = m_poResPeakArenaGeneral->m_szPassiveSkillLv[i];
    }

    //将技能等级赋值给TroopInfo
    rstTroopInfo.m_stGeneralInfo.m_szSkillLevel[1] = m_bActiveSkillLevel;
    for (int i = 0; i < m_bPassiveSkillCnt; i++)
    {
        if (i == 0)
        {
            rstTroopInfo.m_stGeneralInfo.m_szSkillLevel[i] = m_szPassiveSkillLevel[i];
        }
        else
        {
            rstTroopInfo.m_stGeneralInfo.m_szSkillLevel[i+1] = m_szPassiveSkillLevel[i];
        }
    }

    // 武将初始化
    m_oGeneral.Init(this);
}

bool Troop::_InitNormalTroopSkill(PKGMETA::DT_TROOP_INFO& rstTroopInfo)
{
    //主动技能初始化
    m_dwActiveSkillId = m_poResGeneral->m_dwActiveSkillId;
    for (int i=0; i<EQUIP_TYPE_MAX_NUM; i++)
	{
		if (rstTroopInfo.m_astEquipInfoList[i].m_bType == m_poResGeneral->m_bSpecialEquipType)
		{
			if (rstTroopInfo.m_astEquipInfoList[i].m_wStar >= m_poResGeneral->m_bSpecialEquipStar)
			{
				m_dwActiveSkillId = m_poResGeneral->m_dwSpecialEquipSkillId;
			}
			break;
		}
	}
    m_bActiveSkillLevel = rstTroopInfo.m_stGeneralInfo.m_szSkillLevel[1];

    //主动技能秘籍
    m_bActiveSkillCheatsType = rstTroopInfo.m_stGeneralInfo.m_bCheatsType;
    for(int i = 0; i < MAX_SKILL_CHEATS_NUM; i++)
    {
        m_dwActiveCheatsLevel[i] = rstTroopInfo.m_stGeneralInfo.m_astCheatsList[i].m_bLevel;
    }

    //被动技能初始化
    for (int i = 0; i < m_poResGeneral->m_bPassiveSkillNumLevel && i < m_bGeneralStar; i++)
    {
        uint32_t id = m_poResGeneral->m_passiveSkillIdLevel[i];
        // i=1 为主动技
        if (id > 0 && i != 1)
        {
            m_dwPassiveSkillId[m_bPassiveSkillCnt] = id;
            m_szPassiveSkillLevel[m_bPassiveSkillCnt] = rstTroopInfo.m_stGeneralInfo.m_szSkillLevel[i];
            m_bPassiveSkillCnt++;
        }
    }

    // 武将初始化
    m_oGeneral.Init(this);
}

bool Troop::_InitTactics(FightPlayer* poPlayer)
{
    if (poPlayer->m_stPlayerInfo.m_bTacticsType > 0 && poPlayer->m_stPlayerInfo.m_bTacticsType <= MAX_TACTICS_NUM)
    {
        m_poTactics = Tactics::Get();
        return m_poTactics->Init(poPlayer, this);
    }
    return true;
}

void Troop::_InitPeakArenaRtData(FightObjRuntimeData& rRtData)
{
    rRtData.m_arrAttrValue[ATTR_HP] = m_poResPeakArenaGeneral->m_dwHp;
    rRtData.m_arrAttrValue[ATTR_SPEED] = m_poResArmy->m_fMoveSpeed;

    rRtData.m_arrAttrValue[ATTR_STR] = m_poResPeakArenaGeneral->m_dwStr;
    rRtData.m_arrAttrValue[ATTR_WIT] = m_poResPeakArenaGeneral->m_dwWit;
    rRtData.m_arrAttrValue[ATTR_STRDEF] = m_poResPeakArenaGeneral->m_dwStrDef;
    rRtData.m_arrAttrValue[ATTR_WITDEF] = m_poResPeakArenaGeneral->m_dwWitDef;

    rRtData.m_arrAttrValue[ATTR_CHANCE_HIT] = m_poResPeakArenaGeneral->m_fHitChance;
    rRtData.m_arrAttrValue[ATTR_CHANCE_DODGE] = m_poResPeakArenaGeneral->m_fDodgeChance;
    rRtData.m_arrAttrValue[ATTR_CHANCE_CRITICAL] = m_poResPeakArenaGeneral->m_fCriticalChance;
    rRtData.m_arrAttrValue[ATTR_CHANCE_ANTICRITICAL] = m_poResPeakArenaGeneral->m_fAntiCriticalChance;
    rRtData.m_arrAttrValue[ATTR_CHANCE_BLOCK] = m_poResPeakArenaGeneral->m_fBlockChance;
    rRtData.m_arrAttrValue[ATTR_CHANCE_CRITICAL_VALUE] = m_poResPeakArenaGeneral->m_fCriticalPara;
    rRtData.m_arrAttrValue[ATTR_CHANCE_ANTICRITICAL_VALUE] = m_poResPeakArenaGeneral->m_fAntiCriticalPara;
    rRtData.m_arrAttrValue[ATTR_CHANCE_BLOCK_VALUE] = m_poResPeakArenaGeneral->m_fBlockPara;
    rRtData.m_arrAttrValue[ATTR_DAMAGEADD] = m_poResPeakArenaGeneral->m_fAdditionalDamage;
    rRtData.m_arrAttrValue[ATTR_SUCKBLOOD] = m_poResPeakArenaGeneral->m_fVampirePara;

    rRtData.m_arrAttrValue[ATTR_BASE_CITYATK] = m_poResPeakArenaGeneral->m_dwAtkCityBase;
    rRtData.m_arrAttrValue[ATTR_BASE_NORMALATK] = m_poResPeakArenaGeneral->m_dwAtkNormalBase;
    rRtData.m_arrAttrValue[ATTR_BASE_ARMYSKILL] = m_poResPeakArenaGeneral->m_fArmyValue;
}

void Troop::_InitRtData(FightObjRuntimeData& rRtData)
{
    // 基础属性
    rRtData.m_arrAttrValue[ATTR_HP] = m_poResGeneral->m_dwInitHP;
    rRtData.m_arrAttrValue[ATTR_SPEED] = m_poResArmy->m_fMoveSpeed;

    // 武将属性
    rRtData.m_arrAttrValue[ATTR_STR] = m_poResGeneral->m_dwInitStr;
    rRtData.m_arrAttrValue[ATTR_WIT] = m_poResGeneral->m_dwInitWit;
    rRtData.m_arrAttrValue[ATTR_STRDEF] = m_poResGeneral->m_dwInitStrDef;
    rRtData.m_arrAttrValue[ATTR_WITDEF] = m_poResGeneral->m_dwInitWitDef;

    rRtData.m_arrAttrValue[ATTR_CHANCE_HIT] = m_poResGeneral->m_fInitChanceHit;
    rRtData.m_arrAttrValue[ATTR_CHANCE_DODGE] = m_poResGeneral->m_fInitChanceDodge;
    rRtData.m_arrAttrValue[ATTR_CHANCE_CRITICAL] = m_poResGeneral->m_fInitChanceCritical;
    rRtData.m_arrAttrValue[ATTR_CHANCE_ANTICRITICAL] = m_poResGeneral->m_fInitChanceAntiCritical;
    rRtData.m_arrAttrValue[ATTR_CHANCE_BLOCK] = m_poResGeneral->m_fInitChanceBlock;
    rRtData.m_arrAttrValue[ATTR_CHANCE_CRITICAL_VALUE] = m_poResGeneral->m_fInitParaCritical;
    rRtData.m_arrAttrValue[ATTR_CHANCE_ANTICRITICAL_VALUE] = m_poResGeneral->m_fInitParaAntiCritical;
    rRtData.m_arrAttrValue[ATTR_CHANCE_BLOCK_VALUE] = m_poResGeneral->m_fInitParaBlock;
    rRtData.m_arrAttrValue[ATTR_DAMAGEADD] = m_poResGeneral->m_fInitParaDamageAdd;
    rRtData.m_arrAttrValue[ATTR_SUCKBLOOD] = m_poResGeneral->m_fInitParaSuckBlood;

	rRtData.m_arrAttrValueLimit[ATTR_CHANCE_HIT] = m_poResGeneral->m_fChanceHitLimit;
	rRtData.m_arrAttrValueLimit[ATTR_CHANCE_DODGE] = m_poResGeneral->m_fChanceDodgeLimit;
	rRtData.m_arrAttrValueLimit[ATTR_CHANCE_CRITICAL] = m_poResGeneral->m_fChanceCriticalLimit;
	rRtData.m_arrAttrValueLimit[ATTR_CHANCE_ANTICRITICAL] = m_poResGeneral->m_fChanceAntiCriticalLimit;
	rRtData.m_arrAttrValueLimit[ATTR_CHANCE_BLOCK] = m_poResGeneral->m_fChanceBlockLimit;
	rRtData.m_arrAttrValueLimit[ATTR_CHANCE_CRITICAL_VALUE] = m_poResGeneral->m_fParaCriticalLimit;
	rRtData.m_arrAttrValueLimit[ATTR_CHANCE_ANTICRITICAL_VALUE] = m_poResGeneral->m_fParaAntiCriticalLimit;
	rRtData.m_arrAttrValueLimit[ATTR_CHANCE_BLOCK_VALUE] = m_poResGeneral->m_fParaBlockLimit;
	rRtData.m_arrAttrValueLimit[ATTR_DAMAGEADD] = m_poResGeneral->m_fParaDamageAddLimit;
	rRtData.m_arrAttrValueLimit[ATTR_SUCKBLOOD] = m_poResGeneral->m_fParaSuckBloodLimit;

    rRtData.m_arrAttrValue[ATTR_BASE_CITYATK] = m_poResGeneral->m_dwBaseDamageAtkCity;
    rRtData.m_arrAttrValue[ATTR_BASE_NORMALATK] = m_poResGeneral->m_dwBaseDamageAtkNormal;
    rRtData.m_arrAttrValue[ATTR_BASE_ARMYSKILL] = this->GetArmyBaseValue();

	// 兵种属性
	rRtData.m_fArmyAttackCityFirstCD = m_poResArmy->m_fAttackCityFirstCD;
	rRtData.m_fArmyAttackCityCD = m_poResArmy->m_fAttackCityCD;
	rRtData.m_fArmyAttackCityRatio = m_poResArmy->m_fAttackCityRatio;

	rRtData.m_fArmyAttackCD = m_poResArmy->m_fAttackCD;
	rRtData.m_fArmyDamageCD = m_poResArmy->m_fDamageCD;
	rRtData.m_fArmyAttackMoveSpeedRatio = m_poResArmy->m_fAttackSpeedRatio;

    //LOGRUN("Init General Data, id=%u, Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f, BaseParaArmy=%f",  m_dwGeneralId, rRtData.m_arrAttrValue[ATTR_HP], rRtData.m_arrAttrValue[ATTR_STR],
            //rRtData.m_arrAttrValue[ATTR_WIT], rRtData.m_arrAttrValue[ATTR_STRDEF], rRtData.m_arrAttrValue[ATTR_WITDEF], rRtData.m_arrAttrValueLimit[ATTR_BASE_NORMALATK], rRtData.m_arrAttrValue[ATTR_BASE_ARMYSKILL]);
}

void Troop::_AddValue(int& rOriginalData, uint8_t bType,  int iIncValue, int iPer)
{
    int iChgValue = 0;

    if (bType == VALUE_INC_TYPE_PERMILLAGE)
    {
        iChgValue = (int)(rOriginalData * iIncValue / 1000 * iPer / 1000);
    }
    else if (bType == VALUE_INC_TYPE_DIRECT_ADD)
    {
        iChgValue = (int)(iIncValue * iPer / 1000);
    }
    else
    {
        LOGERR("AddValue failed, Type(%d) is invalid", bType);
    }

    //LOGRUN(" _AddValue, OriginalData(%d), Type(%d), IncValue(%d), Per(%d), ChgVal(%d)", rOriginalData, bType, iIncValue, iPer, iChgValue);

    rOriginalData += iChgValue;
}

void Troop::_AddValue(float& rOriginalData, uint8_t bType,  float fIncValue, int iPer)
{
    float fChgValue = 0;

    if (bType == VALUE_INC_TYPE_PERMILLAGE)
    {
        //fChgValue = rOriginalData * fIncValue / 1000 * iPer / 1000;
        fChgValue = rOriginalData * fIncValue * iPer / 1000000;
    }
    else if (bType == VALUE_INC_TYPE_DIRECT_ADD)
    {
        fChgValue = fIncValue * iPer / 1000;
    }
    else
    {
        LOGERR("AddValue failed, Type(%d) is invalid", bType);
    }

    //LOGRUN(" _AddValue, OriginalData(%f), Type(%d), IncValue(%f), Per(%d), ChgVal(%f)", rOriginalData, bType, fIncValue, iPer, fChgValue);

    rOriginalData += fChgValue;
}


void Troop::_AddSkinAttr(FightObjRuntimeData& rRtData, uint32_t dwSkinId)
{
    RESGENERALSKIN* pResSkin = CGameDataMgr::Instance().GetResGeneralSkinMgr().Find(dwSkinId);
    if (!pResSkin)
    {
        LOGERR("Skin(%u) not found", dwSkinId);
        return;
    }

    if (pResSkin->m_dwGeneralId != m_dwGeneralId)
    {
        LOGERR("Skin General(%u) is not match General(%u)", pResSkin->m_dwGeneralId, m_dwGeneralId);
        return;
    }

    for (int i = 0; i < pResSkin->m_bAttrCnt; i++)
    {
        rRtData.m_arrAttrValue[pResSkin->m_szAttrType[i]] += (float)pResSkin->m_attrValue[i];
    }
}


void Troop::_AddGeneralLevel(FightObjRuntimeData& rRtData)
{
    if (m_bGeneralLevel > RES_MAX_GENERAL_LV_LEN || m_bGeneralLevel == 0)
    {
        LOGERR("General Level(%d) is invalid", m_bGeneralLevel);
        return;
    }

    ResGeneralLevelGrowMgr_t& rstResGeneralLvGrowMgr = CGameDataMgr::Instance().GetResGeneralLevelGrowMgr();

    //Hp
    RESGENERALLEVELGROW * poResGrow = rstResGeneralLvGrowMgr.Find(m_poResGeneralLevel->m_dwHpGrowId);
    if (!poResGrow)
    {
        LOGERR("poResGrow(%u) is not found", m_poResGeneralLevel->m_dwHpGrowId);
        return;
    }
    _AddValue(rRtData.m_arrAttrValue[ATTR_HP], m_poResGeneralLevel->m_dwHpGrowType,
                poResGrow->m_growLvList[m_bGeneralLevel-1], m_poResGeneralLevel->m_dwHpGrowPVPRatio);

    //Str
    poResGrow = rstResGeneralLvGrowMgr.Find(m_poResGeneralLevel->m_dwAtkGrowId);
    if (!poResGrow)
    {
        LOGERR("poResGrow(%u) is not found", m_poResGeneralLevel->m_dwAtkGrowId);
        return;
    }
    _AddValue(rRtData.m_arrAttrValue[ATTR_STR], m_poResGeneralLevel->m_dwAtkGrowType,
                poResGrow->m_growLvList[m_bGeneralLevel-1], m_poResGeneralLevel->m_dwAtkGrowPVPRatio);

    //Wit
    poResGrow = rstResGeneralLvGrowMgr.Find(m_poResGeneralLevel->m_dwWitGrowId);
    if (!poResGrow)
    {
        LOGERR("poResGrow(%u) is not found", m_poResGeneralLevel->m_dwWitGrowId);
        return;
    }
    _AddValue(rRtData.m_arrAttrValue[ATTR_WIT], m_poResGeneralLevel->m_dwWitGrowType,
                poResGrow->m_growLvList[m_bGeneralLevel-1], m_poResGeneralLevel->m_dwWitGrowPVPRatio);

    //StrDef
    poResGrow = rstResGeneralLvGrowMgr.Find(m_poResGeneralLevel->m_dwAtkDefGrowId);
    if (!poResGrow)
    {
        LOGERR("poResGrow(%u) is not found", m_poResGeneralLevel->m_dwAtkDefGrowId);
        return;
    }
    _AddValue(rRtData.m_arrAttrValue[ATTR_STRDEF], m_poResGeneralLevel->m_dwAtkDefGrowType,
                poResGrow->m_growLvList[m_bGeneralLevel-1], m_poResGeneralLevel->m_dwAtkDefGrowPVPRatio);

    //WitDef
    poResGrow = rstResGeneralLvGrowMgr.Find(m_poResGeneralLevel->m_dwWitDefGrowId);
    if (!poResGrow)
    {
        LOGERR("poResGrow(%u) is not found", m_poResGeneralLevel->m_dwWitDefGrowId);
        return;
    }
    _AddValue(rRtData.m_arrAttrValue[ATTR_WITDEF], m_poResGeneralLevel->m_dwWitDefGrowType,
                poResGrow->m_growLvList[m_bGeneralLevel-1], m_poResGeneralLevel->m_dwWitDefGrowPVPRatio);

    //BaseDamage
    poResGrow = rstResGeneralLvGrowMgr.Find(m_poResGeneralLevel->m_dwBaseDamageGrowId);
    if (!poResGrow)
    {
        LOGERR("poResGrow(%u) is not found", m_poResGeneralLevel->m_dwBaseDamageGrowId);
        return;
    }
    _AddValue(rRtData.m_arrAttrValue[ATTR_BASE_NORMALATK], m_poResGeneralLevel->m_dwBaseDamageGrowType,
                poResGrow->m_growLvList[m_bGeneralLevel-1], m_poResGeneralLevel->m_dwBaseDamageGrowPVPRatio);

    //LOGRUN("AddGeneralLevel Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f",  rRtData.m_arrAttrValue[ATTR_HP], rRtData.m_arrAttrValue[ATTR_STR],
            //rRtData.m_arrAttrValue[ATTR_WIT], rRtData.m_arrAttrValue[ATTR_STRDEF], rRtData.m_arrAttrValue[ATTR_WITDEF], rRtData.m_arrAttrValue[ATTR_BASE_NORMALATK]);

    return;
}


void Troop::_AddGeneralStar(FightObjRuntimeData& rRtData)
{
    if (m_bGeneralStar >= RES_MAX_STAR_LEVEL)
    {
        LOGERR("General Star(%d) is invalid", m_bGeneralStar);
        return;
    }

    _AddValue(rRtData.m_arrAttrValue[ATTR_HP], m_poResGeneralStar->m_bHpGrowType,
                m_poResGeneralStar->m_hpGrow[m_bGeneralStar], m_poResGeneralStar->m_dwHpGrowPVPRatio);

    _AddValue(rRtData.m_arrAttrValue[ATTR_STR], m_poResGeneralStar->m_bStrGrowType,
                m_poResGeneralStar->m_strGrow[m_bGeneralStar], m_poResGeneralStar->m_dwStrGrowPVPRatio);

    _AddValue(rRtData.m_arrAttrValue[ATTR_WIT], m_poResGeneralStar->m_bWitGrowType,
                m_poResGeneralStar->m_witGrow[m_bGeneralStar], m_poResGeneralStar->m_dwWitGrowPVPRatio);

    _AddValue(rRtData.m_arrAttrValue[ATTR_STRDEF], m_poResGeneralStar->m_bStrDefGrowType,
                m_poResGeneralStar->m_strDefGrow[m_bGeneralStar], m_poResGeneralStar->m_dwStrDefGrowPVPRatio);

    _AddValue(rRtData.m_arrAttrValue[ATTR_WITDEF], m_poResGeneralStar->m_bWitDefGrowType,
                m_poResGeneralStar->m_witDefGrow[m_bGeneralStar], m_poResGeneralStar->m_dwWitDefGrowPVPRatio);

    _AddValue(rRtData.m_arrAttrValue[ATTR_BASE_NORMALATK], m_poResGeneralStar->m_bBaseDamageGrowType,
                m_poResGeneralStar->m_baseDamageGrow[m_bGeneralStar], m_poResGeneralStar->m_dwBaseDamageGrowPVPRatio);

    //LOGRUN("AddGeneralStar Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f",  rRtData.m_arrAttrValue[ATTR_HP], rRtData.m_arrAttrValue[ATTR_STR],
            //rRtData.m_arrAttrValue[ATTR_WIT], rRtData.m_arrAttrValue[ATTR_STRDEF], rRtData.m_arrAttrValue[ATTR_WITDEF], rRtData.m_arrAttrValue[ATTR_BASE_NORMALATK]);

    return;
}

void Troop::_AddGeneralPhase(FightObjRuntimeData& rRtData)
{
    if (m_bGeneralPhase < 1 || m_bGeneralPhase > RES_MAX_GENERAL_PHASE)
    {
        return;
    }

    int iInitHp = m_poResGeneralPhase->m_hpGrow[m_bGeneralPhase -1];
    int iGeneralInitStr = m_poResGeneralPhase->m_strGrow[m_bGeneralPhase -1];
    int iGeneralInitWit = m_poResGeneralPhase->m_witGrow[m_bGeneralPhase -1];
    int iGeneralInitStrDef = m_poResGeneralPhase->m_strDefGrow[m_bGeneralPhase -1];
    int iGeneralInitWitDef = m_poResGeneralPhase->m_witDefGrow[m_bGeneralPhase -1];

    int iGeneralInitBaseDamageAtkCity = 0;//m_poResGeneralPhase->m_baseSiegeDamageGrow[m_bGeneralPhase -1];
    int iGeneralInitBaseDamageAtkNormal = m_poResGeneralPhase->m_baseDamageGrow[m_bGeneralPhase -1];


    float fPer = 1;

    rRtData.m_arrAttrValue[ATTR_HP] += iInitHp * fPer;
    rRtData.m_arrAttrValue[ATTR_STR] += iGeneralInitStr * fPer;
    rRtData.m_arrAttrValue[ATTR_WIT] += iGeneralInitWit * fPer;
    rRtData.m_arrAttrValue[ATTR_STRDEF] += iGeneralInitStrDef * fPer;
    rRtData.m_arrAttrValue[ATTR_WITDEF] += iGeneralInitWitDef * fPer;

    rRtData.m_arrAttrValue[ATTR_BASE_CITYATK] += iGeneralInitBaseDamageAtkCity * fPer;
    rRtData.m_arrAttrValue[ATTR_BASE_NORMALATK] += iGeneralInitBaseDamageAtkNormal * fPer;
}

void Troop::_AddGeneralFateAttr(FightObjRuntimeData& rRtData, OUT float attrValue[], PKGMETA::DT_TROOP_INFO& rstTroopInfo)
{
	RESGENERAL* pstResGeneral = m_poResGeneral;

    for (uint32_t i = 0; i < RES_MAX_GCARD_FATEBUFFIDS_NUM; i++)
    {
        uint32_t dwFateId = rstTroopInfo.m_stGeneralInfo.m_FateBuffIds[i];
        RESGENERALFATE* poResGeneralFate = CGameDataMgr::Instance().GetResGeneralFateMgr().Find(dwFateId);
        if (NULL == poResGeneralFate)
        {
            continue;
        }
        for (uint32_t j = 0; j < MAX_ATTR_ADD_NUM; j++)
        {
            attrValue[j] += rRtData.m_arrAttrValue[j] * poResGeneralFate->m_addAttribute[j] * CONST_RATIO_PERMILLAGE;
        }
    }
}

void Troop::_AddEquipFateAttr(FightObjRuntimeData& rRtData, OUT float attrValue[], PKGMETA::DT_TROOP_INFO& rstTroopInfo)
{
	RESGENERAL* pstResGeneral = m_poResGeneral;

	ResGeneralEquipFateMgr_t& rResGeneralEquipFateMgr = CGameDataMgr::Instance().GetResGeneralEquipFateMgr();
	RESGENERALEQUIPFATE* poResGeneralEquipFate = NULL;

	for (int i = 0; i < RES_MAX_EQUIP_FATEBUFFIDS_NUM; i++)
	{
		if ( 0 == pstResGeneral->m_equipFateId[i] )
		{
			continue;
		}

		poResGeneralEquipFate = rResGeneralEquipFateMgr.Find(pstResGeneral->m_equipFateId[i]);
		if (NULL == poResGeneralEquipFate)
		{
			LOGERR("the poResGeneralEquipFate<%u> is NULL", pstResGeneral->m_equipFateId[i]);
			return ;
		}

		uint8_t bIndex = poResGeneralEquipFate->m_bEquipType;
		DT_ITEM_EQUIP& rstEquipInfo = rstTroopInfo.m_astEquipInfoList[bIndex];

		if ( 0 == rstEquipInfo.m_wStar )
		{
			continue;
		}
		else
		{
			for (uint32_t j = 0; j < MAX_ATTR_ADD_NUM; j++)
			{
				attrValue[j] += rRtData.m_arrAttrValue[j] * poResGeneralEquipFate->m_addAttribute[j] * CONST_RATIO_PERMILLAGE;
			}
		}
	}
}

void Troop::_AddFeedTrainAtrr(FightObjRuntimeData& rRtData, PKGMETA::DT_TROOP_INFO& rstTroopInfo)
{

	for (int i=0; i<MAX_ATTR_ADD_NUM; i++)
	{
		rRtData.m_arrAttrValue[i] += rstTroopInfo.m_AttrAddValue[i];
	}

}

void Troop::_AddEquipBase(FightObjRuntimeData& rRtData, PKGMETA::DT_ITEM_EQUIP& rstEquipInfo)
{
    RESEQUIP* poResEquip = CGameDataMgr::Instance().GetResEquipMgr().Find(rstEquipInfo.m_dwId);
    if (poResEquip == NULL)
    {
        if (rstEquipInfo.m_dwId != 0)
        {
            LOGERR("ResEquip(%u) not found.", rstEquipInfo.m_dwId);
        }
        return;
    }
    uint8_t bLevel = rstEquipInfo.m_bLevel;
    if (bLevel==0 ||bLevel > RES_MAX_GENERAL_LV_LEN)
    {
        LOGERR("Level(%d) is invalid", bLevel);
        return;
    }
    _AddValue(rRtData.m_arrAttrValue[ATTR_HP], poResEquip->m_dwHpType, poResEquip->m_hpList[bLevel -1], poResEquip->m_dwHpPVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_STR], poResEquip->m_dwStrType, poResEquip->m_strList[bLevel -1], poResEquip->m_dwStrPVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_WIT], poResEquip->m_dwWitType, poResEquip->m_witList[bLevel -1], poResEquip->m_dwWitPVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_STRDEF], poResEquip->m_dwStrDefType, poResEquip->m_strDefList[bLevel -1], poResEquip->m_dwStrDefPVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_WITDEF], poResEquip->m_dwWitDefType, poResEquip->m_witDefList[bLevel -1], poResEquip->m_dwWitDefPVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_SPEED], poResEquip->m_dwSpeedType, (float)poResEquip->m_speedList[bLevel -1], poResEquip->m_dwSpeedPVPRatio);

    _AddValue(rRtData.m_arrAttrValue[ATTR_CHANCE_HIT], poResEquip->m_dwChanceHitType, poResEquip->m_fChanceHit, poResEquip->m_dwChanceHitPVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_CHANCE_DODGE], poResEquip->m_dwChanceDodgeType, poResEquip->m_fChanceDodge, poResEquip->m_dwChanceDodgePVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_CHANCE_CRITICAL], poResEquip->m_dwChanceCriticalType, poResEquip->m_fChanceCritical, poResEquip->m_dwChanceCriticalPVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_CHANCE_ANTICRITICAL], poResEquip->m_dwChanceAntiCriticalType, poResEquip->m_fChanceAntiCritical, poResEquip->m_dwChanceAntiCriticalPVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_CHANCE_BLOCK], poResEquip->m_dwChanceBlockType, poResEquip->m_fChanceBlock, poResEquip->m_dwChanceBlockPVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_CHANCE_CRITICAL_VALUE], poResEquip->m_dwParaCriticalType, poResEquip->m_fParaCritical, poResEquip->m_dwParaCriticalPVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_CHANCE_ANTICRITICAL_VALUE], poResEquip->m_dwParaAntiCriticalType, poResEquip->m_fParaAntiCritical, poResEquip->m_dwParaAntiCriticalPVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_CHANCE_BLOCK_VALUE], poResEquip->m_dwParaBlockType, poResEquip->m_fParaBlock, poResEquip->m_dwParaBlockPVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_DAMAGEADD], poResEquip->m_dwParaDamageAddType, poResEquip->m_fParaDamageAdd, poResEquip->m_dwParaDamageAddPVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_SUCKBLOOD], poResEquip->m_dwParaSuckBloodType, poResEquip->m_fParaSuckBlood, poResEquip->m_dwParaSuckBloodPVPRatio);

	_AddValue(rRtData.m_arrAttrValue[ATTR_BASE_NORMALATK], poResEquip->m_dwBaseDamageType, poResEquip->m_baseDamageList[bLevel -1], poResEquip->m_dwBaseDamagePVPRatio);

    //LOGRUN("Add EquipBase Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f",  rRtData.m_arrAttrValue[ATTR_HP], rRtData.m_arrAttrValue[ATTR_STR],
            //rRtData.m_arrAttrValue[ATTR_WIT], rRtData.m_arrAttrValue[ATTR_STRDEF], rRtData.m_arrAttrValue[ATTR_WITDEF], rRtData.m_arrAttrValue[ATTR_BASE_NORMALATK]);

    return;
}


void Troop::_AddEquipStar(FightObjRuntimeData& rRtData, PKGMETA::DT_ITEM_EQUIP& rstEquipInfo)
{
    RESEQUIP* poResEquip = CGameDataMgr::Instance().GetResEquipMgr().Find(rstEquipInfo.m_dwId);
    if (poResEquip == NULL)
    {
        return;
    }
    uint8_t bStar = rstEquipInfo.m_wStar;
    if (bStar >= RES_MAX_STAR_LEVEL)
    {
        LOGERR("Star(%d) is invalid", bStar);
        return;
    }

    //变更装备升星查找的方法，先获得装备上挂的武将id，然后从武将id获取装备相关的信息
    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    RESGENERAL* pResGeneral = rstResGeneralMgr.Find(rstEquipInfo.m_dwGCardId);
    if (!pResGeneral)
    {
        LOGERR("pResGeneral not found id=%u", rstEquipInfo.m_dwGCardId);
        return;
    }
    uint16_t wGeneralEquipStarId = pResGeneral->m_equipStarList[rstEquipInfo.m_bType - 1];//当前星级对应在generalequipstar表中的id
    ResGeneralEquipStarMgr_t& rstResGeneralEquipStarMgr = CGameDataMgr::Instance().GetResGeneralEquipStarMgr();
    RESGENERALEQUIPSTAR* pResGeneralEquipStar = rstResGeneralEquipStarMgr.Find(wGeneralEquipStarId);
    if (!pResGeneralEquipStar)
    {
        LOGERR("pResGeneralEquipStar not found id=%u", wGeneralEquipStarId);
        return;
    }

    //RESEQUIPSTAR* poResEquipStar = CGameDataMgr::Instance().GetResEquipStarMgr().Find(poResEquip->m_starGrow[bStar]);
    RESEQUIPSTAR* poResEquipStar = CGameDataMgr::Instance().GetResEquipStarMgr().Find(pResGeneralEquipStar->m_starList[bStar]);
    if (poResEquipStar == NULL)
    {
        LOGERR("ResEquipStar(%d) not found", poResEquip->m_starGrow[bStar]);
        return;
    }

    _AddValue(rRtData.m_arrAttrValue[ATTR_HP], poResEquipStar->m_dwStarAttributeType, (int)poResEquipStar->m_fStarAttribute, poResEquipStar->m_dwStarAttributePVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_STR], poResEquipStar->m_dwStarAttributeType, (int)poResEquipStar->m_fStarAttribute, poResEquipStar->m_dwStarAttributePVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_WIT], poResEquipStar->m_dwStarAttributeType, (int)poResEquipStar->m_fStarAttribute, poResEquipStar->m_dwStarAttributePVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_STRDEF], poResEquipStar->m_dwStarAttributeType, (int)poResEquipStar->m_fStarAttribute, poResEquipStar->m_dwStarAttributePVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_WITDEF], poResEquipStar->m_dwStarAttributeType, (int)poResEquipStar->m_fStarAttribute, poResEquipStar->m_dwStarAttributePVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_SPEED], poResEquipStar->m_dwSpeedStarAttributeType, poResEquipStar->m_fSpeedStarAttribute, poResEquipStar->m_dwSpeedStarAttributePVPRatio);
    _AddValue(rRtData.m_arrAttrValue[ATTR_BASE_NORMALATK], poResEquipStar->m_dwStarAttributeType, poResEquipStar->m_fStarAttribute, poResEquipStar->m_dwStarAttributePVPRatio);

    //LOGRUN("Add EquipStar Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f",  rRtData.m_arrAttrValue[ATTR_HP], rRtData.m_arrAttrValue[ATTR_STR],
            //rRtData.m_arrAttrValue[ATTR_WIT], rRtData.m_arrAttrValue[ATTR_STRDEF], rRtData.m_arrAttrValue[ATTR_WITDEF], rRtData.m_arrAttrValue[ATTR_BASE_NORMALATK]);

    return;
}

void Troop::_AddGeneralGem(FightObjRuntimeData& rRtData, PKGMETA::DT_TROOP_INFO& rstTroopInfo)
{
	DT_ITEM_GCARD& rstGeneralInfo = rstTroopInfo.m_stGeneralInfo;
	RESGEMLIST* poResGem = NULL;
	for (int i=0; i<MAX_GEM_SLOT_NUM; i++)
	{
		if (0 == rstGeneralInfo.m_GemSlot[i] || 1 == rstGeneralInfo.m_GemSlot[i])
		{//没有镶嵌宝石
			continue;
		}
		poResGem = CGameDataMgr::Instance().GetResGemListMgr().Find(rstGeneralInfo.m_GemSlot[i]);
		if (NULL == poResGem)
		{
			continue;
		}

		rRtData.m_arrAttrValue[ATTR_HP] += poResGem->m_attr[EQ_ATTR_HP-1] * poResGem->m_attrPVPRatio[EQ_ATTR_HP-1] / 1000;
		rRtData.m_arrAttrValue[ATTR_SPEED] += poResGem->m_attr[EQ_ATTR_SPEED-1] *poResGem->m_attrPVPRatio[EQ_ATTR_SPEED-1] / 1000;
		rRtData.m_arrAttrValue[ATTR_STR] += poResGem->m_attr[EQ_ATTR_STR-1] * poResGem->m_attrPVPRatio[EQ_ATTR_STR-1] /1000;
		rRtData.m_arrAttrValue[ATTR_WIT] += poResGem->m_attr[EQ_ATTR_WIT-1] * poResGem->m_attrPVPRatio[EQ_ATTR_WIT-1] / 1000;
		rRtData.m_arrAttrValue[ATTR_STRDEF] += poResGem->m_attr[EQ_ATTR_STRDEF-1] * poResGem->m_attrPVPRatio[EQ_ATTR_STRDEF-1] / 1000;
		rRtData.m_arrAttrValue[ATTR_WITDEF] += poResGem->m_attr[EQ_ATTR_WITDEF-1] * poResGem->m_attrPVPRatio[EQ_ATTR_WITDEF-1] / 1000;
		rRtData.m_arrAttrValue[ATTR_CHANCE_HIT] += poResGem->m_attr[EQ_ATTR_CHANCEHIT-1] * poResGem->m_attrPVPRatio[EQ_ATTR_CHANCEHIT-1] / 1000;
		rRtData.m_arrAttrValue[ATTR_CHANCE_DODGE] += poResGem->m_attr[EQ_ATTR_CHANCEDODGE-1] * poResGem->m_attrPVPRatio[EQ_ATTR_CHANCEDODGE-1] /1000;
		rRtData.m_arrAttrValue[ATTR_CHANCE_CRITICAL] += poResGem->m_attr[EQ_ATTR_CHANCECRITICAL-1] * poResGem->m_attrPVPRatio[EQ_ATTR_CHANCECRITICAL-1] / 1000;
		rRtData.m_arrAttrValue[ATTR_CHANCE_ANTICRITICAL] += poResGem->m_attr[EQ_ATTR_CHANCEANTICRITICAL-1] * poResGem->m_attrPVPRatio[EQ_ATTR_CHANCEANTICRITICAL-1] /1000;
		rRtData.m_arrAttrValue[ATTR_CHANCE_BLOCK] += poResGem->m_attr[EQ_ATTR_CHANCEBLOCK-1] * poResGem->m_attrPVPRatio[EQ_ATTR_CHANCEBLOCK-1] / 1000;
		rRtData.m_arrAttrValue[ATTR_CHANCE_CRITICAL_VALUE] += poResGem->m_attr[EQ_ATTR_CHANCECRITICAL_VALUE-1] * poResGem->m_attrPVPRatio[EQ_ATTR_CHANCECRITICAL_VALUE-1] /1000;
		rRtData.m_arrAttrValue[ATTR_CHANCE_ANTICRITICAL_VALUE] += poResGem->m_attr[EQ_ATTR_CHANCEANTICRITICAL_VALUE-1] * poResGem->m_attrPVPRatio[EQ_ATTR_CHANCECRITICAL_VALUE-1] /1000;
		rRtData.m_arrAttrValue[ATTR_CHANCE_BLOCK_VALUE] += poResGem->m_attr[EQ_ATTR_CHANCEBLOCK_VALUE-1] * poResGem->m_attrPVPRatio[EQ_ATTR_CHANCEBLOCK_VALUE-1] / 1000;
		rRtData.m_arrAttrValue[ATTR_DAMAGEADD] += poResGem->m_attr[EQ_ATTR_DAMAGEADD-1] * poResGem->m_attrPVPRatio[EQ_ATTR_DAMAGEADD-1] /1000;
		rRtData.m_arrAttrValue[ATTR_SUCKBLOOD] += poResGem->m_attr[EQ_ATTR_SUCKBLOOD-1] * poResGem->m_attrPVPRatio[EQ_ATTR_SUCKBLOOD-1] / 1000;
	}

    /*
    LOGRUN("Add EquipGem Hp=%d, Str=%d, Wit=%d, StrDef=%d, WitDef=%d, BaseDam=%d",  rRtData.m_arrAttrValue[ATTR_HP], rRtData.m_arrAttrValue[ATTR_STR],
            rRtData.m_iGeneralInitWit, rRtData.m_arrAttrValue[ATTR_STRDEF], rRtData.m_arrAttrValue[ATTR_WITDEF], rRtData.m_arrAttrValueLimit[ATTR_BASE_NORMALATK]);
    */
    return;
}


void Troop::_AddEquipAttr(FightObjRuntimeData& rRtData, PKGMETA::DT_TROOP_INFO& rstTroopInfo)
{
    FightObjRuntimeData stRtData;
    for (int i=0; i<EQUIP_TYPE_MAX_NUM; i++)
    {
        bzero(&stRtData, sizeof(stRtData));

        DT_ITEM_EQUIP& rstEquipInfo = rstTroopInfo.m_astEquipInfoList[i];

        //计算装备等级基础属性和等级加成
        _AddEquipBase(stRtData, rstEquipInfo);
        //计算星级加成
        _AddEquipStar(stRtData, rstEquipInfo);

        rRtData.m_arrAttrValue[ATTR_BASE_NORMALATK] += stRtData.m_arrAttrValue[ATTR_BASE_NORMALATK];
        rRtData.m_arrAttrValue[ATTR_HP] += stRtData.m_arrAttrValue[ATTR_HP];
        rRtData.m_arrAttrValue[ATTR_SPEED] += stRtData.m_arrAttrValue[ATTR_SPEED] ;
        rRtData.m_arrAttrValue[ATTR_STR] += stRtData.m_arrAttrValue[ATTR_STR] ;
        rRtData.m_arrAttrValue[ATTR_WIT] += stRtData.m_arrAttrValue[ATTR_WIT];
        rRtData.m_arrAttrValue[ATTR_STRDEF] += stRtData.m_arrAttrValue[ATTR_STRDEF];
        rRtData.m_arrAttrValue[ATTR_WITDEF] += stRtData.m_arrAttrValue[ATTR_WITDEF];
        rRtData.m_arrAttrValue[ATTR_CHANCE_HIT] += stRtData.m_arrAttrValue[ATTR_CHANCE_HIT];
        rRtData.m_arrAttrValue[ATTR_CHANCE_DODGE] += stRtData.m_arrAttrValue[ATTR_CHANCE_DODGE];
        rRtData.m_arrAttrValue[ATTR_CHANCE_CRITICAL] += stRtData.m_arrAttrValue[ATTR_CHANCE_CRITICAL];
        rRtData.m_arrAttrValue[ATTR_CHANCE_ANTICRITICAL] += stRtData.m_arrAttrValue[ATTR_CHANCE_ANTICRITICAL];
        rRtData.m_arrAttrValue[ATTR_CHANCE_BLOCK] += stRtData.m_arrAttrValue[ATTR_CHANCE_BLOCK];
        rRtData.m_arrAttrValue[ATTR_CHANCE_CRITICAL_VALUE] += stRtData.m_arrAttrValue[ATTR_CHANCE_CRITICAL_VALUE];
        rRtData.m_arrAttrValue[ATTR_CHANCE_ANTICRITICAL_VALUE] += stRtData.m_arrAttrValue[ATTR_CHANCE_ANTICRITICAL_VALUE];
        rRtData.m_arrAttrValue[ATTR_CHANCE_BLOCK_VALUE] += stRtData.m_arrAttrValue[ATTR_CHANCE_BLOCK_VALUE];
        rRtData.m_arrAttrValue[ATTR_DAMAGEADD] += stRtData.m_arrAttrValue[ATTR_DAMAGEADD];
        rRtData.m_arrAttrValue[ATTR_SUCKBLOOD] += stRtData.m_arrAttrValue[ATTR_SUCKBLOOD];

		/*
        LOGRUN("Add EquipAttr Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f",  rRtData.m_arrAttrValue[ATTR_HP], rRtData.m_arrAttrValue[ATTR_STR],
            rRtData.m_arrAttrValue[ATTR_WIT], rRtData.m_arrAttrValue[ATTR_STRDEF], rRtData.m_arrAttrValue[ATTR_WITDEF], rRtData.m_arrAttrValueLimit[ATTR_BASE_NORMALATK]);
		*/
	}
}


void Troop::_AddPeakArenaAttr(FightObjRuntimeData& rRtData, PKGMETA::DT_TROOP_INFO& rstTroopInfo)
{
    DT_GENERAL_BRIEF_INFO stTemp;
    stTemp.m_dwId = m_dwGeneralId;
    DT_FIGHT_PLAYER_INFO& rstFightPlayerInfo = m_poFightPlayer->m_stPlayerInfo;

    int iEqual = 0;
    int iIndex = MyBSearch(&stTemp, rstFightPlayerInfo.m_astGeneralList, rstFightPlayerInfo.m_bGeneralCnt, sizeof(DT_GENERAL_BRIEF_INFO), &iEqual, Troop::GeneralInfoCmp);
    if (!iEqual)
    {
        return;
    }

    DT_GENERAL_BRIEF_INFO& rstGeneralInfo = rstFightPlayerInfo.m_astGeneralList[iIndex];
    rstTroopInfo.m_bIsAwake = rstGeneralInfo.m_bIsAwake;

    RESPEAKARENAPARA* pResPara = CGameDataMgr::Instance().GetResPeakArenaParaMgr().Find(17);
    for (int i=0; i<pResPara->m_dwParamCount; i++)
    {
        int iPer = pResPara->m_paramList[i] * rstGeneralInfo.m_bStar;
        uint32_t dwAddValue = rRtData.m_arrAttrValue[i] * iPer / 100;
        rRtData.m_arrAttrValue[i] += dwAddValue;
        rstTroopInfo.m_AttrAddValue[i] += dwAddValue;
    }

    return;
}


void Troop::ChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iDamageRef, int& iHpChgBefore, int& iHpChgAfter, int& iDamageFxSrc, int& iDamageFxTar)
{
    if (m_bIsRetreat)
    {
        // 回城部队只能接受复活回血和回城BUFF回血
        bool bCanChgHp = false;
        do
        {
            if (iValueChgType == VALUE_CHG_TYPE::CHG_HP_REVIVE)
            {
                bCanChgHp = true;
                break;
            }

            if (iValueChgType == VALUE_CHG_TYPE::CHG_HP_BUFF && iValueChgPara == BUFF_RETREAT_HEAL)
            {
                bCanChgHp = true;
                break;
            }
        } while (false);


        if (!bCanChgHp)
        {
            LOGERR("troop is retreat but get hp chg not revive heal or buff retreat heal.");

            iHpChgBefore = m_iHpCur;
            iHpChgAfter = m_iHpCur;

            return;
        }
    }

    FightObj::ChgHp(iValueChgType, iValueChgPara, poSource, iDamageRef, iHpChgBefore, iHpChgAfter, iDamageFxSrc, iDamageFxTar);
}

void Troop::AfterChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iHpChgBefore, int iHpChgAfter)
{
    FightObj::AfterChgHp(iValueChgType, iValueChgPara, poSource, iHpChgBefore, iHpChgAfter);

    if (iHpChgBefore > 0 && iHpChgAfter <= 0)
    {
        m_poDungeon->AddTroopDead(this);

        // 设置死亡延迟最后更新时间
        uint8_t bDeadDelay = this->_SetDelayDeadLastTime(iValueChgType);

        // Sync dead
        DungeonLogic::Instance().SendTroopDead(m_poDungeon, this, poSource, bDeadDelay);

        // 每日任务统计
        if (poSource->m_poFightPlayer != NULL)
        {
            poSource->m_poFightPlayer->m_stTaskInfo.m_dwKillGeneral++;

            if (m_poResArmy->m_bType < MAX_ARMY_TYPE_NUM)
            {
                poSource->m_poFightPlayer->m_stTaskInfo.m_KillTypeList[m_poResArmy->m_bType]++;
            }
        }
    }
}

bool Troop::HasSkill(uint32_t dwSkillId)
{
    if (m_poResGeneral == NULL)
    {
        return false;
    }

    if (m_poResGeneral->m_dwActiveSkillId == dwSkillId)
    {
        return true;
    }

    for (int i=0; i<m_poResGeneral->m_bPassiveSkillNumBorn; i++)
    {
        if (m_poResGeneral->m_passiveSkillIdBorn[i] == dwSkillId)
        {
            return true;
        }
    }

    return false;
}

uint8_t Troop::_SetDelayDeadLastTime(int iValueChgType)
{
    uint8_t bRet = 0;

    // Update last time
    switch(iValueChgType)
    {
    case VALUE_CHG_TYPE::CHG_HP_ATK_SPEAR:
    case VALUE_CHG_TYPE::CHG_HP_ATK_RUSH:
    case VALUE_CHG_TYPE::CHG_HP_ATK_FACE:
    case VALUE_CHG_TYPE::CHG_HP_ATK_NORMAL:
    case VALUE_CHG_TYPE::CHG_HP_ATK_SHOOT:
        if ((CGameTime::Instance().GetCurrTimeMs() - m_poDungeon->m_ullDelayDeadLastTime) > DUNGEON_DELAY_DEAD_INTERVAL)
        {
            bRet = 1;
        }

        m_poDungeon->m_ullDelayDeadLastTime = CGameTime::Instance().GetCurrTimeMs();
        break;
    }

    return bRet;
}

float Troop::GetActiveSkillBaseValue(int iParId)
{
	float fRet = 0;
	int iActiveLevel = m_bActiveSkillLevel;

	RESGENERALSKILL* poResGeneralSkill = m_oGeneral.m_poActiveSkill->m_poResGeneralSkill;
	if (poResGeneralSkill == NULL)
	{
		LOGERR("poResGeneralSkill is NULL");
		return fRet;
	}

	if (iParId > poResGeneralSkill->m_wParCount || iParId<=0)
	{
		LOGERR("ParID is out of range, ParId=(%d), ParCount=(%d)", iParId, poResGeneralSkill->m_wParCount);
		return fRet;
	}

	fRet = poResGeneralSkill->m_parBase[iParId -1];

	if (0 == poResGeneralSkill->m_parBaseGrowId[iParId -1])
	{

		return fRet;
	}

	RESCONSUME * poResBaseGrow = CGameDataMgr::Instance().GetResConsumeMgr().Find(poResGeneralSkill->m_parBaseGrowId[iParId -1]);
	if (poResBaseGrow == NULL)
	{
		//LOGERR("poResBaseGrow is NULL, BaseGrowId (%d) not found", poResGeneralSkill->m_parBaseGrowId[iParId -1]);
		return fRet;
	}

	if (iActiveLevel > (int)poResBaseGrow->m_dwLvCount || iActiveLevel <= 0)
	{
		LOGERR("ActiveLevle is out of range, ActiveLevel=(%d), BaseGrowLevelMax=(%d)",
			iActiveLevel, poResBaseGrow->m_dwLvCount);
		return fRet;
	}

	if (poResBaseGrow->m_dwDivideRate == 0)
	{
		LOGERR(" DivideRate is Zero");
		return fRet;
	}

	fRet *= (1.0 + poResBaseGrow->m_lvList[iActiveLevel-1] / (float)poResBaseGrow->m_dwDivideRate * poResGeneralSkill->m_parBasePVPRatio[iParId -1] / 1000);

	return fRet;
}

float Troop::GetActiveSkillRateValue(int iParId)
{
	float fRet = 0;

	int iActiveLevel = m_bActiveSkillLevel;

	RESGENERALSKILL* poResGeneralSkill = m_oGeneral.m_poActiveSkill->m_poResGeneralSkill;
	if (poResGeneralSkill == NULL)
	{
		LOGERR("poResGeneralSkill is NULL");
		return fRet;
	}

	if(iParId > poResGeneralSkill->m_wParCount ||iParId<=0)
	{
		LOGERR("ParID is out of range, ParId=(%d), ParCount=(%d)", iParId, poResGeneralSkill->m_wParCount);
		return fRet;
	}

	fRet = poResGeneralSkill->m_parRatio[iParId -1];

	if (0 == poResGeneralSkill->m_parRatioGrowId[iParId -1])
	{
		return fRet;
	}

	RESCONSUME * poResRateGrow = CGameDataMgr::Instance().GetResConsumeMgr().Find(poResGeneralSkill->m_parRatioGrowId[iParId -1]);
	if (poResRateGrow == NULL)
	{
		//LOGERR("poResBaseGrow is NULL, RatioGrowId(%d) not found", poResGeneralSkill->m_parRatioGrowId[iParID -1]);
		return fRet;
	}

	if (iActiveLevel > (int)poResRateGrow->m_dwLvCount || iActiveLevel <= 0)
	{
		LOGERR("ActiveLevle is out of range, ActiveLevel=(%d), RateGrowLevelMax=(%d)",
			iActiveLevel, poResRateGrow->m_dwLvCount);
		return fRet;
	}

	if (poResRateGrow->m_dwDivideRate == 0)
	{
		LOGERR(" DivideRate is Zero");
		return fRet;
	}

	fRet *= (1.0 + poResRateGrow->m_lvList[iActiveLevel-1] / (float)poResRateGrow->m_dwDivideRate * poResGeneralSkill->m_parRatioPVPRatio[iParId -1] / 1000);

	return fRet;
}

float Troop::GetArmyBaseValueBase()
{
	float fRet = 0;
	if (m_poResArmy == NULL)
	{
		return fRet;
	}

	float fBase = m_poResArmy->m_fBaseValue;

	fRet = fBase;

	return fRet;
}

float Troop::GetArmyBaseValueGrow()
{
	float fRet = 0;
	if (m_poResArmy == NULL)
	{
		return fRet;
	}

	float fGrow = 0;
	float fPVPRatio = m_poResArmy->m_dwLevelupGrowPVPRatio;

	if (m_dwGeneralId <= 1000)
	{
		uint32_t consumeId = m_poResArmy->m_dwLevelupGrowId;

		if (consumeId != 0)
		{
			RESCONSUME* poResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(consumeId);
			if (poResConsume != NULL)
			{
				fGrow = poResConsume->m_lvList[m_iArmyLevel - 1] / (float)poResConsume->m_dwDivideRate;
			}
		}
	}
	else
	{
		// NPC 单独设置
		fGrow = m_poResGeneral->m_fBaseParaArmyNPC;
	}

	fRet = fGrow * fPVPRatio;

	return fRet;
}

float Troop::GetArmyBaseValue()
{
	float fRet = 0;
	if (m_poResArmy == NULL)
	{
		return fRet;
	}

	float fBase = m_poResArmy->m_fBaseValue;
	float fGrow = 0;
	float fPVPRatio = m_poResArmy->m_dwLevelupGrowPVPRatio / (float)1000;

	uint32_t consumeId = m_poResArmy->m_dwLevelupGrowId;

	if (consumeId != 0)
	{
		RESCONSUME* poResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(consumeId);
		if (poResConsume != NULL)
		{
			fGrow = poResConsume->m_lvList[m_iArmyLevel - 1] / (float)poResConsume->m_dwDivideRate;
		}
	}


	fRet = fBase + fGrow * fPVPRatio;

	return fRet;
}
