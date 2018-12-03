#pragma  once
#include <vector>
#include "cs_proto.h"
#include "ov_res_public.h"

using namespace std;

class Troop;
class GeneralSkill;
class PassiveSkill;

class General
{
public:
	General();
	virtual ~General();
	void Clear();

public:
	bool Init(Troop* poTroop);
	void Update(int dt);

private:
	bool _InitActiveSkill();
	bool _InitDefaultPassiveSkill();
	bool _InitPassiveSkill();

public:
	Troop* m_poTroop;
	GeneralSkill* m_poActiveSkill;

public:
	std::vector<PassiveSkill*> m_listPassiveSkill;
	std::map<uint32_t, uint8_t> m_dictPassiveSkillId;
};
