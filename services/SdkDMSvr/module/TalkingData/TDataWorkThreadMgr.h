#pragma once

#include "define.h"
#include "singleton.h"
#include "mempool.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "LogMacros.h"
#include "TDataWorkThread.h"
#include "../../cfg/SdkDMSvrCfgDesc.h"



using namespace PKGMETA;


class TDataWorkThreadMgr : public TSingleton<TDataWorkThreadMgr>
{

public:
    TDataWorkThreadMgr() {}
    virtual ~TDataWorkThreadMgr() {}
public:
    bool Init(SDKDMSVRCFG* pstConfig) ;
    void SendReq(DT_TDATA_ODER_INFO& rstReq);
    void Fini();
    //

private:
    TDataWorkThread* m_astWorkThreads;
    int m_iWorkThreadNum;
    int m_iWorkThreadIter;
};

