#pragma once
#include "define.h"
#include "singleton.h"
#include "common_proto.h"

using namespace PKGMETA;

class ZoneSvrMgr : public TSingleton<ZoneSvrMgr>
{
private:
    static const int IDLE_STATE_PLAYER_NUM_LIMIT = 1000;
    static const int BUSY_STATE_PLAYER_NUM_LIMIT = 2000;

public:
    ZoneSvrMgr();
    virtual ~ZoneSvrMgr(){}

    int HandleSvrStatNtf(PKGMETA::DT_SERVER_INFO& rstSvrInfo);
    DT_SERVER_LIST & GetSvrList();

private:
    uint32_t _GetSvrState(DT_SERVER_INFO& rstSvrInfo);

public:
    DT_SERVER_LIST m_stSvrList;
};

