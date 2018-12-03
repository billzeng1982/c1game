#pragma once

#include "mysql/DataTable.h"
#include "common_proto.h"
#include "define.h"

using namespace PKGMETA;


#define BLOB_NAME_GUILD_GLOBAL 	  "GlobalBlob"
#define BLOB_NAME_GUILD_MEMBER	 "MemberBlob"
#define BLOB_NAME_GUILD_APPLY	  "ApplyBlob"
#define BLOB_NAME_GUILD_REPLAY	  "ReplayBlob"
#define BLOB_NAME_GUILD_BOSS	  "BossBlob"
#define BLOB_NAME_GUILD_SCIENCE    "ScienceBlob"

class GuildTable : public CDataTable
{
public:
    static const int GLOBAL_INFO_BINBUF_SIZE = MAX_LEN_GUILD_GLOBAL * 2 + 1;
    static const int MEMBER_INFO_BINBUF_SIZE = MAX_LEN_GUILD_MEMBER * 2 + 1;
    static const int APPLY_INFO_BINBUF_SIZE = MAX_LEN_GUILD_APPLY * 2 + 1;
    static const int REPLAY_INFO_BINBUF_SIZE = MAX_LEN_GUILD_REPLAY * 2 + 1;
    static const int BOSS_INFO_BINBUF_SIZE = MAX_LEN_GUILD_BOSS * 2 + 1;
	static const int SCIENCE_INFO_BINBUF_SIZE = MAX_LEN_GUILD_SOCIETY * 2 + 1;
public:
    GuildTable();
    virtual ~GuildTable();

    virtual bool Init(const char* pszTableName, CMysqlHandler* poMysqlHandler, int iTableNum);
    int CheckExist(uint64_t ullGuildId);// 0 - no exist, 1 - exist, < 0 error
    int CheckExist(char* pszName);
    int CreateGuild(DT_GUILD_WHOLE_DATA& rstGuildWholeData);
    int DeleteGuild(uint64_t ullGuildId);
    int GetGuildWholeData(uint64_t ullGuildId, DT_GUILD_WHOLE_DATA& rstRoleWholeData);
    int GetGuildWholeData(char* pszName, DT_GUILD_WHOLE_DATA& rstRoleWholeData);
    int UpdateGuildWholeData(DT_GUILD_WHOLE_DATA& rstGuildWholeData);

private:
    int _ConvertWholeGuildToSql(DT_GUILD_WHOLE_DATA& rstGuildWholeData, char* pszSql, int iSqlSize);
    void _ConvertBaseInfoToSql(DT_GUILD_BASE_INFO& rstBaseInfo, char* pszSql, int iSqlSize);
    int _ConvertBlobInfoToSql(char* pszData, int iLen, int iType, char* pszSql, int iSqlSize);

private:
    char* m_szGlobalBinBuf;
    char* m_szMemberBinBuf;
    char* m_szApplyBinBuf;
    char* m_szReplayBinBuf;
    char* m_szBossBinBuf;
	char* m_szScienceBinbuf;
};