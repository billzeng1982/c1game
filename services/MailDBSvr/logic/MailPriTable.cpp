#include "MailPriTable.h"
#include "LogMacros.h"
#include "comm_func.h"
#include "strutil.h"
#include "TableDefine.h"
#include "../MailDBSvr.h"
#include "PKGMETA_metalib.h"

using namespace PKGMETA;

MailPriTable::MailPriTable()
{
    m_szMailBoxPriBinBuf = NULL;
}

MailPriTable::~MailPriTable()
{
    SAFE_DEL_ARRAY(m_szMailBoxPriBinBuf);
}

bool MailPriTable::CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, MAILDBSVRCFG*	pstConfig )
{
    if (NULL == pstConfig)
    {
        return false;
    }
    m_pstConfig = pstConfig;
    CDataTable::Init( pszTableName, poMysqlHandler, m_pstConfig->m_iTableNum );

    m_szMailBoxPriBinBuf = new char[MAIL_BOX_PRI_INFO_BINBUF_SIZE];
    if( !m_szMailBoxPriBinBuf ) return false;

    return true;
}

int MailPriTable::CreateData( IN PKGMETA::DT_MAIL_BOX_DATA& rstData )
{
    //int iTableNum = GET_TABLE_NUM(rstData.m_stBaseInfo.m_ullUin);

    // check is exist
    int iRet = this->CheckExist( rstData.m_stBaseInfo.m_ullUin );
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
    StrCat( pszSql, DB_SQL_STR_SIZE, "INSERT INTO %s SET ", MAIL_PRI_TABLE_NAME );

    DT_MAIL_BOX_INFO stMailBoxInfo;
    stMailBoxInfo.m_wPublicCount = 0;
    stMailBoxInfo.m_wPrivateCount = 0;

    size_t ulUseSize = 0;
    iRet = stMailBoxInfo.pack((char*)rstData.m_stMailBoxBlob.m_szData, MAX_LEN_MAIL_BOX_PRI, &ulUseSize);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("MailTable(%lu) Create failed, pack failed, Ret=%d", rstData.m_stBaseInfo.m_ullUin, iRet);
        return ERR_SYS;
    }
    rstData.m_stMailBoxBlob.m_iLen = (int)ulUseSize;

    rstData.m_stBaseInfo.m_dwPriSeq = 0;
    rstData.m_stBaseInfo.m_dwPubSeq = 0;
    rstData.m_stBaseInfo.m_wVersion = PKGMETA::MetaLib::getVersion();

    iRet = this->_ConvertWholeDataToSql( rstData, pszSql, DB_SQL_STR_SIZE );
    if( iRet < 0 ) return iRet;

    if( m_poMysqlHandler->Execute( pszSql ) < 0 )
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg() );
        return ERR_DB_ERROR;
    }

    return ERR_NONE;
}

int MailPriTable::SaveData( IN PKGMETA::DT_MAIL_BOX_DATA& rstData )
{
    //int iTableNum = GET_TABLE_NUM(rstData.m_stBaseInfo.m_ullUin);


    int iRet = 0;
    char* pszSql = m_poMysqlHandler->GetSql();
    pszSql[0] = '\0';

    // base info
    StrCat( pszSql, DB_SQL_STR_SIZE, "UPDATE %s SET ", MAIL_PRI_TABLE_NAME );
    iRet = this->_ConvertWholeDataToSql( rstData, pszSql, DB_SQL_STR_SIZE );
    if( iRet < 0 ) return iRet;
    StrCat( pszSql, DB_SQL_STR_SIZE, "WHERE Uin=%lu", rstData.m_stBaseInfo.m_ullUin );

    if ( m_poMysqlHandler->Execute( pszSql ) < 0  )
    {
        LOGERR_r( "Save role data failed! Uin <%lu>, Reason:<%s>", rstData.m_stBaseInfo.m_ullUin, m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }

    if( m_poMysqlHandler->AffectedRows() != 1 )
    {
        LOGERR_r( "Save role data failed! Uin <%lu>", rstData.m_stBaseInfo.m_ullUin);
        return ERR_DB_ERROR;
    }
    return ERR_NONE;
}

int MailPriTable::_ConvertWholeDataToSql( IN PKGMETA::DT_MAIL_BOX_DATA& rstData, INOUT char* pszSql, int iSqlSize )
{
    assert( pszSql );
    int iRet = 0;

    this->_ConvertBaseInfoToSql( rstData.m_stBaseInfo, pszSql, iSqlSize );
    SQL_ADD_DELIMITER( pszSql, iSqlSize );

    iRet = this->_ConvertBlobInfoToSql((char*)rstData.m_stMailBoxBlob.m_szData, rstData.m_stMailBoxBlob.m_iLen, MAIL_DATA_TYPE_MAILBOX_PRI, pszSql, iSqlSize);
    if (iRet < 0)
        return iRet;

    return ERR_NONE;
}

void MailPriTable::_ConvertBaseInfoToSql( IN PKGMETA::DT_MAIL_BASE_INFO& rstBaseInfo, INOUT char* pszSql, int iSqlSize )
{
    assert( pszSql );
    StrCat(pszSql, iSqlSize, "Uin=%lu, PriSeq=%u, PubSeq=%u, Version=%hu",
                         rstBaseInfo.m_ullUin, rstBaseInfo.m_dwPriSeq, rstBaseInfo.m_dwPubSeq, rstBaseInfo.m_wVersion);

    return;
}

int MailPriTable::_ConvertBlobInfoToSql( char* pszData, int iLen, int iType, INOUT char* pszSql, int iSqlSize )
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
        case MAIL_DATA_TYPE_MAILBOX_PRI:
            pszBinBuf = m_szMailBoxPriBinBuf;
            iBufSize = MAIL_BOX_PRI_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_MAIL_BOX_PRI;
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

int MailPriTable::CheckExist( uint64_t ullUin )
{
    int iTableNum = GET_TABLE_NUM(ullUin);


    m_poMysqlHandler->FormatSql( "SELECT Uin FROM %s WHERE Uin=%lu", MAIL_PRI_TABLE_NAME, ullUin );
    if( m_poMysqlHandler->Execute() < 0 )
    {
        return ERR_DB_ERROR;
    }

    return m_poMysqlHandler->StoreResult();
}

int MailPriTable::GetData( uint64_t ullUin, OUT PKGMETA::DT_MAIL_BOX_DATA& rstData )
{
    if( 0 == ullUin )
    {
        return ERR_WRONG_PARA;
    }

    //int iTableNum = GET_TABLE_NUM(ullUin );
    m_poMysqlHandler->FormatSql( "SELECT PriSeq, PubSeq, Version, "
                                 "%s " /*1*/
                                 /*注意 逗号 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! "%s," */
                                 "FROM %s WHERE Uin=%lu",
                                 BLOB_NAME_MAIL_BOX_PRI, /*1*/
                                 MAIL_PRI_TABLE_NAME, ullUin );
    if( m_poMysqlHandler->Execute( ) < 0 )
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg() );
        return ERR_DB_ERROR;
    }

    int iEffectRow = m_poMysqlHandler->StoreResult();
    if( iEffectRow < 0 )
    {
        return ERR_DB_ERROR;
    }
    if( 0 == iEffectRow )
    {
        return ERR_NOT_FOUND;
    }

    assert( 1 == iEffectRow );

    CMysqlRowIter& oRowIter = m_poMysqlHandler->GetRowIter();
    int i = 0;
    oRowIter.Begin(m_poMysqlHandler->GetResult());

    DT_MAIL_BASE_INFO& rstBaseInfo = rstData.m_stBaseInfo;
    rstBaseInfo.m_dwPriSeq = (uint32_t)oRowIter.GetField(i++)->GetInteger();
    rstBaseInfo.m_dwPubSeq = (uint32_t)oRowIter.GetField(i++)->GetInteger();
    rstBaseInfo.m_wVersion = (uint16_t)oRowIter.GetField(i++)->GetInteger();
    rstData.m_stMailBoxBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstData.m_stMailBoxBlob.m_szData, MAX_LEN_MAIL_BOX_PRI);

    return ERR_NONE;
}


