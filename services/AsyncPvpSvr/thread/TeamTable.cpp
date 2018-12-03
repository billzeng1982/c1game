#include "TeamTable.h"
#include "LogMacros.h"
#include "comm_func.h"
#include "strutil.h"
#include "TableDefine.h"
#include "common_proto.h"
#include "PKGMETA_metalib.h"

using namespace PKGMETA;

TeamTable::TeamTable()
{
    m_szDataBinBuf = NULL;
}


TeamTable::~TeamTable()
{
    SAFE_DEL_ARRAY(m_szDataBinBuf);
}


bool TeamTable::CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, ASYNCPVPSVRCFG* pstConfig)
{
    if (NULL == pstConfig)
    {
        return false;
    }
    m_pstConfig = pstConfig;
    if(!CDataTable::Init(pszTableName, poMysqlHandler))
    {
        return false;
    }

    m_szDataBinBuf = new char[DATA_INFO_BINBUF_SIZE];
    if (!m_szDataBinBuf )
    {
        return false;
    }
    return true;
}


int TeamTable::CheckExist(uint64_t ullUin)
{
    //int iTableNum = GET_TABLE_NUM(ullUin);
    m_poMysqlHandler->FormatSql("SELECT Uin FROM %s WHERE Uin=%lu", ASYNCPVP_TEAM_TABLE_NAME, ullUin);
    if (m_poMysqlHandler->Execute() < 0)
    {
        LOGERR_r("excute sql(%s) failed, err(%s)", m_poMysqlHandler->GetSql(), m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }
    return m_poMysqlHandler->StoreResult();
}


int TeamTable::CreateTeam(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData)
{
    //检查是否存在
    int iRet = this->CheckExist(rstTeamData.m_ullUin);
    if (iRet < 0)
    {
        LOGERR_r("excute sql(%s) failed, err(%s)", m_poMysqlHandler->GetSql(), m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }
    if (iRet > 0)
    {
        LOGRUN_r("Player(%lu) Create team already exsit, so update", rstTeamData.m_ullUin);
        return this->UpdateTeam(rstTeamData);
    }

    //int iTableNum = GET_TABLE_NUM(rstTeamData.m_ullUin);

    char* pszSql = m_poMysqlHandler->GetSql();
    pszSql[0] = '\0';
    StrCat(pszSql, DB_SQL_STR_SIZE, "INSERT INTO %s SET  ", ASYNCPVP_TEAM_TABLE_NAME);

    iRet = this->_ConvertTeamToSql(rstTeamData, pszSql, DB_SQL_STR_SIZE );
    if (iRet < 0)
    {
        LOGERR_r( "Player(%lu) _ConvertTeamToSql failed, iRet=%d", rstTeamData.m_ullUin, iRet);
        return iRet;
    }

    if (m_poMysqlHandler->Execute(pszSql) < 0)
    {
        LOGERR_r("excute sql(%s) failed, err(%s)", m_poMysqlHandler->GetSql(), m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }

    return ERR_NONE;
}


int TeamTable::DeleteTeam(uint64_t ullUin)
{
    return ERR_NONE;
}


int TeamTable::_ConvertTeamToSql(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData, char* pszSql, int iSqlSize)
{
    assert(pszSql);
    int iRet = 0;

    StrCat(pszSql, iSqlSize, "Uin=%lu, Version=%d, ", rstTeamData.m_ullUin, MetaLib::getVersion());

    size_t ulUseSize = 0;
    DT_ASYNC_PVP_PLAYER_DATA_BLOB stBlob = {0};
    iRet = rstTeamData.pack((char*)stBlob.m_szData, MAX_LEN_ASYNC_PVP_PLAYER_DATA_BLOB, &ulUseSize);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Player(%lu) pack TEAM_DATA failed, Ret=%d", rstTeamData.m_ullUin, iRet);
        return ERR_SYS;
    }

    if (!m_poMysqlHandler->BinToStr((char*)stBlob.m_szData, (int)ulUseSize, m_szDataBinBuf, DATA_INFO_BINBUF_SIZE))
    {
        LOGERR_r("Player(%lu) convert to mysql bin failed", rstTeamData.m_ullUin);
        return ERR_SYS;
    }

    StrCat(pszSql, iSqlSize, "`%s`='%s'", BLOB_NAME_ASYNCPVP_TEAM_DATA, m_szDataBinBuf);

    return ERR_NONE;
}


int TeamTable::UpdateTeam(DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData)
{
    //int iTableNum = GET_TABLE_NUM(rstTeamData.m_ullUin);

    int iRet = 0;
    char* pszSql = m_poMysqlHandler->GetSql();
    pszSql[0] = '\0';

    StrCat(pszSql, DB_SQL_STR_SIZE, "UPDATE %s SET ", ASYNCPVP_TEAM_TABLE_NAME);
    iRet = this->_ConvertTeamToSql(rstTeamData, pszSql, DB_SQL_STR_SIZE);
    if( iRet < 0 )
    {
        LOGERR_r( "Player(%lu) _ConvertTeamToSql failed, iRet=%d", rstTeamData.m_ullUin, iRet);
        return iRet;
    }

    StrCat(pszSql, DB_SQL_STR_SIZE, " WHERE Uin=%lu", rstTeamData.m_ullUin);
    if( m_poMysqlHandler->Execute( pszSql ) < 0 ||m_poMysqlHandler->AffectedRows() != 1 )
    {
        LOGERR_r("Save Player(%lu) Teamdata failed, sql(%s), err(%s)", rstTeamData.m_ullUin,
                    m_poMysqlHandler->GetSql(), m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }

    return ERR_NONE;
}

int TeamTable::GetTeam(uint64_t ullUin,  DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData)
{
    //int iTableNum = GET_TABLE_NUM(ullUin);
    m_poMysqlHandler->FormatSql("SELECT Uin, Version, "
                                 "%s "
                                 /*注意 逗号 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! "%s," */
                                 "FROM %s WHERE Uin=%lu",
                                 BLOB_NAME_ASYNCPVP_TEAM_DATA,/*1*/
                                 ASYNCPVP_TEAM_TABLE_NAME,
                                 ullUin);
    if (m_poMysqlHandler->Execute() < 0)
    {
        LOGERR_r("excute sql(%s) failed, err(%s)", m_poMysqlHandler->GetSql(), m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }

    int iEffectRow = m_poMysqlHandler->StoreResult();
    if (iEffectRow < 0)
    {
        LOGERR_r("excute sql(%s) failed, err(%s)", m_poMysqlHandler->GetSql(), m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }
    if (0 == iEffectRow)
    {
        return ERR_NOT_FOUND;
    }

    assert(1 == iEffectRow);

    CMysqlRowIter& oRowIter = m_poMysqlHandler->GetRowIter();
    int i = 0;
    oRowIter.Begin(m_poMysqlHandler->GetResult());

    uint64_t ullPlayerId;
    uint32_t dwVersion;
    DT_ASYNC_PVP_PLAYER_DATA_BLOB stBlob;
    ullPlayerId = oRowIter.GetField(i++)->GetInteger();
    dwVersion = oRowIter.GetField(i++)->GetInteger();
    stBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)stBlob.m_szData, MAX_LEN_ASYNC_PVP_PLAYER_DATA_BLOB);

    size_t ulUseSize = 0;
    int iRet = rstTeamData.unpack((char*)stBlob.m_szData, sizeof(stBlob.m_szData), &ulUseSize, dwVersion);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Player(%lu) Team unpack failed, Ret=%d, BlobLen=%d, UsedSize=%lu", ullPlayerId, iRet, stBlob.m_iLen, ulUseSize);
        return iRet;
    }

    return ERR_NONE;
}




