#pragma once

#include "mysql/MysqlHandler.h"
#include "common_proto.h"
#include "singleton.h"
#include "../cfg/MiscSvrCfgDesc.h"
#include "ss_proto.h"

using namespace PKGMETA;

class LevelRankTable
{
public:
	static const int LEVEL_RANK_INFO_BINBUF_SIZE = PKGMETA::MAX_LEN_LEVEL_RECORD*2 + 1;
	LevelRankTable();
	~LevelRankTable();

	bool Init(MISCSVRCFG* pstConfig);

public:
	int UpdateFromDB(PKGMETA::SS_PKG_LEVEL_RECORD_READ_RSP& rstLevelRecordBlob);
	int UpdateToDB(PKGMETA::SS_PKG_LEVEL_RECORD_SAVE_REQ& rstLevelRecordBlob);

private:
	int _ConnectToDB();
	bool _MakeFirstData(PKGMETA::SS_PKG_LEVEL_RECORD_READ_RSP& rstLevelRecordBlob);
	int _InsertFirstData(PKGMETA::SS_PKG_LEVEL_RECORD_READ_RSP& rstLevelRecordBlob);
	int _CheckMysqlConn();
private:

	CMysqlHandler m_stMysqlHandler;
	MISCSVRCFG* m_pstConfig;
	char* m_szLevelRankBinBuf;
};