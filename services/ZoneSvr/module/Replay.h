#pragma once
#include "define.h"
#include "singleton.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "cs_proto.h"
using namespace PKGMETA;

class Replay : public TSingleton<Replay>
{
public:
    Replay();
    ~Replay();
    void UpdateReplay(SS_PKG_REFRESH_REPLAYLIST_NTF& rstSsPkgBodyRsp);
    void GetReplayList(CS_PKG_REFRESH_REPLAYLIST_REQ& rstCsPkgBodyReq, SC_PKG_REFRESH_REPLAYLIST_RSP& rstScPkgBodyRsp);

 private:
    uint64_t m_ullLastUpdateTime;
    DT_WHOLE_REPLAY_LIST m_stReplayList;
};


