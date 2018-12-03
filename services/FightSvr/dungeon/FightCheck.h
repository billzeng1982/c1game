#pragma once
#include "define.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "singleton.h"
#include "FightObj.h"

using namespace std;
using namespace PKGMETA;

class FightCheck : public TSingleton<FightCheck>
{
public:
    //连续小于最小攻击间隔的次数
    const static uint8_t MaxOutNum = 3;
    //判断作弊的系数，95就表示小于最小攻击间隔的95%算一次异常
    const static uint64_t MinDetaTimeRatio = 80;

public:
    bool CheckCityWallAttFreq(FightObj* poSource, FightObj* poTarget, int16_t iValueChgType, DT_LAST_ATTACK_TIME_INFO& rstLastAttackTimeInfo);
    int  SendKickPlayerNtf(uint64_t ullUin, int iZoneSvrProcId);
    void SetMinDetaTime(float value);

public:
    uint64_t m_ullMinDetaTime;

private:
    PKGMETA::SSPKG m_stSsPkg;
};