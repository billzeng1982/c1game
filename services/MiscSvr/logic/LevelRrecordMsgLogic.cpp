#include "LevelRrecordMsgLogic.h"
#include "LogMacros.h"
#include "../framework/MiscSvrMsgLayer.h"
#include "strutil.h"
#include "common_proto.h"
#include "../module/LevelRankTable.h"
#include "../thread/DBWorkThread.h"


using namespace PKGMETA;

int LevelRecordReadReq_SS::HandleServerMsg(SSPKG& rstSsPkg, void* pvPara)
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
	SS_PKG_LEVEL_RECORD_READ_RSP& rstLevelRecordRsp = m_stSsPkg.m_stBody.m_stLevelRecordReadRsp;
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_LEVEL_RECORD_READ_RSP;
    poWorkThread->GetLevelRankTabke().UpdateFromDB(rstLevelRecordRsp);
    poWorkThread->SendToZoneSvr(m_stSsPkg);

	return 0;
}



int LevelRecordSaveReq_SS::HandleServerMsg(SSPKG& rstSsPkg, void* pvPara)
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
	SS_PKG_LEVEL_RECORD_SAVE_REQ& rstLevelRecordReq = rstSsPkg.m_stBody.m_stLevelRecordSaveReq;
    poWorkThread->GetLevelRankTabke().UpdateToDB(rstLevelRecordReq);
	return 0;
}

