#include "LogMacros.h"
#include "string.h"
#include <sstream>
#include "../../framework/GameObjectPool.h"
#include "../fightobj/FightObj.h"
#include "../luainterface/LuaBinding.h"
#include "MasterSkill.h"

MasterSkill::MasterSkill()
{
    this->_Construct();
}

MasterSkill::~MasterSkill()
{

}

void MasterSkill::Clear()
{
    this->_Construct();
    IObject::Clear();
}

void MasterSkill::_Construct()
{
    m_dwSkillId = 0;
    m_bSkillLevel = 0;
    m_fPowerProgress = 0;

    m_poFightPlayer = NULL;
    m_poResMasterSkill = NULL;

    m_bEnable = false;
    m_fSkillCD = 0;
}

MasterSkill* MasterSkill::Get()
{
    return GET_GAMEOBJECT(MasterSkill, GAMEOBJ_MASTERSKILL);
}

void MasterSkill::Release(MasterSkill* pObj)
{
    RELEASE_GAMEOBJECT(pObj);
}

bool MasterSkill::Init(FightObj* poOwner, const std::string& rStrLuaTable, const std::string& rStrLuaScript)
{
    return SkillBase::Init(poOwner, rStrLuaTable, rStrLuaScript);
}

bool MasterSkill::Init(uint32_t dwSkillId, uint8_t bSkillLevel, FightPlayer* poFightPlayer)
{
    m_dwSkillId = dwSkillId;
    m_bSkillLevel = bSkillLevel;
    m_poFightPlayer = poFightPlayer;

    m_fMSAddRatioTime = 1;
    m_fMSAddRatioPower = 1;

    m_poResMasterSkill = CGameDataMgr::Instance().GetResMasterSkillMgr().Find(dwSkillId);
    if (m_poResMasterSkill == NULL)
    {
        LOGERR("ResMasterSkill(%u) not found.", dwSkillId);
        return false;
    }

    m_bEnable = true;
    m_fSkillCD = 0;


    // 脚本初始化
    std::stringstream ss;
    ss << "MasterSkill_" << m_poResMasterSkill->m_dwScriptId;
    std::string strLuaTable = ss.str();

    ss.clear();
    ss.str("");
    ss << CWorkDir::string() << "/gamedata/Scripts/MasterSkill/MasterSkill_" << m_poResMasterSkill->m_dwScriptId << ".txt";
    std::string strLuaScript = ss.str();
    this->Init(poFightPlayer, strLuaTable, strLuaScript);

    // 调用脚本runBorn函数
    LuaRunBorn();

    return true;
}

float MasterSkill::GetAddValue()
{
   RESCONSUME* poResConsume =  CGameDataMgr::Instance().GetResConsumeMgr().Find(m_poResMasterSkill->m_mSParBaseGrowId[0]);

   uint dwBaseValue = m_poResMasterSkill->m_mSParBase[0];
   uint32_t dwGrowCurr = 0;
   if (m_bSkillLevel > 0)
   {
      dwGrowCurr = poResConsume->m_lvList[m_bSkillLevel - 1];
   }
   uint32_t dwGrowDivide = poResConsume->m_dwDivideRate;
   uint32_t dwGrowDividePVP = m_poResMasterSkill->m_mSParBaseGrowPVPRatio[0];

   return dwBaseValue * (1 + (float)dwGrowCurr / dwGrowDivide * dwGrowDividePVP / 1000.0f);
}
