#pragma  once
#include <list>
#include "cs_proto.h"
#include "ov_res_public.h"
#include "FightObj.h"

class Barrier : public FightObj
{
public:
	Barrier();
	virtual ~Barrier();
	virtual void Clear();

private:
	void _Construct();

public:
	static Barrier* Get();
	static void Release(Barrier* pObj);

public:
	bool Init(Dungeon* poDungeon, FightPlayer* poFightPlayer, uint8_t bId, uint8_t bHp);

public:
	virtual void ChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iDamageRef, int& iHpChgBefore, int& iHpChgAfter, int& iDamageFxSrc, int& iDamageFxTar);
	virtual void AfterChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iHpChgBefore, int iHpChgAfter);
};
