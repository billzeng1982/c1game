#pragma once
#include <map>
#include "define.h"
#include "Buff.h"

class FightObj;

class BuffManager
{
private:
	typedef std::map<uint32_t /*Buff ID*/, Buff*> MapId2Buff_t;
	typedef std::map<int32_t /*»¥³â×é*/, Buff*> MapGroup2Buff_t;
	
	MapId2Buff_t m_dictBuffAll;
	MapGroup2Buff_t m_dictBuffIncompatible;

	FightObj* m_poOwner;
public:
	BuffManager() { m_poOwner = NULL; }
	
	virtual ~BuffManager() { }

	void Clear();

public:
	bool Init(FightObj* poOwner);

	void Update(float dt);

public:
	bool HasBuff(uint32_t dwBuffId);
	bool HasBuffInType(int iType);

	Buff* GetBuff(uint32_t dwBuffId);

	void AddBuff(uint32_t dwBuffId, FightObj* poSource, std::list<FightObj*>* poOwnerList);

	void DelBuff(uint32_t dwBuffId, int iBuffType);

private:
	void _ReplaceBuff(Buff* poBuff, uint32_t dwNewBuffId, FightObj* poSource, std::list<FightObj*>* poOwnerList);
	void _DelBuff(Buff* poBuff);
};
