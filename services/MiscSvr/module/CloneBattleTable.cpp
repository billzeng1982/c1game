#include "CloneBattleTable.h"
#include "LogMacros.h"
#include "comm_func.h"
#include "strutil.h"
#include "TableDefine.h"
#include "../MiscSvr.h"
#include "PKGMETA_metalib.h"

#define BLOB_TYPE_TEAM                  1
#define TABLE_NAME_CLOEBATTLE           "tbl_clonebattle_team"
#define BLOB_NAME_CLONEBATTLE_TEAM 		"TeamBlob"

using namespace PKGMETA;



CloneBattleTeamTable::CloneBattleTeamTable()
{
    m_szTeamBinBuf = NULL;
}

CloneBattleTeamTable::~CloneBattleTeamTable()
{
    SAFE_DEL_ARRAY(m_szTeamBinBuf);
}

bool CloneBattleTeamTable::CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, MISCSVRCFG* pstConfig)
{
    if (NULL == pstConfig)
    {
        return false;
    }
    StrCpy(m_szTableName, pszTableName, sizeof(m_szTableName));
    m_pstConfig = pstConfig;
    m_stMysqlHandler.ConnectDB(m_pstConfig->m_szDBAddr, m_pstConfig->m_wPort, m_pstConfig->m_szDBName,
        m_pstConfig->m_szUser, m_pstConfig->m_szPassword);

    m_szTeamBinBuf = new char[CONST_CLONEBATTLE_TEAM_INFO_BINBUF_SIZE];
    if( !m_szTeamBinBuf) return false;

    return true;
}

int CloneBattleTeamTable::_ConvertWholeDataToSql( IN PKGMETA::DT_CLONE_BATTLE_TEAM_DATA& rstData, INOUT char* pszSql, int iSqlSize )
{
    assert( pszSql );
    int iRet = 0;

    StrCat(pszSql, iSqlSize, " Id=%lu, Version=%u ", 
        rstData.m_stBaseInfo.m_ullId, PKGMETA::MetaLib::getVersion());
    SQL_ADD_DELIMITER( pszSql, iSqlSize );
    iRet = this->_ConvertBlobInfoToSql((char*)rstData.m_stTeamBlob.m_szData, rstData.m_stTeamBlob.m_iLen, BLOB_TYPE_TEAM, pszSql, iSqlSize);
    if (iRet < 0)
        return iRet;

    return ERR_NONE;
}


int CloneBattleTeamTable::_ConvertBlobInfoToSql( char* pszData, int iLen, int iType, INOUT char* pszSql, int iSqlSize )
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
        case BLOB_TYPE_TEAM:
            pszBinBuf = m_szTeamBinBuf;
            iBufSize = CONST_CLONEBATTLE_TEAM_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_CLONEBATTLE_TEAM;
            break;
        default:
            LOGERR_r("type error <%d>", iType);
            return ERR_SYS;
    }

    if (!m_stMysqlHandler.BinToStr(pszData, iLen, pszBinBuf, iBufSize))
    {
        LOGERR_r( "convert to mysql bin failed, type <%d>", iType );
        return ERR_SYS;
    }

    StrCat(pszSql, iSqlSize, "`%s`='%s'", pszBlobName, pszBinBuf);
    return ERR_NONE;
}



int CloneBattleTeamTable::GetData(PKGMETA::DT_CLONE_BATTLE_TEAM_DATA & rstTeamData)
{
    //int iTableNum = GET_TABLE_NUM(rstTeamData.m_stBaseInfo.m_ullId);
    m_stMysqlHandler.FormatSql(
        "SELECT  Version, %s FROM %s WHERE Id=%lu ", BLOB_NAME_CLONEBATTLE_TEAM, m_szTableName,
        rstTeamData.m_stBaseInfo.m_ullId);
    if (m_stMysqlHandler.Execute() < 0)
    {
        LOGERR("excute sql failed: %s", m_stMysqlHandler.GetLastErrMsg());
        return ERR_DB_ERROR;
    }

    int iEffectRow = m_stMysqlHandler.StoreResult();
    if (iEffectRow < 0)
    {
        return ERR_DB_ERROR;
    }
    if (0 == iEffectRow)
    {
        return ERR_NOT_FOUND;
    }
    int i = 0;
    CMysqlRowIter& oRowIter = m_stMysqlHandler.GetRowIter();
    oRowIter.Begin(m_stMysqlHandler.GetResult());
    rstTeamData.m_stBaseInfo.m_dwVersion = (uint32_t)oRowIter.GetField(i++)->GetInteger();
    rstTeamData.m_stTeamBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstTeamData.m_stTeamBlob.m_szData, MAX_LEN_CLONE_BATTLE_TEAM_BLOB);
    return ERR_NONE;
}


int CloneBattleTeamTable::UptData(PKGMETA::DT_CLONE_BATTLE_TEAM_DATA& rstTeamData)
{

    char* pszSql = m_stMysqlHandler.GetSql();
    pszSql[0] = '\0';
    //int iTableNum = GET_TABLE_NUM(rstTeamData.m_stBaseInfo.m_ullId);
    // base info
    StrCat(pszSql, DB_SQL_STR_SIZE, " REPLACE INTO %s SET ", m_szTableName);

    int iRet = this->_ConvertWholeDataToSql(rstTeamData, pszSql, DB_SQL_STR_SIZE);
    if (iRet < 0) return iRet;

    if (m_stMysqlHandler.Execute(pszSql) < 0)
    {
        LOGERR_r("excute sql failed: %s", m_stMysqlHandler.GetLastErrMsg());
        return ERR_DB_ERROR;
    }
    return ERR_NONE;
}

int CloneBattleTeamTable::DelData(uint64_t ullTeamIdList[], size_t uNum)
{
    //int iTableNum = GET_TABLE_NUM(ullTeamIdList[0]);
    char* pszSql = m_stMysqlHandler.GetSql();
    pszSql[0] = '\0';
    StrCat(pszSql, DB_SQL_STR_SIZE,  "DELETE FROM  %s  WHERE Id in ( ", m_szTableName);
    
    for (size_t i = 0; i < uNum - 1; i++)
    {
        StrCat(pszSql, DB_SQL_STR_SIZE, "%lu, ", ullTeamIdList[i]);
    }
    StrCat(pszSql, DB_SQL_STR_SIZE, "%lu) ", ullTeamIdList[uNum - 1]);

    if (m_stMysqlHandler.Execute(pszSql) < 0 ||
        m_stMysqlHandler.AffectedRows() != (int)uNum)
    {
        LOGERR_r("Del CloneBattleTeam Num<%u> error %s", (int)uNum, m_stMysqlHandler.GetLastErrMsg());
        return ERR_DB_ERROR;
    }

    return ERR_NONE;

}






