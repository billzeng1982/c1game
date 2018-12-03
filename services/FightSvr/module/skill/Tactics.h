#pragma once
#include <vector>
#include "define.h"
#include "object.h"
#include "../../gamedata/GameDataMgr.h"

class Troop;
class FightPlayer;

struct BuffId2Value
{
    uint32_t dwBuffId;
    float fBuffValue;
};

class Tactics : public IObject
{
public:
    Tactics();
    virtual ~Tactics();
    virtual void Clear();
private:
    void _Construct();
    bool _AddBuffId2Value(uint32_t dwBuffIndex, uint8_t bArmyType);

public:
    static Tactics* Get();
    static void Release(Tactics* pObj);

public:
    bool Init(FightPlayer* poFightPlayer, Troop *poTroop);
    float GetAddValue(uint32_t dwBuffId);

public:
    uint8_t m_bTacticsType;      //阵法类型
    uint8_t m_bTacticsLevel;     //阵法等级

private:
    FightPlayer* m_poFightPlayer;
    Troop *m_poTroop;

    vector<BuffId2Value> m_BuffId2BuffValue;
    vector<BuffId2Value>::iterator m_BuffId2BuffValueIter;
    BuffId2Value m_oBuffId2Value;

    RESTACTICS* m_pResTactics;
    RESTACTICIALBUFFINDEX* m_pResTacticailBuffIndex;
    RESTACTICIALBUFFLIST* m_pResTacticailBuffList;
    RESCONSUME* m_pResConsume;
    RESGENERAL* m_pResGeneral;
};

