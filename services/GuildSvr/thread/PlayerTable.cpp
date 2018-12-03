#include "PlayerTable.h"
#include "LogMacros.h"
#include "comm_func.h"
#include "strutil.h"
#include "TableDefine.h"
#include "PKGMETA_metalib.h"

using namespace PKGMETA;

PlayerTable::PlayerTable()
{
    m_szApplyBinBuf = NULL;
}


PlayerTable::~PlayerTable()
{
    SAFE_DEL_ARRAY(m_szApplyBinBuf);
}


bool PlayerTable::CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, GUILDSVRCFG* pstConfig)
{
    if (NULL == pstConfig)
    {
        return false;
    }
    m_pstConfig = pstConfig;
    if(!CDataTable::Init(pszTableName, poMysqlHandler, pstConfig->m_iGuildTableNum))
    {
        return false;
    }

    m_szApplyBinBuf = new char[APPLY_INFO_BINBUF_SIZE];
    if (!m_szApplyBinBuf )
    {
        return false;
    }
    return true;
}


int PlayerTable::CheckExist(uint64_t ullUin)
{
    //int iTableNum = GET_TABLE_NUM(ullUin);
    m_poMysqlHandler->FormatSql("SELECT Uin FROM %s WHERE Uin=%lu", GUILD_PLAYER_TABLE_NAME,  ullUin);
    if (m_poMysqlHandler->Execute() < 0)
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }
    return m_poMysqlHandler->StoreResult();
}


int PlayerTable::CreatePlayer(DT_GUILD_PLAYER_DATA& rstPlayerData)
{
    //检查是否存在
    int iRet = this->CheckExist(rstPlayerData.m_stBaseInfo.m_ullUin);
    if (iRet < 0)
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }
    if (iRet > 0)
    {
        LOGERR_r( "Create Player failed! GuildId(%lu) already exsit",rstPlayerData.m_stBaseInfo.m_ullUin);
        return ERR_ALREADY_EXISTED;
    }

    //int iTableNum = GET_TABLE_NUM(rstPlayerData.m_stBaseInfo.m_ullUin) ;

    char* pszSql = m_poMysqlHandler->GetSql();
    pszSql[0] = '\0';
    StrCat(pszSql, DB_SQL_STR_SIZE, "INSERT INTO %s SET  ", GUILD_PLAYER_TABLE_NAME);

    rstPlayerData.m_stBaseInfo.m_wVersion = PKGMETA::MetaLib::getVersion();

    size_t ulUseSize = 0;
    DT_GUILD_PLAYER_APPLY_BLOB stGuildPlayerApplyInfo = {0};
    iRet = stGuildPlayerApplyInfo.pack((char*)rstPlayerData.m_stApplyBlob.m_szData, MAX_LEN_GUILD_PLAYER_APPLY, &ulUseSize);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Player(%lu) Create failed, pack DT_GUILD_PLAYER_APPLY_INFO failed, Ret=%d", rstPlayerData.m_stBaseInfo.m_ullUin, iRet);
        return ERR_SYS;
    }
    rstPlayerData.m_stApplyBlob.m_iLen = (int)ulUseSize;


    iRet = this->_ConvertPlayerToSql(rstPlayerData, pszSql, DB_SQL_STR_SIZE );
    if (iRet < 0)
    {
        LOGERR_r( "_ConvertPlayerToSql failed, iRet=%d", iRet);
        return iRet;
    }

    if (m_poMysqlHandler->Execute(pszSql) < 0)
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }

    return ERR_NONE;
}


int PlayerTable::DeletePlayer(uint64_t ullUin)
{
    //int iTableNum = ullUin % m_iTableNum + 1;
    return ERR_NONE;
}


int PlayerTable::_ConvertPlayerToSql(DT_GUILD_PLAYER_DATA& rstPlayerData, char* pszSql, int iSqlSize)
{
    assert(pszSql);
    int iRet = 0;

    this->_ConvertBaseInfoToSql(rstPlayerData.m_stBaseInfo, pszSql, iSqlSize);
    SQL_ADD_DELIMITER(pszSql, iSqlSize);

    iRet = this->_ConvertBlobInfoToSql( (char*)rstPlayerData.m_stApplyBlob.m_szData, rstPlayerData.m_stApplyBlob.m_iLen, GUILD_DATA_TYPE_APPLY, pszSql, iSqlSize );
    if( iRet < 0 )
    {
        LOGERR_r( "_ConvertBlobInfoToSql failed, iRet=%d", iRet);
        return iRet;
    }
    return ERR_NONE;
}


void PlayerTable::_ConvertBaseInfoToSql(DT_GUILD_PLAYER_BASE_INFO& rstBaseInfo, char* pszSql, int iSqlSize)
{
    assert(pszSql);
    StrCat(pszSql, iSqlSize, "Uin=%lu, GuildId=%lu, Version=%u",
           rstBaseInfo.m_ullUin,
           rstBaseInfo.m_ullGuildId,
           rstBaseInfo.m_wVersion);
    return;
}


int PlayerTable::_ConvertBlobInfoToSql(char* pszData, int iLen, int iType, char* pszSql, int iSqlSize)
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
        case GUILD_DATA_TYPE_APPLY:
            pszBinBuf = m_szApplyBinBuf;
            iBufSize = APPLY_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_GUILD_PLAYER_APPLY;
            break;
        default:
            return ERR_SYS;
    }

    if (!m_poMysqlHandler->BinToStr(pszData, iLen, pszBinBuf, iBufSize))
    {
        LOGERR_r("convert to mysql bin failed, type <%d>", iType);
        return ERR_SYS;
    }

    StrCat(pszSql, iSqlSize, "`%s`='%s'", pszBlobName, pszBinBuf);
    return ERR_NONE;
}


int PlayerTable::UpdatePlayerData(DT_GUILD_PLAYER_DATA& rstPlayerData)
{
    //int iTableNum = GET_TABLE_NUM(rstPlayerData.m_stBaseInfo.m_ullUin);

    int iRet = 0;
    char* pszSql = m_poMysqlHandler->GetSql();
    pszSql[0] = '\0';

    StrCat(pszSql, DB_SQL_STR_SIZE, "UPDATE %s SET ", GUILD_PLAYER_TABLE_NAME);
    iRet = this->_ConvertPlayerToSql(rstPlayerData, pszSql, DB_SQL_STR_SIZE);
    if( iRet < 0 )
    {
        LOGERR_r( "_ConvertPlayerToSql failed, iRet=%d", iRet);
        return iRet;
    }

    StrCat(pszSql, DB_SQL_STR_SIZE, "WHERE Uin=%lu", rstPlayerData.m_stBaseInfo.m_ullUin);
    if( m_poMysqlHandler->Execute( pszSql ) < 0 ||m_poMysqlHandler->AffectedRows() != 1 )
    {
        LOGERR_r("Save Player data failed, Uin(%lu), GuildId(%lu), ErrMsg(%s)",
                   rstPlayerData.m_stBaseInfo.m_ullUin, rstPlayerData.m_stBaseInfo.m_ullGuildId, m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }

    return ERR_NONE;
}


int PlayerTable::GetPlayerData(uint64_t ullUin,  DT_GUILD_PLAYER_DATA& rstPlayerData)
{
    //int iTableNum = GET_TABLE_NUM(ullUin);
    m_poMysqlHandler->FormatSql("SELECT GuildId, Version, "
                                 "%s "
                                 /*注意 逗号 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! "%s," */
                                 "FROM %s WHERE Uin=%lu",
                                 BLOB_NAME_GUILD_PLAYER_APPLY,/*1*/
                                 GUILD_PLAYER_TABLE_NAME,
                                 ullUin);
    if (m_poMysqlHandler->Execute( ) < 0)
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }

    int iEffectRow = m_poMysqlHandler->StoreResult();
    if (iEffectRow < 0)
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg());
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

    DT_GUILD_PLAYER_BASE_INFO& rstBaseInfo = rstPlayerData.m_stBaseInfo;
    rstBaseInfo.m_ullUin = ullUin;
    rstBaseInfo.m_ullGuildId = oRowIter.GetField(i++)->GetInteger();
    rstBaseInfo.m_wVersion = (uint16_t)oRowIter.GetField(i++)->GetInteger();
    rstPlayerData.m_stApplyBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstPlayerData.m_stApplyBlob.m_szData, MAX_LEN_GUILD_PLAYER_APPLY);

    return ERR_NONE;
}




