#include "define.h"
#include "LogMacros.h"
#include "../../framework/GameObjectPool.h"
#include "../../gamedata/GameDataMgr.h"
#include "FightPlayer.h"
#include "ObjectUpdatorMgr.h"
#include "ObjectUpdator.h"
#include "framework/GameObjectPool.h"
#include "CpuSampleStats.h"
#include "FakeRandom.h"

using namespace PKGMETA;

FightPlayer::FightPlayer()
{
	this->_Construct();
}

FightPlayer::~FightPlayer()
{
}

void FightPlayer::Clear()
{
    if (m_poMasterSkill)
    {
	    MasterSkill::Release(m_poMasterSkill);
    }

	this->_Construct();
	FightObj::Clear();
}

void FightPlayer::_Construct()
{
	m_ullUin = 0;
	m_poPlayer = NULL;
	m_bReady = false;
    m_iZoneSvrProcId = 0;

	bzero(&m_stTaskInfo, sizeof(m_stTaskInfo));
    bzero(&m_stPlayerInfo, sizeof(m_stPlayerInfo));

	m_fMoraleMax = 0;
	m_fMorale = 0;
	m_fMoraleIncSpeed = 0;
    m_fMoraleIncSpeed1 = 0;
	m_fMoraleIncSpeed2 = 0;
	m_iMoraleSpeedUpTime1 = 0;
	m_iMoraleSpeedUpTime2 = 0;
    m_dwBarrierCount = 0;

	m_poMasterSkill = NULL;
}

FightPlayer* FightPlayer::Get()
{
	return GET_GAMEOBJECT(FightPlayer, GAMEOBJ_FIGHTPLAYER);
}

void FightPlayer::Release(FightPlayer* pObj)
{
	RELEASE_GAMEOBJECT(pObj);
}

bool FightPlayer::Init(Dungeon* poDungeon, DT_FIGHT_PLAYER_INFO& rPlayerInfo)
{
    m_ullUin = rPlayerInfo.m_ullUin;
    STRNCPY(m_szName, rPlayerInfo.m_szName, MAX_NAME_LENGTH);
    m_poPlayer = NULL;
    m_iZoneSvrProcId = rPlayerInfo.m_iZoneSvrId;
	m_stPlayerInfo = rPlayerInfo;
	m_bReady = false;
	m_chGroup = rPlayerInfo.m_chGroup;

	FightObj::Init(poDungeon, this, FIGHTOBJ_PLAYER, rPlayerInfo.m_chGroup);

    // 初始化军师技
    if (m_stPlayerInfo.m_dwMasterSkillId != 0)
    {
        //军师计Id为0，延迟初始化
	    m_poMasterSkill = MasterSkill::Get();
	    m_poMasterSkill->Init(m_stPlayerInfo.m_dwMasterSkillId, m_stPlayerInfo.m_bMSkillLevel, this);
    }

	bzero(&m_stTaskInfo, sizeof(m_stTaskInfo));

	// 初始化士气
	ResBasicMgr_t& rResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
	RESBASIC* poBasicMorale = rResBasicMgr.Find(BASIC_MORALE);              // 初始阶段
    RESBASIC* poBasicMorale1 = rResBasicMgr.Find(BASIC_MORALE_1);           // 剩余120秒的情况
    RESBASIC* poBasicMorale2 = rResBasicMgr.Find(BASIC_MORALE_2);           // 剩余60秒的情况

	m_fMoraleMax = poBasicMorale->m_para[0];
	m_fMoraleIncSpeed = poBasicMorale->m_para[1];
	m_fMorale = poBasicMorale->m_para[2];

    m_fMoraleIncSpeed1 = m_fMoraleIncSpeed * poBasicMorale1->m_para[2];
    m_fMoraleIncSpeed2 = m_fMoraleIncSpeed * poBasicMorale2->m_para[2];

    // 配置档配的是秒
    m_iMoraleSpeedUpTime1 = poBasicMorale1->m_para[0] * 1000;
    m_iMoraleSpeedUpTime2 = poBasicMorale2->m_para[0] * 1000;

	return true;
}

void FightPlayer::Update(int iDeltaTime)
{
    float fIncSpeed = _GetMoraleIncSpeed();
    m_fMorale += fIncSpeed * ((float)iDeltaTime / 1000.0f);

	if (m_fMorale > m_fMoraleMax)
	{
		m_fMorale = m_fMoraleMax;
	}
}

float FightPlayer::_GetMoraleIncSpeed()
{
    float fIncSpeed = 0; // 士气每秒

	int iTimePass = m_poDungeon->m_iDungeonTime - m_poDungeon->m_iTimeLeft4Fight;

	if (iTimePass >= m_iMoraleSpeedUpTime2)
	{
		fIncSpeed = m_fMoraleIncSpeed2;
	}
	else if (iTimePass >= m_iMoraleSpeedUpTime1)
	{
		fIncSpeed = m_fMoraleIncSpeed1;
	}
	else
	{
		fIncSpeed = m_fMoraleIncSpeed;
	}

    return fIncSpeed;
}

float FightPlayer::GetMorale()
{
	return m_fMorale;
}

// 在释放技能判断的时候，由于服务器更新的频率间隔较大，预先计算当前时间士气值
float FightPlayer::GetMoraleForPreUpdate()
{
#if 0
    // 方案一，按服务器时间计算当前真实更新
    ObjectUpdator* pObjUpdator = ObjectUpdatorMgr::Instance().GetObjUpdatorPtr(GAMEOBJ_DUNGEON);
    if (!pObjUpdator)
    {
        return m_fMorale;
    }

    return m_fMorale + _GetMoraleIncSpeed() * ((float)pObjUpdator->GetMsPassInterval() / 1000.0f);
#else
    // 方案二，直接增加一帧的预判
    return m_fMorale + _GetMoraleIncSpeed() * (DFT_UPDATE_FREQ / 1000.0f);
#endif
}

void FightPlayer::ChgMorale(float fMorale)
{
	m_fMorale += fMorale;

	if (m_fMorale > m_fMoraleMax)
	{
		m_fMorale = m_fMoraleMax;
	}
	else if (m_fMorale < 0)
	{
		m_fMorale = 0;
	}
}

bool FightPlayer::IsGeneralInTeam(uint32_t dwGeneralId)
{
    for (int i=0; i<m_stPlayerInfo.m_bTroopNum; i++)
    {
        if (m_stPlayerInfo.m_astTroopList[i].m_stGeneralInfo.m_dwId == dwGeneralId)
        {
            return true;
        }
    }
    return false;
}

DT_TROOP_INFO* FightPlayer::ChooseGeneral(uint32_t dwGeneralId)
{
    if ((m_poDungeon->m_bChooseCount >= Dungeon::ChooseOrder[m_poDungeon->m_iCntLeft4Choose-1]) ||
        (m_chGroup != m_poDungeon->m_bChooseGroup))
    {
        LOGERR("Player(%lu) group(%d) choose General(%u) failed, ChooseCount(%d), ChooseGroup(%d)",
                m_ullUin, m_chGroup, dwGeneralId, m_poDungeon->m_bChooseCount, m_poDungeon->m_bChooseGroup);
        return NULL;
    }

    int iIndex = m_poDungeon->FindGeneral(dwGeneralId);
    if (iIndex < 0)
    {
        LOGERR("Player(%lu) choose General(%u) failed, not found", m_ullUin, dwGeneralId);
        return NULL;
    }

    if (IsGeneralInTeam(dwGeneralId))
    {
        LOGERR("Player(%lu) choose General(%u) failed, already existed", m_ullUin, dwGeneralId);
        return NULL;
    }

    if (m_stPlayerInfo.m_bTroopNum >= MAX_TROOP_NUM_PVP)
    {
        LOGERR("Player(%lu) choose General(%u) failed, troop num(%d) out of range", m_ullUin, dwGeneralId, m_stPlayerInfo.m_bTroopNum);
        return NULL;
    }

    DT_TROOP_INFO& rstTroopInfo = m_stPlayerInfo.m_astTroopList[m_stPlayerInfo.m_bTroopNum++];
    bzero(&rstTroopInfo, sizeof(rstTroopInfo));

    rstTroopInfo.m_bId = MAX_TROOP_NUM * m_chGroup + m_stPlayerInfo.m_bTroopNum;

    DT_ITEM_GCARD& rstGCard = rstTroopInfo.m_stGeneralInfo;
    rstGCard.m_dwId = dwGeneralId;

    //武将的等级，星级，阶
    rstGCard.m_bLevel = m_poDungeon->m_GeneralList[iIndex].m_bLv;
    rstGCard.m_bStar = m_poDungeon->m_GeneralList[iIndex].m_bStar;
    rstGCard.m_bPhase = m_poDungeon->m_GeneralList[iIndex].m_bGrade;

    //武将的兵种
    RESPEAKARENAPARA* pResPara = CGameDataMgr::Instance().GetResPeakArenaParaMgr().Find(3);
    assert(pResPara);
    rstGCard.m_bArmyLv = pResPara->m_paramList[0];
    rstGCard.m_bArmyPhase = pResPara->m_paramList[1];

    //每选择一个武将，就初始化一个武将
    m_poDungeon->InitOneTroop(this, rstTroopInfo);

    //不能选择重复武将，选择的武将从武将列表中删除
    m_poDungeon->DelGeneral(dwGeneralId);
    m_poDungeon->m_bChooseCount++;

    LOGRUN("Player(%lu) choose general(%u)", m_ullUin, dwGeneralId);

    return &rstTroopInfo;
}

DT_TROOP_INFO* FightPlayer::RandomChooseGenaral()
{
    uint8_t bRandom = (uint8_t)CFakeRandom::Instance().Random(m_poDungeon->m_bGeneralCount);
    uint32_t dwGeneralId = m_poDungeon->m_GeneralList[bRandom].m_dwGeneralID;

    LOGRUN("Player(%lu) Random choose general(%u)", m_ullUin, dwGeneralId);

    return ChooseGeneral(dwGeneralId);
}

MasterSkill* FightPlayer::ChooseMasterSkill(uint32_t dwMSkillId)
{
    if (m_poMasterSkill)
    {
        LOGERR("Player(%lu) choose MSkill(%u) failed, already existed", m_ullUin, m_poMasterSkill->m_dwSkillId);
        return NULL;
    }

    int iIndex = m_poDungeon->FindMSkill((uint8_t)dwMSkillId);
    if (iIndex < 0)
    {
        LOGERR("Player(%lu) choose MSkill(%u) failed, not found", m_ullUin, dwMSkillId);
        return NULL;
    }

    m_stPlayerInfo.m_dwMasterSkillId = m_poDungeon->m_MSkillList[iIndex].m_bId;
    m_stPlayerInfo.m_bMSkillLevel = m_poDungeon->m_MSkillList[iIndex].m_bLevel;

    m_poMasterSkill = MasterSkill::Get();
    m_poMasterSkill->Init(m_stPlayerInfo.m_dwMasterSkillId, m_stPlayerInfo.m_bMSkillLevel, this);

    m_poDungeon->m_bChooseCount++;

    return m_poMasterSkill;
}

MasterSkill* FightPlayer::RandomChooseMSkill()
{
    return ChooseMasterSkill(1);
}

