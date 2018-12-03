#pragma once
#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include <map>

class Sign : public TSingleton<Sign>
{
public:
    Sign(){};
    virtual ~Sign() {};
    bool Init();
private:

public:
    int HandleSign7dAward(IN PlayerData* pstData, OUT SC_PKG_SIGN7D_CLICK_RSP& rstSign7dClickRsp);
    int HandleSign30dAward(INOUT PlayerData* pstData, IN CS_PKG_SIGN30D_CLICK_REQ& rstSign30dClickReq, OUT SC_PKG_SIGN30D_CLICK_RSP& rstSign7dClickRsp);
    //uint8_t GetVipCnt(PlayerData* pstData, uint8_t bType);
    
    int Compensate30dAward(PlayerData* pstData);    //vip升级补偿double签到


private:
    void _Sign30dReset(INOUT DT_ROLE_MISC_INFO& rstMiscInfo);
    int _HandleSign30dNormal(PlayerData* pstData, IN CS_PKG_SIGN30D_CLICK_REQ& rstSign30dClickReq, OUT SC_PKG_SIGN30D_CLICK_RSP& rstSign30dClickRsp);
    int _HandleSign30dExtraAward(IN PlayerData* pstData, OUT SC_PKG_SIGN30D_CLICK_RSP& rstSign30dClickRsp);
    //int _HandleSign30dForgetFree(PlayerData* pstData, IN CS_PKG_SIGN30D_CLICK_REQ& rstSign30dClickReq, OUT SC_PKG_SIGN30D_CLICK_RSP& rstSign30dClickRsp);
    //int _HandleSign30dForgetDiamond(PlayerData* pstData, IN CS_PKG_SIGN30D_CLICK_REQ& rstSign30dClickReq, OUT SC_PKG_SIGN30D_CLICK_RSP& rstSign30dClickRsp);
    int _AwardProp(PlayerData* pstData, int iAwardMonth, int iAwardDay, OUT SC_PKG_SIGN30D_CLICK_RSP& rstSign30dClickRsp);

private:

    tm m_stFirstTm;
    tm m_stLastTm;
    time_t m_lCurTimeSec;
    SSPKG m_stSsPkg;
};

