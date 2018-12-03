#pragma once

/*
    角色表db操作api, 注意所有api必须是Reentrant的
*/

#include "mysql/DataTable.h"
#include "ss_proto.h"
#include "define.h"
#include "singleton.h"
#include "../cfg/MiscSvrCfgDesc.h"


class CloneBattleTeamTable
{
public:
    static const int CONST_CLONEBATTLE_TEAM_INFO_BINBUF_SIZE = PKGMETA::MAX_LEN_CLONE_BATTLE_TEAM_BLOB *2 + 1;

public:
    CloneBattleTeamTable();
    virtual ~CloneBattleTeamTable();

    virtual bool CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, MISCSVRCFG* m_pstConfig);
    int GetData(PKGMETA::DT_CLONE_BATTLE_TEAM_DATA& rstTeamData);
    int UptData(PKGMETA::DT_CLONE_BATTLE_TEAM_DATA& rstTeamData);
    int DelData(uint64_t ullTeamIdList [], size_t uNum);
private:
    int _ConvertWholeDataToSql(IN PKGMETA::DT_CLONE_BATTLE_TEAM_DATA& rstData, INOUT char* pszSql, int iSqlSize);
    //void _ConvertBaseInfoToSql(IN PKGMETA::DT_MAIL_BASE_INFO& rstBaseInfo, INOUT char* pszSql, int iSqlSize);	
    int _ConvertBlobInfoToSql(char* pszData, int iLen, int iType, INOUT char* pszSql, int iSqlSize);

private:
    CMysqlHandler m_stMysqlHandler;
    char m_szTableName[255];
    char* m_szTeamBinBuf;
    MISCSVRCFG*	m_pstConfig;
};

