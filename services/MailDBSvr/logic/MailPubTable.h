#pragma once

/*
    角色表db操作api, 注意所有api必须是Reentrant的
*/

#include "mysql/DataTable.h"
#include "ss_proto.h"
#include "define.h"
#include "../cfg/MailDBSvrCfgDesc.h"



#define BLOB_NAME_MAIL_PUB 		"MailDataBlob"



class MailPubTable : public CDataTable
{
public:
    static const int MAIL_BOX_PUB_INFO_BINBUF_SIZE = PKGMETA::MAX_LEN_MAIL_PUB_BLOB*2 + 1; 

public:
    MailPubTable();
    virtual ~MailPubTable();

    virtual bool CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, MAILDBSVRCFG* m_pstConfig);

    
    //  删除一封
    int DelData(uint32_t dwId);

    //  增加一封
    int AddData(IN PKGMETA::DT_MAIL_PUB_SER_DATA& rstData);
    int CheckExist(uint32_t ullUin);	// 0 - no exist, 1 - exist, < 0 error
    
    //  获取所有公共邮件,只会在服务器启动时调用
    //      多次调用 GetOnceData();
    //int GetAllData(CDBWorkThread* poWorkThread);

    //  从数据库中获取一次,每次是MAX_MAIL_BOX_PUBLIC_NUM封
    int GetOnceData(PKGMETA::SS_PKG_MAIL_PUB_TABLE_GET_DATA_RSP& rstAddRsp, int iLimitStart, int iLimitEnd);
private:
    int _ConvertWholeDataToSql(IN PKGMETA::DT_MAIL_PUB_SER_DATA& rstData, INOUT char* pszSql, int iSqlSize);
    //void _ConvertBaseInfoToSql(IN PKGMETA::DT_MAIL_BASE_INFO& rstBaseInfo, INOUT char* pszSql, int iSqlSize);	
    int _ConvertBlobInfoToSql(char* pszData, int iLen, int iType, INOUT char* pszSql, int iSqlSize);

private:
    char* m_szMailBoxPubBinBuf;
    MAILDBSVRCFG*	m_pstConfig;
};

