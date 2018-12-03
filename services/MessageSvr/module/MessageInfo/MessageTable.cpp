#include "MessageTable.h"
#include "LogMacros.h"
#include "comm_func.h"
#include "strutil.h"
#include "TableDefine.h"
#include "hash_func.h"

using namespace PKGMETA;

MessageTable::MessageTable()
{
    m_szBoxBinBuf = NULL;
}

MessageTable::~MessageTable()
{
     SAFE_DEL_ARRAY(m_szBoxBinBuf);

}

//  初始化, 分配空间
bool MessageTable::CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, MESSAGESVRCFG* pstConfig)
{
    if (NULL == pstConfig)
    {
        return false;
    }
    m_pstConfig = pstConfig;
    if(!CDataTable::Init(pszTableName, poMysqlHandler, m_pstConfig->m_iSvrID))
    {
        return false;
    }
    m_szBoxBinBuf = new char[BOX_INFO_BINBUF_SIZE];
    if (!m_szBoxBinBuf )
    {
        return false;
    }
    return true;
}

int MessageTable::_ConvertBlobInfoToSql(char* pszData, int iLen, int iType, char* pszSql, int iSqlSize)
{
    if( !pszData || iLen < 0 )
    {
        return ERR_SYS;
    }

    int iBufSize = 0;
    char* pszBinBuf = NULL;
    const char* pszBlobName = NULL;

    switch( iType )
    {
    case MESSAGE_DATA_TYPE_BOX:
        pszBinBuf = m_szBoxBinBuf;
        iBufSize = BOX_INFO_BINBUF_SIZE;
        pszBlobName = MESSAGE_BOX_BLOB;
        break;
    default:
        return ERR_SYS;
    }

    if (!m_poMysqlHandler->BinToStr(pszData, iLen, pszBinBuf, iBufSize))
    {
        LOGERR_r( "convert to mysql bin failed, type <%d>", iType );
        return ERR_SYS;
    }

    StrCat(pszSql, iSqlSize, "`%s`='%s'", pszBlobName, pszBinBuf);

    return ERR_NONE;
}

int MessageTable::_ConvertBaseInfoToSql(DT_MESSAGE_BASE_INFO& rstMessageBaseInfo, char* pszSql, int iSqlSize)
{
    assert(pszSql);
    StrCat(pszSql, iSqlSize, "Uin=%lu, `Channel`='%hhu' ",
        rstMessageBaseInfo.m_ullUin, rstMessageBaseInfo.m_bChannel);
    return ERR_NONE;
}

int MessageTable::_ConvertWholeToSql(IN DT_MESSAGE_WHOLE_DATA& rstMessageWholeData, OUT char* pszSql, int iSqlSize)
{
    assert(pszSql);
    int iRet = 0;
    this->_ConvertBaseInfoToSql(rstMessageWholeData.m_stBaseInfo, pszSql, iSqlSize);
    SQL_ADD_DELIMITER(pszSql, iSqlSize);
    iRet = this->_ConvertBlobInfoToSql( (char*)rstMessageWholeData.m_stBoxBlob.m_szData, rstMessageWholeData.m_stBoxBlob.m_iLen, MESSAGE_DATA_TYPE_BOX, pszSql, iSqlSize );
    if( iRet < 0 )
    {
        return iRet;
    }
    return ERR_NONE;
}

int MessageTable::GetMessageWholeData(uint64_t ullUin, uint8_t bChannel, OUT DT_MESSAGE_WHOLE_DATA& rstMessageWholeData)
{
    //int iTableNum = GET_TABLE_NUM(ullUin);
    rstMessageWholeData.m_stBaseInfo.m_ullUin = ullUin;
    rstMessageWholeData.m_stBaseInfo.m_bChannel = bChannel;
    m_poMysqlHandler->FormatSql("SELECT MessageBoxBlob FROM %s WHERE Uin='%lu' and `Channel`='%hhu'", 
        MESSAGE_TABLE_NAME, ullUin, bChannel);
    if (m_poMysqlHandler->Execute() < 0)
    {
        LOGERR_r("excute sql failed: %s, Uin <%lu> Channel<%hhu> ",
            m_poMysqlHandler->GetLastErrMsg(), ullUin, bChannel);
        return ERR_DB_ERROR;
    }
    int iEffectRow = m_poMysqlHandler->StoreResult();
    if (iEffectRow < 0)
    {
        return ERR_DB_ERROR;
    }
    if (0 == iEffectRow)
    {
        return ERR_NOT_FOUND;
    }
    assert(1 == iEffectRow);
    CMysqlRowIter& oRowIter = m_poMysqlHandler->GetRowIter();
    oRowIter.Begin(m_poMysqlHandler->GetResult());
    rstMessageWholeData.m_stBoxBlob.m_iLen = oRowIter.GetField(0)->GetBinData((char*)rstMessageWholeData.m_stBoxBlob.m_szData, MAX_LEN_MESSAGE_BOX_BLOB);

    return ERR_NONE;
}



int MessageTable::UpdateMessageWholeData( IN DT_MESSAGE_WHOLE_DATA& rstMessageWholeData )
{
    uint64_t ullUin = rstMessageWholeData.m_stBaseInfo.m_ullUin;
    uint8_t bChannel = rstMessageWholeData.m_stBaseInfo.m_bChannel;
    //int iTableNum = GET_TABLE_NUM(ullUin);
    char* pszSql = m_poMysqlHandler->GetSql();
    
    pszSql[0] = '\0';

    // base info
    StrCat( pszSql, DB_SQL_STR_SIZE, " REPLACE INTO %s SET ", MESSAGE_TABLE_NAME);
    int iRet = this->_ConvertWholeToSql(rstMessageWholeData, pszSql, DB_SQL_STR_SIZE);
    if( iRet < 0 )
    {
        return iRet;
    }
    //StrCat(pszSql, DB_SQL_STR_SIZE, " WHERE Uin=%lu and `Channel`='%hhu' ", ullUin, bChannel);

    if( m_poMysqlHandler->Execute( pszSql ) < 0 /*||m_poMysqlHandler->AffectedRows() != 1*/ )
    {
        LOGERR_r("UpdateWholeData:excute sql failed: %s, SqlStr<%s>", m_poMysqlHandler->GetLastErrMsg(), pszSql);
        return ERR_DB_ERROR;
    }

    return ERR_NONE;
}





