#pragma  once
#include <list>
#include "cs_proto.h"
#include "ov_res_public.h"
#include "FightObj.h"

// Í¶Ê¯³µ
class Catapult : public FightObj
{
public:
	Catapult();
	virtual ~Catapult();
	virtual void Clear();

private:
	void _Construct();

public:
	static Catapult* Get();
	static void Release(Catapult* pObj);

public:
	bool Init(Dungeon* poDungeon, uint8_t bId);

public:
	FightObj* m_poTroopOccupy;
};

