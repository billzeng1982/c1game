#include "SvrTime.h"
#include "../ZoneSvr.h"
#include "GameTime.h"

bool SvrTime::Init()
{
    ZONESVRCFG& rstConfig = ZoneSvr::Instance().GetConfig();
    time_t tmp = 0;
    if (0 != CGameTime::Instance().ConvStrToTime(rstConfig.m_szSvrOpenTime, tmp, "%Y/%m/%d:%H:%M:%S"))
    {
        LOGERR("Get the server open time error!");
        return false;
    }
    m_ullOpenSvrTime = tmp;
    LOGRUN("### server open time is %s .", rstConfig.m_szSvrOpenTime);
    return true;
}

uint64_t SvrTime::GetOpenSvrTime()
{
    return m_ullOpenSvrTime;
}

int SvrTime::GetOpenSvrDay()
{
    uint64_t dwCurTime = CGameTime::Instance().GetCurrSecond();
    return (dwCurTime - m_ullOpenSvrTime) / SECONDS_OF_DAY + 1;
}

