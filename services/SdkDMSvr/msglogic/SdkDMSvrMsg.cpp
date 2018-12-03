#include "strutil.h"
#include "../framework/SdkDMSvrMsgLayer.h"
#include "../module/TalkingData/TDataWorkThreadMgr.h"
#include "SdkDMSvrMsg.h"

using namespace PKGMETA;

int SdkDMTDataSendOrderNtf::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    SS_PKG_SDKDM_TDATA_SEND_ORDER_NTF& rstOrderNtf = rstSsPkg.m_stBody.m_stSdkDMTDataSendOrderNtf;      
    TDataWorkThreadMgr::Instance().SendReq(rstOrderNtf.m_stOrderMsg);
    return 0;
}
