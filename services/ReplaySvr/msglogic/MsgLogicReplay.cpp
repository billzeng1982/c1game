#include <string.h>
#include "strutil.h"
#include "LogMacros.h"
#include "MsgLogicReplay.h"
#include "common_proto.h"
#include "../framework/ReplaySvrMsgLayer.h"
#include "../module/ReplayMgr.h"

using namespace PKGMETA;

int UploadReplayReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_UPLOAD_REPLAY_REQ & rstSsPkgBodyReq = rstSsPkg.m_stBody.m_stUploadReplayReq;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_UPLOAD_REPLAY_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    SS_PKG_UPLOAD_REPLAY_RSP & rstUploadReplayRsp = m_stSsPkg.m_stBody.m_stUploadReplayRsp;
    bzero(&rstUploadReplayRsp, sizeof(rstUploadReplayRsp));

    int iRet = ReplayMgr::Instance().UploadReplay(&rstSsPkgBodyReq.m_stFileHead, rstUploadReplayRsp.m_szURL);
    rstUploadReplayRsp.m_nErrNo= iRet;

    ReplaySvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return 1;
}

