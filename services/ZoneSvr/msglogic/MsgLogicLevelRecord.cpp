#include "MsgLogicLevelRecord.h"
#include "LogMacros.h"
#include "ss_proto.h"
#include "../module/LevelRank.h"

using namespace PKGMETA;

int LevelRecordReadRsp_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
	SS_PKG_LEVEL_RECORD_READ_RSP& rstLevelRecordReadRsp = rstSsPkg.m_stBody.m_stLevelRecordReadRsp;
	LevelRank::Instance().SetRecordBlob(rstLevelRecordReadRsp);
	return 0;
}