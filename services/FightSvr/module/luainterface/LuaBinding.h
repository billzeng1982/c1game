#pragma once
#include <lua.hpp>
#include "define.h"
#include "singleton.h"
#include "../skill/FilterManager.h"
#include "../skill/FormulaManager.h"

class LuaDataAgt;

class LuaBinding: public TSingleton<LuaBinding>
{
public:
	LuaBinding();
	virtual ~LuaBinding();

	lua_State* getLuaState() { return L; }

private:
	void init();
	void _initLuaLib();
	void _initGameLib();

	bool loadScript(const char* pszScriptFile);
	bool _getLuaTable(const char* pszTableName, const char* pszScript);

public:
	void func_runBorn(const char* pszTableName, const char* pszScript, LuaDataAgt* poAgt);
	void func_runDead(const char* pszTableName, const char* pszScript, LuaDataAgt* poAgt);
	bool func_checkCond(const char* pszTableName, const char* pszScript, LuaDataAgt* poAgt, int iAIId = 0);
	void func_takeEffect(const char* pszTableName, const char* pszScript, LuaDataAgt* poAgt);
	void func_doFilter(const char* pszTableName, const char* pszScript, LuaDataAgt* poAgt, int iFilterType, LuaDataAgt* poFilterOwner, FilterValue* poValue);
	int func_doFormula(const char* pszTableName, const char* pszScript, LuaDataAgt* poAgt, int iFormulaType, int iFormulaPara, LuaDataAgt* poSource, LuaDataAgt* poTarget, int iDamageRef);

private:
	lua_State* L;
};

