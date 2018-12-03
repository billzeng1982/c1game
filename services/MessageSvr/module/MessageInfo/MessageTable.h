#pragma once

#include "mysql/DataTable.h"
#include "common_proto.h"
#include "define.h"
#include "../../cfg/MessageSvrCfgDesc.h"
using namespace PKGMETA;

#define MESSAGE_LOW_MASK  0xffffffff
#define MESSAGE_BOX_BLOB 	"MessageBoxBlob"

class MessageTable : public CDataTable
{
public:
    static const int BOX_INFO_BINBUF_SIZE = MAX_LEN_MESSAGE_BOX_BLOB * 2 + 1;

public:
    MessageTable();
    virtual ~MessageTable();
    virtual bool CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, MESSAGESVRCFG* m_pstConfig);
    int GetMessageWholeData(uint64_t ullUin, uint8_t bChannel, OUT DT_MESSAGE_WHOLE_DATA& rstMessageWholeData);
    int UpdateMessageWholeData(IN DT_MESSAGE_WHOLE_DATA& rstMessageWholeData);
private:

    int _ConvertWholeToSql(IN DT_MESSAGE_WHOLE_DATA& rstMessageWholeData, OUT char* pszSql, int iSqlSize);
    int _ConvertBaseInfoToSql(IN DT_MESSAGE_BASE_INFO& rstMessageBoxInfo, OUT char* pszSql, int iSqlSize);
    int _ConvertBlobInfoToSql(IN char* pszData, int iLen, int iType, OUT char* pszSql, int iSqlSize);
private:
    char* m_szBoxBinBuf;
    MESSAGESVRCFG*				m_pstConfig;
/*	DT_MESSAGE_BOX_BLOB m_stBoxBlob;*/
};

