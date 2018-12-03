#pragma once
#include "define.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "singleton.h"


using namespace std;
using namespace PKGMETA;

class SdkDMMgr : public TSingleton<SdkDMMgr>
{
public:
    SdkDMMgr() {}
    virtual ~SdkDMMgr() {}
    void TDataSendOrder(DT_TDATA_ODER_INFO& rstInfo);
    void TestSend( uint64_t ullNum );
private:
    SSPKG m_stSsPkg;
};

