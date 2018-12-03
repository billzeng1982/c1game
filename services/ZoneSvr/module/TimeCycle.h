#pragma once
#include <vector>
#include "define.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include "../gamedata/GameDataMgr.h"

class TimeCycle : public TSingleton<TimeCycle>
{
private:
    vector<uint32_t> m_Usage4Pvp6v6Vector; //Pvp使用的Id
    

public:
    TimeCycle(){ };
    ~TimeCycle() { m_Usage4Pvp6v6Vector.clear(); };
    bool Init();
    bool LoadGameData();
    // 检查时间ID在当前是否有效
    int CheckTime(uint32_t dwTimeId);

    // 检查时间ID在当前是否有效,并返回开始时间和结束时间
    int CheckTime(uint32_t dwTimeId, uint64_t ullPara, OUT uint64_t* pullStart, OUT uint64_t* pullEnd);

    //  检查巅峰排位是否开启
    int CheckTime4Pvp6v6();

private:
    int _CheckNoCycle(RESTIMECYCLE* poResTimeCycle, uint64_t ullPara, OUT uint64_t* pullStart, OUT uint64_t* pullEnd);
    int _CheckSecondCycle(RESTIMECYCLE* poResTimeCycle, uint64_t ullPara, OUT uint64_t* pullStart, OUT uint64_t* pullEnd);
    int _CheckMonthCycle(RESTIMECYCLE* poResTimeCycle, uint64_t ullPara, OUT uint64_t* pullStart, OUT uint64_t* pullEnd);
    int _GetStartTime(RESTIMECYCLE* poResTimeCycle, uint64_t ullPara, OUT uint64_t* pullStart);
private:
    int m_iDailyResetTime;
};

