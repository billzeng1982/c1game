#pragma once

#include "mysql/DataTable.h"
#include "common_proto.h"
#include "define.h"
#include "../cfg/GuildSvrCfgDesc.h"

using namespace PKGMETA;

#define BLOB_NAME_GUILD_PLAYER_APPLY	    "ApplyBlob"


class PlayerTable : public CDataTable
{
public:
    static const int APPLY_INFO_BINBUF_SIZE = MAX_LEN_GUILD_APPLY * 2 + 1;
public:
    PlayerTable();
    virtual ~PlayerTable();

    virtual bool CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, GUILDSVRCFG* pstConfig);
    int CheckExist(uint64_t ullUin);// 0 - no exist, 1 - exist, < 0 error
    int CreatePlayer(DT_GUILD_PLAYER_DATA& rstPlayerData);
    int DeletePlayer(uint64_t ullUin);
    int GetPlayerData(uint64_t ullUin, DT_GUILD_PLAYER_DATA& rstPlayerData);
    int UpdatePlayerData(DT_GUILD_PLAYER_DATA& rstPlayerData);

private:
    int _ConvertPlayerToSql(DT_GUILD_PLAYER_DATA& rstPlayerData, char* pszSql, int iSqlSize);
    void _ConvertBaseInfoToSql(DT_GUILD_PLAYER_BASE_INFO& rstBaseInfo, char* pszSql, int iSqlSize);
    int _ConvertBlobInfoToSql(char* pszData, int iLen, int iType, char* pszSql, int iSqlSize);

private:
    char* m_szApplyBinBuf;
    GUILDSVRCFG* m_pstConfig;
};

