#include "AccountTable.h"
#include "LogMacros.h"
#include "pal/tstring.h"
#include "hash_func.h"
#include "../AccountSvr.h"

using namespace PKGMETA;

int AccountTable::GetAccountData( const char* pszAccoutName, const char* pszChannelID, uint64_t ullNewUin, PKGMETA::SSDT_ACCOUNT_DATA& rstAccData )
{
    if( !pszAccoutName )
    {
        assert( false );
        return ERR_WRONG_PARA;
    }

    rstAccData.m_chIsNew = 0;

    m_poMysqlHandler->FormatSql( "SELECT "
                                 "Uin, PassWord, AccountType, BanTime "
                                 "FROM tbl_account where binary AccountName='%s'", pszAccoutName );

    if( m_poMysqlHandler->Execute() < 0 )
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg() );
        return ERR_DB_ERROR;
    }

    int iRowEffected = m_poMysqlHandler->StoreResult();
    if( iRowEffected < 0 )
    {
        return ERR_DB_ERROR;
    }

    if( 1 == iRowEffected )
    {
        CMysqlRowIter& oRowIter = m_poMysqlHandler->GetRowIter();
        int i = 0;
        oRowIter.Begin( m_poMysqlHandler->GetResult() );

        rstAccData.m_ullUin = oRowIter.GetField(i++)->GetBigInteger();
        STRNCPY(rstAccData.m_szAccountPwd, oRowIter.GetField(i++)->GetString(), MAX_NAME_LENGTH);
        rstAccData.m_bAccountType = oRowIter.GetField(i++)->GetInteger();
        rstAccData.m_ullBanTime = oRowIter.GetField(i++)->GetInteger();
        return ERR_NONE;
    }

    ACCOUNTSVRCFG& rstConfig = AccountSvr::Instance().GetConfig();
    if (0 == iRowEffected)
    {
        rstAccData.m_chIsNew = 1;
        return this->CreateNewAccount( pszAccoutName, pszChannelID, ullNewUin, rstAccData );
    }

    LOGERR_r( "error row effected(%d) when get account(%s)", iRowEffected, pszAccoutName );

    return ERR_DB_ERROR;
}

int AccountTable::CreateNewAccount( const DT_ACCOUNT_INFO* pstAccInfo, uint64_t ullNewUin, int iParaNum )
{
    SSDT_ACCOUNT_DATA stAccData;

    if( iParaNum <= 0 )
    {
        LOGERR("Create accounts err iPara : %d", iParaNum);
        return -1;
    }

    int iRet = 0;
    for( int i = 0; i < iParaNum; i++)
    {
        if( pstAccInfo[i].m_szUserName == NULL || pstAccInfo[i].m_szUserName[0] == '\0' )
        {
            LOGERR("Invalid Name");
            return -2;
        }
        iRet = CreateNewAccount(pstAccInfo[i].m_szUserName, ullNewUin, pstAccInfo[i].m_szPassWord, stAccData);
        if( iRet )
        {
            return iRet;
        }
    }

    return 0;
}

int AccountTable::CreateNewAccount( const char* pszAccoutName, uint64_t ullNewUin, const char* pszPassWord, PKGMETA::SSDT_ACCOUNT_DATA& rstAccData )
{
    /*
    uint32_t dwHash = ::zend_inline_hash_func( pszAccoutName, strlen( pszAccoutName ) );
    int iTableNum = dwHash % m_iTableNum;
    iTableNum = iTableNum & 0x000000ff;
    iTableNum++;
    */
    //int iTableNum = GET_TABLE_NUM(ullNewUin);

    m_poMysqlHandler->FormatSql( "INSERT INTO tbl_account "
                                 "SET AccountName='%s', Uin=%lu "
                                 "PassWord='%s'",
                                 pszAccoutName,
                                 ullNewUin,
                                 pszPassWord);

    if( m_poMysqlHandler->Execute() < 0 )
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg() );
        return ERR_DB_ERROR;
    }
    rstAccData.m_ullUin = ullNewUin;
    return ERR_NONE;
}

int AccountTable::CreateNewAccount(const char* pszAccoutName, const char* pszChannelID, uint64_t ullNewUin, PKGMETA::SSDT_ACCOUNT_DATA& rstAccData )
{

	char szTime[MAX_TIME_STR_LENGTH];

	CGameTime::Instance().GetTimeFmtStr(szTime);

    //int iTableNum = GET_TABLE_NUM(ullNewUin);
    m_poMysqlHandler->FormatSql( "INSERT INTO tbl_account "
                                 "SET AccountName='%s', Uin=%lu, ChannelID='%s', CreateTime='%s' ",
                                 pszAccoutName ,ullNewUin, pszChannelID, szTime);

    if (m_poMysqlHandler->Execute() < 0)
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }
    rstAccData.m_ullUin = ullNewUin;
    rstAccData.m_bAccountType = 0;
    return ERR_NONE;
}
/*
int AccountTable::GetUin( const char* pszAccoutName,  uint64_t &uin )
{
	
    if( !pszAccoutName )
    {
        assert( false );
        return ERR_WRONG_PARA;
    }
    int iTableNum = m_pstConfig->m_iSvrID;

    m_poMysqlHandler->FormatSql( "SELECT "
                                 "Uin "
                                 "FROM tbl_account_%d where binary AccountName='%s'", iTableNum, pszAccoutName );

    if( m_poMysqlHandler->Execute() < 0 )
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg() );
        return ERR_DB_ERROR;
    }

    int iRowEffected = m_poMysqlHandler->StoreResult();
    if( iRowEffected < 0 )
    {
        return ERR_DB_ERROR;
    }

    if( 1 == iRowEffected )
    {
        CMysqlRowIter& oRowIter = m_poMysqlHandler->GetRowIter();
        int i = 0;
        oRowIter.Begin( m_poMysqlHandler->GetResult() );

        uin = oRowIter.GetField(i++)->GetBigInteger();
        return ERR_NONE;
    }

    LOGERR_r( "error row effected(%d) when get account(%s)", iRowEffected, pszAccoutName );

    return ERR_DB_ERROR;
}
*/
int AccountTable::GmUpdateAccountBanTime(uint64_t ullUin, uint64_t BanTime)
{
    //int iTableNum = GET_TABLE_NUM(ullUin);


    m_poMysqlHandler->FormatSql( "UPDATE tbl_account SET "
        "BanTime=%lu "
        "where Uin=%lu", BanTime, ullUin );
    if( m_poMysqlHandler->Execute() < 0 )
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg() );
        return ERR_DB_ERROR;
    }
    return ERR_NONE;;

}




bool AccountTable::CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, ACCOUNTSVRCFG* pstConfig)
{
    m_pstConfig  = pstConfig;
    if (NULL == pstConfig)
    {
        return false;
    }
    return CDataTable::Init(pszTableName, poMysqlHandler, m_pstConfig->m_iAccTableNum);
}






