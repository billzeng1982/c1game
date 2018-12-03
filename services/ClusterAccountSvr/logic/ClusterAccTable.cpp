/*
    注意分表是按 SdkUserName做的hash, 不要用uin, 因为外部是通过SdkUserName
*/

#include "ClusterAccTable.h"
#include "LogMacros.h"
#include "pal/tstring.h"
#include "hash_func.h"
#include "../ClusterAccountSvr.h"

using namespace PKGMETA;

// 只插入数据
int ClusterAccTable::CreateNewAccount( SS_PKG_CLUSTER_ACC_NEW_ROLE_REG& rNewRoleReg )
{
    char szTime[MAX_TIME_STR_LENGTH];

    CGameTime::Instance().GetTimeFmtStr(szTime);

    int iTableId = (zend_inline_hash_func(rNewRoleReg.m_szSdkUserName, strlen(rNewRoleReg.m_szSdkUserName))) % m_pstConfig->m_iAccTableNum + 1;
    
    m_poMysqlHandler->FormatSql( "INSERT INTO %s_%d "
                                 "SET Uin=%lu, SdkUserName='%s', RoleName='%s', ServerID=%d, ChannelID='%s', CreateTime='%s' ",
                                 m_szTableName, iTableId, 
                                 rNewRoleReg.m_ullUin, rNewRoleReg.m_szSdkUserName, rNewRoleReg.m_szRoleName, rNewRoleReg.m_iServerID, rNewRoleReg.m_szChannelID, szTime);

    if (m_poMysqlHandler->Execute() < 0)
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }
    
    return ERR_NONE;
}

int ClusterAccTable::ChangeRoleName(uint64_t ullUin, char* szSdkUserName, char* szRoleName )
{
    if( szRoleName[0] == '\0' )
    {
        assert( false );
        return ERR_WRONG_PARA;
    }

    int iTableId = (zend_inline_hash_func(szSdkUserName, strlen(szSdkUserName))) % m_pstConfig->m_iAccTableNum + 1; 

    m_poMysqlHandler->FormatSql( "UPDATE %s_%d "
                                 "SET RoleName='%s' where Uin=%lu",
                                 m_szTableName, iTableId, 
                                 szRoleName, ullUin );

    if (m_poMysqlHandler->Execute() < 0)
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }
    
    return ERR_NONE;
}

int ClusterAccTable::GetAccInfo( char* szSdkUserName, int iServerID, SS_PKG_GET_ONE_CLUSTER_ACC_INFO_RSP& rstGetAccInfoRsp )
{
    bzero(&rstGetAccInfoRsp, sizeof(rstGetAccInfoRsp));

    if( szSdkUserName[0] == '\0' )
    {
        assert( false );
        return ERR_WRONG_PARA;
    }

    int iTableId = (zend_inline_hash_func(szSdkUserName, strlen(szSdkUserName))) % m_pstConfig->m_iAccTableNum + 1; 

    m_poMysqlHandler->FormatSql( "SELECT "
                                 "Uin, RoleName "
                                 "FROM %s_%d where SdkUserName='%s' and ServerID=%d", 
                                 m_szTableName, iTableId,
                                 szSdkUserName, iServerID );

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

        rstGetAccInfoRsp.m_ullUin = oRowIter.GetField(i++)->GetBigInteger();
        STRNCPY( rstGetAccInfoRsp.m_szRoleName, oRowIter.GetField(i++)->GetString(), MAX_NAME_LENGTH);

        rstGetAccInfoRsp.m_iServerID = iServerID;
        STRNCPY( rstGetAccInfoRsp.m_szSdkUserName, szSdkUserName, MAX_NAME_LENGTH);
       
        return ERR_NONE;
    }

    LOGERR_r( "error row effected(%d) when get account(%s, %d)", iRowEffected, szSdkUserName, iServerID );

    return ERR_DB_ERROR;
}


