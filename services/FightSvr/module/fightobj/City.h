#pragma  once
#include <list>
#include "cs_proto.h"
#include "ov_res_public.h"
#include "FightObj.h"

class City : public FightObj
{
public:
	City();
	virtual ~City();
	virtual void Clear();

private:
	void _Construct();

public:
	static City* Get();
	static void Release(City* pObj);

public:
	bool Init(Dungeon* poDungeon, FightPlayer* poFightPlayer);


public:
	virtual void ChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iDamageRef, int& iHpChgBefore, int& iHpChgAfter, int& iDamageFxSrc, int& iDamageFxTar);
	virtual void AfterChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iHpChgBefore, int iHpChgAfter);
};

