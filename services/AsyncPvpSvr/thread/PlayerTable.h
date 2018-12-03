#pragma once

#include "mysql/DataTable.h"
#include "common_proto.h"
#include "define.h"
#include "AsyncPvpSvrCfgDesc.h"

using namespace PKGMETA;

#define BLOB_NAME_ASYNCPVP_PLAYER_DATA     "DataBlob"


class PlayerTable : public CDataTable
{
public:
    static const int DATA_INFO_BINBUF_SIZE = MAX_LEN_ASYNC_PVP_PLAYER_DATA_BLOB * 2 + 1;
public:
    PlayerTable();
    virtual ~PlayerTable();

    virtual bool CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, ASYNCPVPSVRCFG* pstConfig);
    int CheckExist(uint64_t ullUin);// 0 - no exist, 1 - exist, < 0 error
    int CreatePlayer(DT_ASYNC_PVP_PLAYER_WHOLE_DATA& rstPlayerData);
    int DeletePlayer(uint64_t ullUin);
    int GetPlayer(uint64_t ullUin, DT_ASYNC_PVP_PLAYER_WHOLE_DATA& rstPlayerData);
    int UpdatePlayer(DT_ASYNC_PVP_PLAYER_WHOLE_DATA& rstPlayerData);

private:
    int _ConvertPlayerToSql(DT_ASYNC_PVP_PLAYER_WHOLE_DATA& rstPlayerData, char* pszSql, int iSqlSize);

private:
    char* m_szDataBinBuf;
    ASYNCPVPSVRCFG* m_pstConfig;
};

