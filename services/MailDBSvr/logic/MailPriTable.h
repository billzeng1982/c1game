#pragma once

/*
    角色表db操作api, 注意所有api必须是Reentrant的
*/

#include "mysql/DataTable.h"
#include "ss_proto.h"
#include "define.h"
#include "../cfg/MailDBSvrCfgDesc.h"
#define BLOB_NAME_MAIL_BOX_PRI 	"MailBoxPriBlob"

class MailPriTable : public CDataTable
{
public:
    static const int MAIL_BOX_PRI_INFO_BINBUF_SIZE = PKGMETA::MAX_LEN_MAIL_BOX_PRI*2 + 1;

public:
    MailPriTable();
    virtual ~MailPriTable();

    virtual bool CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, MAILDBSVRCFG*	m_pstConfig);

    int CreateData(IN PKGMETA::DT_MAIL_BOX_DATA& rstData);
    int SaveData(IN PKGMETA::DT_MAIL_BOX_DATA& rstData);
    int GetData(uint64_t ullUin, OUT PKGMETA::DT_MAIL_BOX_DATA& rstData);

    int CheckExist(uint64_t ullUin);	// 0 - no exist, 1 - exist, < 0 error

private:
    int _ConvertWholeDataToSql(IN PKGMETA::DT_MAIL_BOX_DATA& rstData, INOUT char* pszSql, int iSqlSize);
    void _ConvertBaseInfoToSql(IN PKGMETA::DT_MAIL_BASE_INFO& rstBaseInfo, INOUT char* pszSql, int iSqlSize);
    int _ConvertBlobInfoToSql(char* pszData, int iLen, int iType, INOUT char* pszSql, int iSqlSize);

private:
    char* m_szMailBoxPriBinBuf;
    MAILDBSVRCFG*	m_pstConfig;
};

