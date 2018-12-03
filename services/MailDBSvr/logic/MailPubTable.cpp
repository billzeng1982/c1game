#include "MailPubTable.h"
#include "LogMacros.h"
#include "comm_func.h"
#include "strutil.h"
#include "TableDefine.h"
#include "../MailDBSvr.h"
#include "PKGMETA_metalib.h"

using namespace PKGMETA;

MailPubTable::MailPubTable()
{
    m_szMailBoxPubBinBuf = NULL;
}

MailPubTable::~MailPubTable()
{
    SAFE_DEL_ARRAY(m_szMailBoxPubBinBuf);
}

bool MailPubTable::CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, MAILDBSVRCFG* pstConfig)
{
    if (NULL == pstConfig)
    {
        return false;
    }
    m_pstConfig = pstConfig;
    CDataTable::Init( pszTableName, poMysqlHandler, m_pstConfig->m_iTableNum);

    m_szMailBoxPubBinBuf = new char[MAIL_BOX_PUB_INFO_BINBUF_SIZE];
    if( !m_szMailBoxPubBinBuf ) return false;

    return true;
}

int MailPubTable::_ConvertWholeDataToSql( IN PKGMETA::DT_MAIL_PUB_SER_DATA& rstData, INOUT char* pszSql, int iSqlSize )
{
    assert( pszSql );
    int iRet = 0;

    StrCat(pszSql, iSqlSize, " Id=%u, Version=%d, DelFlag=0, StartTime=%lu, EndTime=%lu ", rstData.m_dwId, PKGMETA::MetaLib::getVersion(),
        rstData.m_ullStartTimeSec, rstData.m_ullEndTimeSec);
    SQL_ADD_DELIMITER( pszSql, iSqlSize );
    DT_MAIL_PUB_BLOB stMailPubBlob = {0};
    size_t ulUseSize = 0;
    iRet = rstData.pack((char*)stMailPubBlob.m_szData, sizeof(stMailPubBlob.m_szData), &ulUseSize, PKGMETA::MetaLib::getVersion());
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("MailId<%u> pack DT_MAIL_PUB_BLOB failed!", rstData.m_dwId);
        return -1;
    }
    stMailPubBlob.m_iLen = (int32_t) ulUseSize;
    iRet = this->_ConvertBlobInfoToSql((char*)stMailPubBlob.m_szData, stMailPubBlob.m_iLen, MAIL_DATA_TYPE_MAIL_PUB, pszSql, iSqlSize);
    if (iRet < 0)
        return iRet;

    return ERR_NONE;
}


int MailPubTable::_ConvertBlobInfoToSql( char* pszData, int iLen, int iType, INOUT char* pszSql, int iSqlSize )
{
    if( !pszData || iLen < 0 )
    {
        return ERR_SYS;
    }

    char* pszBinBuf = NULL;
    int iBufSize = 0;
    const char* pszBlobName = NULL;

    switch (iType)
    {
        case MAIL_DATA_TYPE_MAIL_PUB:
            pszBinBuf = m_szMailBoxPubBinBuf;
            iBufSize = MAIL_BOX_PUB_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_MAIL_PUB;
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

int MailPubTable::CheckExist( uint32_t dwId )
{
    //int iTableNum = 1;


    m_poMysqlHandler->FormatSql( "SELECT Id FROM %s WHERE Id=%lu", MAIL_PUB_TABLE_NAME, dwId );
    if( m_poMysqlHandler->Execute() < 0 )
    {
        return ERR_DB_ERROR;
    }

    return m_poMysqlHandler->StoreResult();
}


int MailPubTable::AddData(IN PKGMETA::DT_MAIL_PUB_SER_DATA& rstData)
{
    //int iTableNum = 1;


    // check is exist
    int iRet = this->CheckExist( rstData.m_dwId );
    if( iRet < 0 )
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg() );
        return ERR_DB_ERROR;
    }
    if( iRet > 0 )
    {
        return ERR_ALREADY_EXISTED;
    }

    char* pszSql = m_poMysqlHandler->GetSql();
    pszSql[0] = '\0';

    // base info
    StrCat( pszSql, DB_SQL_STR_SIZE, "INSERT INTO %s SET ", MAIL_PUB_TABLE_NAME);

    iRet = this->_ConvertWholeDataToSql( rstData, pszSql, DB_SQL_STR_SIZE );
    if( iRet < 0 ) return iRet;

    if( m_poMysqlHandler->Execute( pszSql ) < 0 )
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg() );
        return ERR_DB_ERROR;
    }
    return ERR_NONE;
}

int MailPubTable::GetOnceData(SS_PKG_MAIL_PUB_TABLE_GET_DATA_RSP& rstAddRsp, int iLimitStart, int iLimitEnd)
{
    //int iTableNum = 1;
    uint64_t ullNowTime = CGameTime::Instance().GetCurrSecond();
    m_poMysqlHandler->FormatSql( "SELECT "
        "%s "
        /*注意 逗号 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! "%s," */
        "FROM %s WHERE DelFlag=0 and EndTime>%lu limit %d, %d ",
        BLOB_NAME_MAIL_PUB, MAIL_PUB_TABLE_NAME, ullNowTime, iLimitStart, iLimitEnd);
    if( m_poMysqlHandler->Execute() < 0 )
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg() );
        return ERR_DB_ERROR;
    }
    int iSqlRetNum = m_poMysqlHandler->StoreResult();

    CMysqlRowIter& oRowIter = m_poMysqlHandler->GetRowIter();
    oRowIter.Begin( m_poMysqlHandler->GetResult() );
    DT_MAIL_PUB_BLOB stMailPubBlob = {0};
    int iRet = 0;
    for (int i = 0; i < iSqlRetNum && i < MAX_MAIL_BOX_PUBLIC_NUM && !oRowIter.IsEnd(); i++)
    {
        stMailPubBlob.m_iLen = oRowIter.GetField(0)->GetBinData((char*)stMailPubBlob.m_szData, MAX_LEN_MAIL_PUB_BLOB);
        iRet = rstAddRsp.m_astPubMails[rstAddRsp.m_wNum].unpack((char*)stMailPubBlob.m_szData, sizeof(stMailPubBlob.m_szData)) ;
        if (iRet != TdrError::TDR_NO_ERROR)
        {
            LOGERR_r(" unpack DT_MAIL_PUB_BLOB failed ");
            continue;
        }
        rstAddRsp.m_wNum++;
        oRowIter.Next();
    }
    return iSqlRetNum;
}

int MailPubTable::DelData(uint32_t dwId)
{
    int iTableNum = 1;

    m_poMysqlHandler->FormatSql("UPDATE %s SET DelTag=1 WHERE Id=%u ", MAIL_PUB_TABLE_NAME,  dwId);

    if( m_poMysqlHandler->Execute() < 0 ||
        m_poMysqlHandler->AffectedRows() != 1 )
    {
        LOGERR_r("Del pub mail error %s", m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }

    return ERR_NONE;
}






