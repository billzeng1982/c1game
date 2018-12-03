#pragma once

#include "mysql/DataTable.h"
#include "common_proto.h"
#include "define.h"
#include "AsyncPvpSvrCfgDesc.h"

using namespace PKGMETA;

#define BLOB_NAME_ASYNCPVP_TEAM_DATA     "TeamBlob"


class TeamTable : public CDataTable
{
public:
    static const int DATA_INFO_BINBUF_SIZE = MAX_LEN_ASYNC_PVP_PLAYER_DATA_BLOB * 2 + 1;
public:
    TeamTable();
    virtual ~TeamTable();

    virtual bool CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, ASYNCPVPSVRCFG* pstConfig);
    int CheckExist(uint64_t ullUin);// 0 - no exist, 1 - exist, < 0 error
    int CreateTeam(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData);
    int DeleteTeam(uint64_t ullUin);
    int GetTeam(uint64_t ullUin, DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData);
    int UpdateTeam(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData);

private:
    int _ConvertTeamToSql(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData, char* pszSql, int iSqlSize);

private:
    char* m_szDataBinBuf;
    ASYNCPVPSVRCFG* m_pstConfig;
};

