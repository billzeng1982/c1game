#include <cstdio>
#include <cstring>
#include "workdir.h"
#include "LogMacros.h"
#include "LuaBinding.h"
#include "LuaDataAgt.h"
#include "LuaBridge.h"
#include "common_proto.h"
#include "ov_res_keywords.h"
#include "ov_res_public.h"
#include "CpuSampleStats.h"

using namespace luabridge;

#define LOAD_ALL_LIB 0

static int pcall_callback(lua_State* L)
{
#if 0
	// 解析错误
	lua_Debug debug;
	//取得层数
	uint32_t level = 0;
	while (lua_getstack(L, level, &debug)) {
		level++;
	}
	if (!level) {
		return 0;
	}

	lua_getstack(L, level, &debug);
	lua_getinfo(L, "Sln", &debug);

	const char* err = lua_tostring(L, -1);
	lua_pop(L, 1);

	char msg[256] = { 0 };
	snprintf(msg, 256, "%s:line %d, info(%s %s), err[%s].", debug.short_src, debug.currentline, debug.namewhat, debug.name, err);

	lua_pushstring(L, msg);
	return 1;
#else
	// 直接获取错误
	const char* msg = lua_tostring(L, -1);
	//LOGERR(msg);

	// 并向上传递错误
	lua_pushstring(L, msg);
	return 1;
#endif
}

static int myPrint(lua_State *L)
{
	//返回栈中元素的个数
	int n = lua_gettop(L);

	if (n > 1)
	{
		lua_pushstring(L, "Too more argument for myPrint.");
		lua_error(L);
		return 0;
	}

	const char* msg = lua_tostring(L, 1);

	//LOGRUN(msg);

	/* return the number of results */
	return 0;
}

LuaBinding::LuaBinding()
{
	this->init();
}

LuaBinding::~LuaBinding()
{
	//关闭lua虚拟机
	lua_close(L);
}

void LuaBinding::init()
{
	//创建lua虚拟机
	L = luaL_newstate();
	this->_initLuaLib();
	this->_initGameLib();
}

void LuaBinding::_initLuaLib()
{
#if LOAD_ALL_LIB
	//包含全部的lua基本库
	luaL_openlibs(L);
#else
	// luaopen_xxx(L)需要和luaL_requiref配合使用以便能把模块名注册到_G表中
	luaL_Reg luaLib[] = {
		{"_G", luaopen_base},
		{LUA_LOADLIBNAME, luaopen_package},
		//{LUA_COLIBNAME, luaopen_coroutine},
		{LUA_TABLIBNAME, luaopen_table},
		//{LUA_IOLIBNAME, luaopen_io},
		{LUA_OSLIBNAME, luaopen_os},
		{LUA_STRLIBNAME, luaopen_string},
		{LUA_MATHLIBNAME, luaopen_math},
		//{LUA_UTF8LIBNAME, luaopen_utf8},
		//{LUA_DBLIBNAME, luaopen_debug},
#if defined(LUA_COMPAT_BITLIB)
		//{LUA_BITLIBNAME, luaopen_bit32},
#endif
		{NULL, NULL}
	};

	const luaL_Reg *lib = luaLib;
	for (; lib->func; lib++) {
		luaL_requiref(L, lib->name, lib->func, 1);
		//remove lib on stack
		lua_pop(L, 1);
	}
#endif

	// 提供一个打印调试接口
	lua_register(L, "print", myPrint);
}

/*
all custom class or function registerd here!
*/
void LuaBinding::_initGameLib()
{
	getGlobalNamespace(L)
		.beginClass<LuaDataAgt>("CLuaDataAgt")
		.addFunction("Equal", &LuaDataAgt::Equal)
		.addFunction("SetFilter", &LuaDataAgt::SetFilter)
		.addFunction("SetFormula", &LuaDataAgt::SetFormula)
		.addFunction("GetShareValue", &LuaDataAgt::GetShareValue)
		.addFunction("SetShareValue", &LuaDataAgt::SetShareValue)
		.addFunction("GetGoGroup", &LuaDataAgt::GetGoGroup)
		.addFunction("GetGoType", &LuaDataAgt::GetGoType)
		.addFunction("GetGoId", &LuaDataAgt::GetGoId)
		.addFunction("GetArmyType", &LuaDataAgt::GetArmyType)
		.addFunction("GetArmyPhase", &LuaDataAgt::GetArmyPhase)
		.addFunction("GetArmyBaseValueBase", &LuaDataAgt::GetArmyBaseValueBase)
		.addFunction("GetArmyBaseValueGrow", &LuaDataAgt::GetArmyBaseValueGrow)
		.addFunction("GetArmyBaseValue", &LuaDataAgt::GetArmyBaseValue)
		.addFunction("GetArmyRestrain", &LuaDataAgt::GetArmyRestrain)
		.addFunction("GetHp", &LuaDataAgt::GetHp)
		.addFunction("ChgHp", &LuaDataAgt::ChgHp)
		.addFunction("ChgTowerHp", &LuaDataAgt::ChgTowerHp)
		.addFunction("ChgBarrierHp", &LuaDataAgt::ChgBarrierHp)
		.addFunction("GetMoveSpeed", &LuaDataAgt::GetMoveSpeed)
		.addFunction("ChgSpeedByRatio", &LuaDataAgt::ChgSpeedByRatio)
		.addFunction("GetStrAtk", &LuaDataAgt::GetStrAtk)
		.addFunction("ChgStrAtk", &LuaDataAgt::ChgStrAtk)
		.addFunction("GetStrDef", &LuaDataAgt::GetStrDef)
		.addFunction("ChgStrDef", &LuaDataAgt::ChgStrDef)
		.addFunction("GetWitAtk", &LuaDataAgt::GetWitAtk)
		.addFunction("ChgWitAtk", &LuaDataAgt::ChgWitAtk)
		.addFunction("GetWitDef", &LuaDataAgt::GetWitDef)
		.addFunction("ChgWitDef", &LuaDataAgt::ChgWitDef)
		.addFunction("GetAttribute", &LuaDataAgt::GetAttribute)
		.addFunction("ChgAttribute", &LuaDataAgt::ChgAttribute)
		.addFunction("GetAttributeLimit", &LuaDataAgt::GetAttributeLimit)
		.addFunction("ChgAttributeLimit", &LuaDataAgt::ChgAttributeLimit)
		.addFunction("GetSiegeBack", &LuaDataAgt::GetSiegeBack)
		.addFunction("GetSiegeBackRatio", &LuaDataAgt::GetSiegeBackRatio)
		.addFunction("SetSkillEnable", &LuaDataAgt::SetSkillEnable)
		.addFunction("GetBuffLifeTime", &LuaDataAgt::GetBuffLifeTime)
		.addFunction("SetBuffLifeTime", &LuaDataAgt::SetBuffLifeTime)
		.addFunction("GetBuffEffectCD", &LuaDataAgt::GetBuffEffectCD)
		.addFunction("SetBuffEffectCD", &LuaDataAgt::SetBuffEffectCD)
		.addFunction("GetBuffLevel", &LuaDataAgt::GetBuffLevel)
		.addFunction("HasBuff", &LuaDataAgt::HasBuff)
		.addFunction("HasBuffInType", &LuaDataAgt::HasBuffInType)
		.addFunction("GetBuffSource", &LuaDataAgt::GetBuffSource)
		.addFunction("GetBuffOwnerListCnt", &LuaDataAgt::GetBuffOwnerListCnt)
		.addFunction("IsAllTroopDead", &LuaDataAgt::IsAllTroopDead)
		.addFunction("IsAttackCity", &LuaDataAgt::IsAttackCity)
		.addFunction("GetMSLevel4Skill", &LuaDataAgt::GetMSLevel4Skill)
		.addFunction("GetMSProgress4Skill", &LuaDataAgt::GetMSProgress4Skill)
		.addFunction("GetMSLevel4Buff", &LuaDataAgt::GetMSLevel4Buff)
		.addFunction("GetMSProgress4Buff", &LuaDataAgt::GetMSProgress4Buff)
		.addFunction("GetMSAddRatioTime4Buff", &LuaDataAgt::GetMSAddRatioTime4Buff)
		.addFunction("ChgMSAddRatioTime4Buff", &LuaDataAgt::ChgMSAddRatioTime4Buff)
		.addFunction("GetMSAddRatioTime", &LuaDataAgt::GetMSAddRatioTime)
		.addFunction("ChgMSAddRatioTime", &LuaDataAgt::ChgMSAddRatioTime)
		.addFunction("GetMSAddRatioPower4Buff", &LuaDataAgt::GetMSAddRatioPower4Buff)
		.addFunction("ChgMSAddRatioPower4Buff", &LuaDataAgt::ChgMSAddRatioPower4Buff)
		.addFunction("GetMSAddRatioPower", &LuaDataAgt::GetMSAddRatioPower)
		.addFunction("ChgMSAddRatioPower", &LuaDataAgt::ChgMSAddRatioPower)
		.addFunction("GetMSAddValue4Buff", &LuaDataAgt::GetMSAddValue4Buff)
		.addFunction("GetMSAddValue4Skill", &LuaDataAgt::GetMSAddValue4Skill)
		.addFunction("GetActiveSkillMorale", &LuaDataAgt::GetActiveSkillMorale)
		.addFunction("ChgActiveSkillMorale", &LuaDataAgt::ChgActiveSkillMorale)
		.addFunction("GetActiveSkillLevel", &LuaDataAgt::GetActiveSkillLevel)
		.addFunction("GetActiveSkillBaseValue", &LuaDataAgt::GetActiveSkillBaseValue)
		.addFunction("GetActiveSkillRateValue", &LuaDataAgt::GetActiveSkillRateValue)
		.addFunction("GetActiveSkillAdditionValue", &LuaDataAgt::GetActiveSkillAdditionValue)
		.addFunction("GetPassiveSkillBaseValue", &LuaDataAgt::GetPassiveSkillBaseValue)
		.addFunction("IsHaveActiveSkillCheats", &LuaDataAgt::IsHaveActiveSkillCheats)
		.addFunction("GetActiveSkillCheatsLevel", &LuaDataAgt::GetActiveSkillCheatsLevel)
		.addFunction("GetActiveSkillCheatsValue", &LuaDataAgt::GetActiveSkillCheatsValue)
		.addFunction("GetDamageFxType", &LuaDataAgt::GetDamageFxType)
		.addFunction("SetDamageFxType", &LuaDataAgt::SetDamageFxType)
		.addFunction("IsFateInTeam", &LuaDataAgt::IsFateInTeam)
		.addFunction("GetCountryID", &LuaDataAgt::GetCountryID)
		.addFunction("GetGeneralSex", &LuaDataAgt::GetGeneralSex)
		.addFunction("GetSelfTroopCountOnBattle", &LuaDataAgt::GetSelfTroopCountOnBattle)
        .addFunction("GetResTacticsBuffValue", &LuaDataAgt::GetResTacticsBuffValue)
		.endClass()

		.beginClass<FilterValue>("CFilterValue")
		.addData("m_iValueChgType", &FilterValue::m_iValueChgType)
		.addData("m_iValueChgPara", &FilterValue::m_iValueChgPara)
		.addData("m_fFoo1", &FilterValue::m_fFoo1)
		.addData("m_fFoo2", &FilterValue::m_fFoo2)
		.addData("m_fFoo3", &FilterValue::m_fFoo3)
		.addData("m_fFoo4", &FilterValue::m_fFoo4)
		.addData("m_fFoo5", &FilterValue::m_fFoo5)
		.addFunction("GetSource", &FilterValue::GetSource)
		.addFunction("GetTarget", &FilterValue::GetTarget)
		.endClass()

		.beginNamespace("FILTER_TYPE")
		.addConst<int>("NONE", FILTER_TYPE::NONE)
		.addConst<int>("POS", FILTER_TYPE::POS)
		.addConst<int>("HP", FILTER_TYPE::HP)
		.addConst<int>("SPEED", FILTER_TYPE::SPEED)
		.addConst<int>("COND", FILTER_TYPE::COND)
		.addConst<int>("TRIG", FILTER_TYPE::TRIG)
		.endNamespace()

		.beginNamespace("VALUE_CHG_TYPE")
		.addConst<int>("NONE", VALUE_CHG_TYPE::NONE)
		.addConst<int>("CHG_POS_MOVE", VALUE_CHG_TYPE::CHG_POS_MOVE)
		.addConst<int>("CHG_POS_TOWER", VALUE_CHG_TYPE::CHG_POS_TOWER)
		.addConst<int>("CHG_POS_BARRIER", VALUE_CHG_TYPE::CHG_POS_BARRIER)

		.addConst<int>("CHG_HP_COMM", VALUE_CHG_TYPE::CHG_HP_COMM)
		.addConst<int>("CHG_HP_ATK_CITYGATE", VALUE_CHG_TYPE::CHG_HP_ATK_CITYGATE)
		.addConst<int>("CHG_HP_ATK_CITYWALL", VALUE_CHG_TYPE::CHG_HP_ATK_CITYWALL)
		.addConst<int>("CHG_HP_ATK_CITYBACK", VALUE_CHG_TYPE::CHG_HP_ATK_CITYBACK)
		.addConst<int>("CHG_HP_ATK_CITYGATEBACK", VALUE_CHG_TYPE::CHG_HP_ATK_CITYGATEBACK)
		.addConst<int>("CHG_HP_ATK_TOWER", VALUE_CHG_TYPE::CHG_HP_ATK_TOWER)
		.addConst<int>("CHG_HP_ATK_BARRIER", VALUE_CHG_TYPE::CHG_HP_ATK_BARRIER)
		.addConst<int>("CHG_HP_ATK_NORMAL", VALUE_CHG_TYPE::CHG_HP_ATK_NORMAL)
		.addConst<int>("CHG_HP_ATK_SHOOT", VALUE_CHG_TYPE::CHG_HP_ATK_SHOOT)
		.addConst<int>("CHG_HP_ATK_SPEAR", VALUE_CHG_TYPE::CHG_HP_ATK_SPEAR)
		.addConst<int>("CHG_HP_ATK_RUSH", VALUE_CHG_TYPE::CHG_HP_ATK_RUSH)
		.addConst<int>("CHG_HP_ATK_FACE", VALUE_CHG_TYPE::CHG_HP_ATK_FACE)
		.addConst<int>("CHG_HP_ATK_AMBUSH", VALUE_CHG_TYPE::CHG_HP_ATK_AMBUSH)
		.addConst<int>("CHG_HP_SOLO", VALUE_CHG_TYPE::CHG_HP_SOLO)
		.addConst<int>("CHG_HP_REVIVE", VALUE_CHG_TYPE::CHG_HP_REVIVE)
		.addConst<int>("CHG_HP_GENERALSKILL", VALUE_CHG_TYPE::CHG_HP_GENERALSKILL)
		.addConst<int>("CHG_HP_PASSIVESKILL", VALUE_CHG_TYPE::CHG_HP_PASSIVESKILL)
		.addConst<int>("CHG_HP_MASTERSKILL", VALUE_CHG_TYPE::CHG_HP_MASTERSKILL)
		.addConst<int>("CHG_HP_BUFF", VALUE_CHG_TYPE::CHG_HP_BUFF)
		.addConst<int>("CHG_HP_SKILLCHEATS", VALUE_CHG_TYPE::CHG_HP_SKILLCHEATS)

		.addConst<int>("CHG_SPEED_ARMYSKILL", VALUE_CHG_TYPE::CHG_SPEED_ARMYSKILL)
		.addConst<int>("CHG_SPEED_GENERALSKILL", VALUE_CHG_TYPE::CHG_SPEED_GENERALSKILL)
		.addConst<int>("CHG_SPEED_ATK_NORMAL", VALUE_CHG_TYPE::CHG_SPEED_ATK_NORMAL)

		.addConst<int>("CHG_COND_ATK_NORMAL", VALUE_CHG_TYPE::CHG_COND_ATK_NORMAL)
		.addConst<int>("CHG_COND_ATK_ARMYSKILL", VALUE_CHG_TYPE::CHG_COND_ATK_ARMYSKILL)
		.addConst<int>("CHG_COND_ATK_CITY", VALUE_CHG_TYPE::CHG_COND_ATK_CITY)
		.addConst<int>("CHG_COND_ATK_OBSTACLE", VALUE_CHG_TYPE::CHG_COND_ATK_OBSTACLE)
		.addConst<int>("CHG_COND_ATK_LOCK", VALUE_CHG_TYPE::CHG_COND_ATK_LOCK)
		.addConst<int>("CHG_COND_ATK_CITYPAUSE", VALUE_CHG_TYPE::CHG_COND_ATK_CITYPAUSE)

		.addConst<int>("CHG_COND_SOLO", VALUE_CHG_TYPE::CHG_COND_SOLO)
		.addConst<int>("CHG_COND_ARMYSKILL", VALUE_CHG_TYPE::CHG_COND_ARMYSKILL)
		.addConst<int>("CHG_COND_GENERALSKILL", VALUE_CHG_TYPE::CHG_COND_GENERALSKILL)
		.addConst<int>("CHG_COND_MASTERSKILL", VALUE_CHG_TYPE::CHG_COND_MASTERSKILL)

		.addConst<int>("CHG_TRIG_GENERALSKILL", VALUE_CHG_TYPE::CHG_TRIG_GENERALSKILL)
		.addConst<int>("CHG_TRIG_GENERALSKILL_END", VALUE_CHG_TYPE::CHG_TRIG_GENERALSKILL_END)
		.addConst<int>("CHG_TRIG_MASTERSKILL", VALUE_CHG_TYPE::CHG_TRIG_MASTERSKILL)
		.addConst<int>("CHG_TRIG_MASTERSKILL_END", VALUE_CHG_TYPE::CHG_TRIG_MASTERSKILL_END)
		.addConst<int>("CHG_TRIG_BUFF_ADD_BEGIN", VALUE_CHG_TYPE::CHG_TRIG_BUFF_ADD_BEGIN)
		.addConst<int>("CHG_TRIG_BUFF_ADD_END", VALUE_CHG_TYPE::CHG_TRIG_BUFF_ADD_END)
		.addConst<int>("CHG_TRIG_BUFF_DEL", VALUE_CHG_TYPE::CHG_TRIG_BUFF_DEL)
		.addConst<int>("CHG_TRIG_TROOP_DEAD_DONE", VALUE_CHG_TYPE::CHG_TRIG_TROOP_DEAD_DONE)
		.addConst<int>("CHG_TRIG_TROOP_RETREAT_DONE", VALUE_CHG_TYPE::CHG_TRIG_TROOP_RETREAT_DONE)
		.addConst<int>("CHG_TRIG_TROOP_OUTCITY_DONE", VALUE_CHG_TYPE::CHG_TRIG_TROOP_OUTCITY_DONE)
		.endNamespace()

		.beginNamespace("TARGET_GROUP")
		.addConst<int>("NONE", TARGET_GROUP::NONE)
		.addConst<int>("SELF", TARGET_GROUP::SELF)
		.addConst<int>("ENEMY", TARGET_GROUP::ENEMY)
		.endNamespace()

		.beginNamespace("VALUE_TYPE")
		.addConst<int>("CURRENT", VALUE_TYPE::CURRENT)
		.addConst<int>("ORIGIN_CONF", VALUE_TYPE::ORIGIN_CONF)
		.addConst<int>("RUNTIME_CONF", VALUE_TYPE::RUNTIME_CONF)
		.endNamespace()

		.beginNamespace("PKGMETA")
		.beginNamespace("FIGHTOBJ_TYPE")
		.addConst<int>("FIGHTOBJ_NONE", PKGMETA::FIGHTOBJ_NONE)
		.addConst<int>("FIGHTOBJ_PLAYER", PKGMETA::FIGHTOBJ_PLAYER)
		.addConst<int>("FIGHTOBJ_WALL", PKGMETA::FIGHTOBJ_WALL)
		.addConst<int>("FIGHTOBJ_TROOP", PKGMETA::FIGHTOBJ_TROOP)
		.addConst<int>("FIGHTOBJ_BARRIER", PKGMETA::FIGHTOBJ_BARRIER)
		.addConst<int>("FIGHTOBJ_TOWER", PKGMETA::FIGHTOBJ_TOWER)
		.addConst<int>("FIGHTOBJ_CATAPULT", PKGMETA::FIGHTOBJ_CATAPULT)
		.endNamespace()
		.endNamespace()

		.beginNamespace("OvRes")
		.beginNamespace("ARMY_TYPE")
		.addConst<int>("ARMY_CAVALRY", ARMY_CAVALRY)
		.addConst<int>("ARMY_ARCHER", ARMY_ARCHER)
		.addConst<int>("ARMY_SPEARMAN", ARMY_SPEARMAN)
		.addConst<int>("ARMY_SHIELDMAN", ARMY_SHIELDMAN)
		.addConst<int>("ARMY_WISER", ARMY_WISER)
		.addConst<int>("ARMY_TAOIST", ARMY_TAOIST)
		.addConst<int>("ARMY_CRASHCAR", ARMY_CRASHCAR)
		.addConst<int>("ARMY_GUARD", ARMY_GUARD)
		.addConst<int>("ARMY_SPECIAL", ARMY_SPECIAL)
		.endNamespace()
		.endNamespace()

		.beginNamespace("OvRes")
		.beginNamespace("BUFF_TYPE")
		.addConst<int>("BUFF_TYPE_NONE", BUFF_TYPE_NONE)
		.addConst<int>("BUFF_TYPE_ATTR_GAIN", BUFF_TYPE_ATTR_GAIN)
		.addConst<int>("BUFF_TYPE_ATTR_LOSS", BUFF_TYPE_ATTR_LOSS)
		.addConst<int>("BUFF_TYPE_SPEED_GAIN", BUFF_TYPE_SPEED_GAIN)
		.addConst<int>("BUFF_TYPE_SPEED_LOSS", BUFF_TYPE_SPEED_LOSS)
		.addConst<int>("BUFF_TYPE_SILENT", BUFF_TYPE_SILENT)
		.addConst<int>("BUFF_TYPE_FAINT", BUFF_TYPE_FAINT)
		.addConst<int>("BUFF_TYPE_RESTRAIN", BUFF_TYPE_RESTRAIN)
		.addConst<int>("BUFF_TYPE_SCOFF", BUFF_TYPE_SCOFF)
		.addConst<int>("BUFF_TYPE_DANCE", BUFF_TYPE_DANCE)
		.addConst<int>("BUFF_TYPE_REVIVE", BUFF_TYPE_REVIVE)
		.endNamespace()
		.endNamespace()

		.beginNamespace("OvRes")
		.beginNamespace("ATTR_TYPE")
		.addConst<int>("ATTR_HP", ATTR_HP)
		.addConst<int>("ATTR_STR", ATTR_STR)
		.addConst<int>("ATTR_WIT", ATTR_WIT)
		.addConst<int>("ATTR_STRDEF", ATTR_STRDEF)
		.addConst<int>("ATTR_WITDEF", ATTR_WITDEF)
		.addConst<int>("ATTR_SPEED", ATTR_SPEED)
		.addConst<int>("ATTR_CHANCE_DODGE", ATTR_CHANCE_DODGE)
		.addConst<int>("ATTR_CHANCE_HIT", ATTR_CHANCE_HIT)
		.addConst<int>("ATTR_CHANCE_BLOCK", ATTR_CHANCE_BLOCK)
		.addConst<int>("ATTR_CHANCE_BLOCK_VALUE", ATTR_CHANCE_BLOCK_VALUE)
		.addConst<int>("ATTR_CHANCE_CRITICAL", ATTR_CHANCE_CRITICAL)
		.addConst<int>("ATTR_CHANCE_CRITICAL_VALUE", ATTR_CHANCE_CRITICAL_VALUE)
		.addConst<int>("ATTR_CHANCE_ANTICRITICAL", ATTR_CHANCE_ANTICRITICAL)
		.addConst<int>("ATTR_CHANCE_ANTICRITICAL_VALUE", ATTR_CHANCE_ANTICRITICAL_VALUE)
		.addConst<int>("ATTR_DAMAGEADD", ATTR_DAMAGEADD)
		.addConst<int>("ATTR_SUCKBLOOD", ATTR_SUCKBLOOD)
		.addConst<int>("ATTR_BASE_CITYATK", ATTR_BASE_CITYATK)
		.addConst<int>("ATTR_BASE_NORMALATK", ATTR_BASE_NORMALATK)
		.addConst<int>("ATTR_BASE_ARMYSKILL", ATTR_BASE_ARMYSKILL)
		.endNamespace()
		.endNamespace()

		.beginNamespace("OvRes")
		.beginNamespace("DAMAGE_FX_TYPE")
		.addConst<int>("DAMAGE_FX_NONE", DAMAGE_FX_NONE)
		.addConst<int>("DAMAGE_FX_NORMAL", DAMAGE_FX_NORMAL)
		.addConst<int>("DAMAGE_FX_SHOOT", DAMAGE_FX_SHOOT)
		.addConst<int>("DAMAGE_FX_SPEAR", DAMAGE_FX_SPEAR)
		.addConst<int>("DAMAGE_FX_RUSH", DAMAGE_FX_RUSH)
		.addConst<int>("DAMAGE_FX_FACE", DAMAGE_FX_FACE)
		.addConst<int>("DAMAGE_FX_WALL", DAMAGE_FX_WALL)
		.addConst<int>("DAMAGE_FX_GATE", DAMAGE_FX_GATE)
		.addConst<int>("DAMAGE_FX_CRITICAL", DAMAGE_FX_CRITICAL)
		.addConst<int>("DAMAGE_FX_BLOCK", DAMAGE_FX_BLOCK)
		.addConst<int>("DAMAGE_FX_DODGE", DAMAGE_FX_DODGE)
		.endNamespace()
		.endNamespace();

	char szScript[256];
	snprintf(szScript, sizeof(szScript), "%s/gamedata/Scripts/Common/Define.txt", CWorkDir::string());
	loadScript(szScript);
}

// 如果已在内存，则不产生IO
bool LuaBinding::_getLuaTable(const char* pszTableName, const char* pszScript)
{
	// 看相应脚本是否已在内存
	lua_getglobal(L, pszTableName);
	if(lua_isnil(L, -1))
	{
		// 不存在, 加载脚本
		lua_pop(L, 1);	// remove nil value
		if(!this->loadScript(pszScript))
		{
			return false;
		}
		lua_getglobal(L, pszTableName);
		if(lua_isnil(L, -1))
		{
			lua_pop(L, 1);	// remove nil value
			return false;;
		}
	}

	return true;
}

bool LuaBinding::loadScript(const char* pszScriptFile)
{
	// filePath is the real script needs reading
	int iError = luaL_dofile(L, pszScriptFile);
	if (iError == LUA_OK )
	{
		//LOGRUN("load script %s succeed.", pszScriptFile);
		//printf("load script %s succeed.\n", pszScriptFile);
		return true;
	}
	else
	{
		const char* strMsg = lua_tostring(L, -1);
		lua_pop(L, 1); // remove err
		LOGWARN("load script %s failed: %s", pszScriptFile, strMsg);
		//printf("load script %s failed: %s", pszScriptFile, strMsg);
		return false;
	}
}

void LuaBinding::func_runBorn(const char* pszTableName, const char* pszScript, LuaDataAgt* poAgt)
{
	if (poAgt == NULL)
	{
		assert(false);
		return;
	}

	do {
#if 0
		lua_pushcfunction(L, pcall_callback);
		int iPosErr = lua_gettop(L);
#else
		int iPosErr = 0;
#endif
		if (!this->_getLuaTable(pszTableName, pszScript))
		{
			break;
		}

		// 调用脚本的runBorn函数
		lua_getfield(L, -1, "func_runBorn");
		if (!lua_isfunction(L, -1))
		{
			LOGERR("func_runBorn is not a function!");
			//printf("func_runBorn is not a function!");
			break;
		}

		push<LuaDataAgt*>(L, poAgt);

        CpuSampleStats::Instance().BeginSample(pszScript);

		int iError = lua_pcall(L, 1, 0, iPosErr);	// has one para, has no ret value

        CpuSampleStats::Instance().EndSample();

		if (iError != LUA_OK)
		{
			const char* strMsg = lua_tostring(L, -1);
			lua_pop(L, 1);
			LOGWARN("lua_pcall error: %s", strMsg);
		}
	} while (0);

	lua_settop(L, 0);	// clear the stack.
	return;
}

void LuaBinding::func_runDead(const char* pszTableName, const char* pszScript, LuaDataAgt* poAgt)
{
	if (poAgt == NULL)
	{
		assert(false);
		return;
	}

	do {
		if (!this->_getLuaTable(pszTableName, pszScript))
		{
			break;
		}

		// 调用脚本的runDead函数
		lua_getfield(L, -1, "func_runDead");
		if (!lua_isfunction(L, -1))
		{
			LOGERR("func_runDead is not a function!");
			//printf("func_runDead is not a function!");
			break;
		}

		push<LuaDataAgt*>(L, poAgt);

        CpuSampleStats::Instance().BeginSample(pszScript);

		int iError = lua_pcall(L, 1, 0, 0);	// has one para, has no ret value

        CpuSampleStats::Instance().EndSample();


		if (iError != LUA_OK)
		{
			const char* strMsg = lua_tostring(L, -1);
			lua_pop(L, 1);
			LOGWARN("lua_pcall error: %s", strMsg);
		}
	} while (0);

	lua_settop(L, 0);	// clear the stack.
	return;
}

bool LuaBinding::func_checkCond(const char* pszTableName, const char* pszScript, LuaDataAgt* poAgt, int iAIId /*= 0*/)
{
	bool bRet = false;
	if (poAgt == NULL)
	{
		assert(false);
		return bRet;
	}

	do {
		if (!this->_getLuaTable(pszTableName, pszScript))
		{
			break;
		}

		// 调用脚本的checkCond函数
		lua_getfield(L, -1, "func_checkCond");
		if (!lua_isfunction(L, -1))
		{
			LOGERR("func_checkCond is not a function!");
			//printf("func_checkCond is not a function!");
			break;
		}

		push<LuaDataAgt*>(L, poAgt);
		push<int>(L, iAIId);

        CpuSampleStats::Instance().BeginSample(pszScript);

		int iError = lua_pcall(L, 2, 1, 0);	// has two para, has one ret value

        CpuSampleStats::Instance().EndSample();

		if (iError != LUA_OK)
		{
			const char* strMsg = lua_tostring(L, -1);
			lua_pop(L, 1);
			LOGWARN("lua_pcall error: %s", strMsg);
		}
		else
		{
			bRet = lua_toboolean(L, -1);
			lua_pop(L, 1);	// remove the ret value
		}
	} while (0);

	lua_settop(L, 0);	// clear the stack.
	return bRet;
}

void LuaBinding::func_takeEffect(const char* pszTableName, const char* pszScript, LuaDataAgt* poAgt)
{
	if (poAgt == NULL)
	{
		assert(false);
		return;
	}

	do {
		if (!this->_getLuaTable(pszTableName, pszScript))
		{
			break;
		}

		// 调用脚本的runDead函数
		lua_getfield(L, -1, "func_takeEffect");
		if (!lua_isfunction(L, -1))
		{
			LOGERR("func_takeEffect is not a function!");
			//printf("func_takeEffect is not a function!");
			break;
		}

		push<LuaDataAgt*>(L, poAgt);

        CpuSampleStats::Instance().BeginSample(pszScript);

		int iError = lua_pcall(L, 1, 0, 0);	// has one para, has no ret value

        CpuSampleStats::Instance().EndSample();

		if (iError != LUA_OK)
		{
			const char* strMsg = lua_tostring(L, -1);
			lua_pop(L, 1);
			LOGWARN("lua_pcall error: %s", strMsg);
		}
	} while (0);

	lua_settop(L, 0);	// clear the stack.
	return;
}

void LuaBinding::func_doFilter(const char* pszTableName, const char* pszScript, LuaDataAgt* poAgt, int iFilterType, LuaDataAgt* poFilterOwner, FilterValue* poValue)
{
	if (poAgt == NULL)
	{
		assert(false);
		return;
	}

	do {
		if (!this->_getLuaTable(pszTableName, pszScript))
		{
			break;
		}

		// 调用脚本的doFilter函数
		lua_getfield(L, -1, "func_doFilter");
		if (!lua_isfunction(L, -1))
		{
			LOGWARN("func_doFilter is not a function!");
			//printf("func_doFilter is not a function!");
			lua_pop(L, 3);
			break;
		}

		push<LuaDataAgt*>(L, poAgt);
		push<int>(L, iFilterType);
		push<LuaDataAgt*>(L, poFilterOwner);
		push<FilterValue*>(L, poValue);


        CpuSampleStats::Instance().BeginSample(pszScript);

		int iError = lua_pcall(L, 4, 0, 0);	// has four para, has no ret value

        CpuSampleStats::Instance().EndSample();

		if (iError != LUA_OK)
		{
			const char* strMsg = lua_tostring(L, -1);
			lua_pop(L, 1);
			LOGWARN("lua_pcall error: %s", strMsg);
		}

	} while (0);

	lua_settop(L, 0);	// clear the stack.
	return;
}

int LuaBinding::func_doFormula(const char* pszTableName, const char* pszScript, LuaDataAgt* poAgt, int iFormulaType, int iFormulaPara, LuaDataAgt* poSource, LuaDataAgt* poTarget, int iDamageRef)
{
	int iRet = 0;
	double fRet = 0;
	if (poAgt == NULL)
	{
		assert(false);
		return iRet;
	}

	do {
		if (!this->_getLuaTable(pszTableName, pszScript))
		{
			break;
		}

		// 调用脚本的doFormula函数
		lua_getfield(L, -1, "func_doFormula");
		if (!lua_isfunction(L, -1))
		{
			LOGERR("func_doFormula is not a function!");
			//printf("func_doFormula is not a function!");
			break;
		}

		push<LuaDataAgt*>(L, poAgt);
		push<int>(L, iFormulaType);
		push<int>(L, iFormulaPara);
		push<LuaDataAgt*>(L, poSource);
		push<LuaDataAgt*>(L, poTarget);
		push<int>(L, iDamageRef);


        CpuSampleStats::Instance().BeginSample(pszScript);

		int iError = lua_pcall(L, 6, 1, 0);	// has six para, has one ret value

        CpuSampleStats::Instance().EndSample();

		if (iError != LUA_OK)
		{
			const char* strMsg = lua_tostring(L, -1);
			lua_pop(L, 1);
			LOGWARN("lua_pcall error: %s", strMsg);
		}
		else
		{
			fRet = lua_tonumber(L, -1);
			lua_pop(L, 1);	// remove the ret value
		}
	} while (0);

	lua_settop(L, 0);	// clear the stack.
	iRet = (int)fRet;
	return iRet;
}

