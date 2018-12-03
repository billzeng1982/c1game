#include "GuildTable.h"
#include "LogMacros.h"
#include "comm_func.h"
#include "strutil.h"
#include "TableDefine.h"
#include "hash_func.h"
#include "../gamedata/GameDataMgr.h"
#include "PKGMETA_metalib.h"

using namespace PKGMETA;

GuildTable::GuildTable()
{
    m_szGlobalBinBuf = NULL;
    m_szMemberBinBuf = NULL;
    m_szApplyBinBuf = NULL;
    m_szReplayBinBuf = NULL;
    m_szBossBinBuf = NULL;
	m_szScienceBinbuf = NULL;
}

GuildTable::~GuildTable()
{
    SAFE_DEL_ARRAY(m_szGlobalBinBuf);
    SAFE_DEL_ARRAY(m_szMemberBinBuf);
    SAFE_DEL_ARRAY(m_szApplyBinBuf);
    SAFE_DEL_ARRAY(m_szReplayBinBuf);
    SAFE_DEL_ARRAY(m_szBossBinBuf);
	SAFE_DEL_ARRAY(m_szScienceBinbuf);
}


bool GuildTable::Init(const char* pszTableName, CMysqlHandler* poMysqlHandler, int iTableNum)
{
    if(!CDataTable::Init(pszTableName, poMysqlHandler, iTableNum))
    {
        return false;
    }

    m_szGlobalBinBuf = new char[GLOBAL_INFO_BINBUF_SIZE];
    if (!m_szGlobalBinBuf )
    {
        return false;
    }

    m_szMemberBinBuf = new char[MEMBER_INFO_BINBUF_SIZE];
    if (!m_szMemberBinBuf )
    {
        return false;
    }

    m_szApplyBinBuf = new char[APPLY_INFO_BINBUF_SIZE];
    if (!m_szApplyBinBuf )
    {
        return false;
    }

    m_szReplayBinBuf = new char[REPLAY_INFO_BINBUF_SIZE];
    if (!m_szReplayBinBuf )
    {
        return false;
    }

    m_szBossBinBuf = new char[BOSS_INFO_BINBUF_SIZE];
    if (!m_szBossBinBuf )
    {
        return false;
    }

	m_szScienceBinbuf = new char[SCIENCE_INFO_BINBUF_SIZE];
	if (!m_szScienceBinbuf )
	{
		return false;
	}

    return true;
}

int GuildTable::CheckExist(uint64_t ullGuildId)
{
    //int iTableNum = GET_TABLE_NUM(ullGuildId);

    m_poMysqlHandler->FormatSql("SELECT GuildId FROM %s WHERE GuildId=%lu and DelFlag=0", GUILD_TABLE_NAME, ullGuildId);
    if (m_poMysqlHandler->Execute() < 0)
    {
        return ERR_DB_ERROR;
    }

    return m_poMysqlHandler->StoreResult();
}

int GuildTable::CheckExist(char* pszName)
{
    assert(pszName);

    //TODO
    //int iTableNum = 1;

    m_poMysqlHandler->FormatSql("SELECT GuildId FROM %s WHERE Name='%s' and DelFlag=0", GUILD_TABLE_NAME, pszName);
    if( m_poMysqlHandler->Execute() < 0 )
    {
        return ERR_DB_ERROR;
    }

    return m_poMysqlHandler->StoreResult();
}

int GuildTable::CreateGuild(DT_GUILD_WHOLE_DATA& rstGuildWholeData)
{
    //检查是否存在
    int iRet = this->CheckExist(rstGuildWholeData.m_stBaseInfo.m_szName);
    if (iRet < 0)
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }
    if (iRet > 0)
    {
        return ERR_ALREADY_EXISTED;
    }

    char* pszSql = m_poMysqlHandler->GetSql();
    pszSql[0] = '\0';

    //int iTableNum = GET_TABLE_NUM(rstGuildWholeData.m_stBaseInfo.m_ullGuildId );
    StrCat(pszSql, DB_SQL_STR_SIZE, "INSERT INTO %s SET  DelFlag=0,  ", GUILD_TABLE_NAME);

    rstGuildWholeData.m_stBaseInfo.m_wVersion = PKGMETA::MetaLib::getVersion();

    size_t ulUseSize = 0;
    DT_GUILD_GLOBAL_INFO stGuildGlobalInfo = {0};
    stGuildGlobalInfo.m_bGuildLevel = 1;

	ResGuildLevelMgr_t& rstResGuildLevelMgr = CGameDataMgr::Instance().GetResGuildLevelMgr();
	RESGUILDLEVEL* pstResGuildLevel = rstResGuildLevelMgr.Find(1);//初识工会等级为1
	assert(pstResGuildLevel);
    stGuildGlobalInfo.m_wMaxMemberNum = pstResGuildLevel->m_dwMemberNum;
	stGuildGlobalInfo.m_wMaxViceLeaderNum = pstResGuildLevel->m_dwViceLeaderNum;
	stGuildGlobalInfo.m_wMaxEliteNum = pstResGuildLevel->m_dwElite;

    iRet = stGuildGlobalInfo.pack((char*)rstGuildWholeData.m_stGlobalBlob.m_szData, MAX_LEN_GUILD_GLOBAL, &ulUseSize);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Guild(%s) Create failed, pack DT_GUILD_GLOBAL_INFO failed, Ret=%d", rstGuildWholeData.m_stBaseInfo.m_szName, iRet);
        return ERR_SYS;
    }
    rstGuildWholeData.m_stGlobalBlob.m_iLen = (int)ulUseSize;

    DT_GUILD_MEMBER_INFO stGuildMemberInfo = {0};
    iRet = stGuildMemberInfo.pack((char*)rstGuildWholeData.m_stMemberBlob.m_szData, MAX_LEN_GUILD_MEMBER, &ulUseSize);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Guild(%s) Create failed, pack DT_GUILD_MEMBER_INFO failed, Ret=%d", rstGuildWholeData.m_stBaseInfo.m_szName, iRet);
        return ERR_SYS;
    }
    rstGuildWholeData.m_stMemberBlob.m_iLen = (int)ulUseSize;

    DT_GUILD_APPLY_INFO stGuildApplyInfo = {0};
    iRet = stGuildApplyInfo.pack((char*)rstGuildWholeData.m_stApplyBlob.m_szData, MAX_LEN_GUILD_APPLY, &ulUseSize);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Guild(%s) Create failed, pack DT_GUILD_APPLY_INFO failed, Ret=%d", rstGuildWholeData.m_stBaseInfo.m_szName, iRet);
        return ERR_SYS;
    }
    rstGuildWholeData.m_stApplyBlob.m_iLen = (int)ulUseSize;

    DT_GUILD_REPLAY_INFO stGuildReplayInfo = {0};
    iRet = stGuildReplayInfo.pack((char*)rstGuildWholeData.m_stReplayBlob.m_szData, MAX_LEN_GUILD_REPLAY, &ulUseSize);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Guild(%s) Create failed, pack DT_GUILD_REPLAY_INFO failed, Ret=%d", rstGuildWholeData.m_stBaseInfo.m_szName, iRet);
        return ERR_SYS;
    }
    rstGuildWholeData.m_stApplyBlob.m_iLen = (int)ulUseSize;

    DT_GUILD_BOSS_INFO stGuildBossInfo = {0};
    iRet = stGuildBossInfo.pack((char*)rstGuildWholeData.m_stBossBlob.m_szData, MAX_LEN_GUILD_BOSS, &ulUseSize);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Guild(%s) Create failed, pack DT_GUILD_BOSS_INFO failed, Ret=%d", rstGuildWholeData.m_stBaseInfo.m_szName, iRet);
        return ERR_SYS;
    }
    rstGuildWholeData.m_stBossBlob.m_iLen = (int)ulUseSize;

	DT_GUILD_SOCIETY_INFO stGuildScienceInfo = {0};
	iRet = stGuildScienceInfo.pack((char*)rstGuildWholeData.m_stSocietyBlob.m_szData, MAX_LEN_GUILD_SOCIETY, &ulUseSize);
	if (iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("Guild(%s) Create failed, pack DT_GUILD_SOCIETY_INFO failed, Ret=%d", rstGuildWholeData.m_stBaseInfo.m_szName, iRet);
		return ERR_SYS;
	}
	rstGuildWholeData.m_stSocietyBlob.m_iLen = (int)ulUseSize;


    this->_ConvertWholeGuildToSql(rstGuildWholeData, pszSql, DB_SQL_STR_SIZE);

    if (m_poMysqlHandler->Execute(pszSql) < 0)
    {
        LOGERR_r("excute sql failed: %s, SqlStr<%s>", m_poMysqlHandler->GetLastErrMsg(), pszSql);
        return ERR_DB_ERROR;
    }

    return ERR_NONE;
}

int GuildTable::DeleteGuild(uint64_t ullGuildId)
{
    //int iTableNum = GET_TABLE_NUM(ullGuildId );
    m_poMysqlHandler->FormatSql("UPDATE %s SET DelFlag=1 WHERE GuildId=%lu and DelFlag=0", GUILD_TABLE_NAME, ullGuildId);
    if (m_poMysqlHandler->Execute() < 0 ||m_poMysqlHandler->AffectedRows() != 1)
    {
        LOGERR_r( "Delete Guild data failed! GuildId <%lu>",ullGuildId);
        return ERR_DB_ERROR;
    }
    return ERR_NONE;
}

int GuildTable::_ConvertWholeGuildToSql(DT_GUILD_WHOLE_DATA& rstGuildWholeData, char* pszSql, int iSqlSize)
{
    assert(pszSql);
    int iRet = 0;

    this->_ConvertBaseInfoToSql(rstGuildWholeData.m_stBaseInfo, pszSql, iSqlSize);
    SQL_ADD_DELIMITER(pszSql, iSqlSize);

    iRet = this->_ConvertBlobInfoToSql( (char*)rstGuildWholeData.m_stGlobalBlob.m_szData, rstGuildWholeData.m_stGlobalBlob.m_iLen, GUILD_DATA_TYPE_GLOBAL, pszSql, iSqlSize );
    if( iRet < 0 )
    {
        return iRet;
    }
    SQL_ADD_DELIMITER(pszSql, iSqlSize);

    iRet = this->_ConvertBlobInfoToSql( (char*)rstGuildWholeData.m_stMemberBlob.m_szData, rstGuildWholeData.m_stMemberBlob.m_iLen, GUILD_DATA_TYPE_MEMBER, pszSql, iSqlSize );
    if( iRet < 0 )
    {
        return iRet;
    }
    SQL_ADD_DELIMITER(pszSql, iSqlSize);

    iRet = this->_ConvertBlobInfoToSql( (char*)rstGuildWholeData.m_stApplyBlob.m_szData, rstGuildWholeData.m_stApplyBlob.m_iLen, GUILD_DATA_TYPE_APPLY, pszSql, iSqlSize );
    if( iRet < 0 )
    {
        return iRet;
    }
    SQL_ADD_DELIMITER(pszSql, iSqlSize);

    iRet = this->_ConvertBlobInfoToSql( (char*)rstGuildWholeData.m_stReplayBlob.m_szData, rstGuildWholeData.m_stReplayBlob.m_iLen, GUILD_DATA_TYPE_REPLAY, pszSql, iSqlSize );
    if( iRet < 0 )
    {
        return iRet;
    }
    SQL_ADD_DELIMITER(pszSql, iSqlSize);

    iRet = this->_ConvertBlobInfoToSql( (char*)rstGuildWholeData.m_stBossBlob.m_szData, rstGuildWholeData.m_stBossBlob.m_iLen, GUILD_DATA_TYPE_BOSS, pszSql, iSqlSize );
    if( iRet < 0 )
    {
        return iRet;
    }
	SQL_ADD_DELIMITER(pszSql, iSqlSize);

	iRet = this->_ConvertBlobInfoToSql( (char*)rstGuildWholeData.m_stSocietyBlob.m_szData, rstGuildWholeData.m_stSocietyBlob.m_iLen, GUILD_DATA_TYPE_SOCIETY, pszSql, iSqlSize );
	if( iRet < 0 )
	{
		return iRet;
	}

    return ERR_NONE;
}

void GuildTable::_ConvertBaseInfoToSql(DT_GUILD_BASE_INFO& rstBaseInfo, char* pszSql, int iSqlSize)
{
    assert(pszSql);
    StrCat(pszSql, iSqlSize, "GuildId=%lu, Name='%s', Version=%u ",
           rstBaseInfo.m_ullGuildId,
           rstBaseInfo.m_szName,
           rstBaseInfo.m_wVersion);
    return;
}

int GuildTable::_ConvertBlobInfoToSql(char* pszData, int iLen, int iType, char* pszSql, int iSqlSize)
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
        case GUILD_DATA_TYPE_GLOBAL:
            pszBinBuf = m_szGlobalBinBuf;
            iBufSize = GLOBAL_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_GUILD_GLOBAL;
            break;
        case GUILD_DATA_TYPE_MEMBER:
            pszBinBuf = m_szMemberBinBuf;
            iBufSize = MEMBER_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_GUILD_MEMBER;
            break;
        case GUILD_DATA_TYPE_APPLY:
            pszBinBuf = m_szApplyBinBuf;
            iBufSize = APPLY_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_GUILD_APPLY;
            break;
        case GUILD_DATA_TYPE_REPLAY:
            pszBinBuf = m_szReplayBinBuf;
            iBufSize = REPLAY_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_GUILD_REPLAY;
            break;
        case GUILD_DATA_TYPE_BOSS:
            pszBinBuf = m_szBossBinBuf;
            iBufSize = BOSS_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_GUILD_BOSS;
            break;
		case GUILD_DATA_TYPE_SOCIETY:
			pszBinBuf = m_szScienceBinbuf;
			iBufSize = SCIENCE_INFO_BINBUF_SIZE;
			pszBlobName = BLOB_NAME_GUILD_SCIENCE;
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

int GuildTable::UpdateGuildWholeData(DT_GUILD_WHOLE_DATA& rstGuildWholeData)
{
    //int iTableNum = GET_TABLE_NUM(rstGuildWholeData.m_stBaseInfo.m_ullGuildId);

    int iRet = 0;
    char* pszSql = m_poMysqlHandler->GetSql();
    pszSql[0] = '\0';

    StrCat( pszSql, DB_SQL_STR_SIZE, "UPDATE %s SET ", GUILD_TABLE_NAME);
    iRet = this->_ConvertWholeGuildToSql(rstGuildWholeData, pszSql, DB_SQL_STR_SIZE);
    if( iRet < 0 )
    {
        return iRet;
    }
    StrCat(pszSql, DB_SQL_STR_SIZE, " WHERE GuildId=%lu", rstGuildWholeData.m_stBaseInfo.m_ullGuildId);

    if( m_poMysqlHandler->Execute( pszSql ) < 0 ||m_poMysqlHandler->AffectedRows() != 1 )
    {
        LOGERR_r("excute sql failed: %s, SqlStr<%s>", m_poMysqlHandler->GetLastErrMsg(), pszSql);
        return ERR_DB_ERROR;
    }

    return ERR_NONE;
}

int GuildTable::GetGuildWholeData(uint64_t ullGuildId,  DT_GUILD_WHOLE_DATA& rstGuildWholeData)
{
    //int iTableNum = GET_TABLE_NUM(ullGuildId);
    m_poMysqlHandler->FormatSql( "SELECT Name, Version, "
                                 "%s, "
                                 "%s, "
                                 "%s, "
                                 "%s, "
								 "%s, "
                                 "%s "
                                 /*注意 逗号 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! "%s," */
                                 "FROM %s WHERE GuildId=%lu and DelFlag=0",
                                 BLOB_NAME_GUILD_GLOBAL, /*1*/
                                 BLOB_NAME_GUILD_MEMBER,/*2*/
                                 BLOB_NAME_GUILD_APPLY,/*3*/
                                 BLOB_NAME_GUILD_REPLAY,/*4*/
                                 BLOB_NAME_GUILD_BOSS,/*5*/
								 BLOB_NAME_GUILD_SCIENCE,/*6*/
                                 GUILD_TABLE_NAME,
                                 ullGuildId);
    if (m_poMysqlHandler->Execute( ) < 0 )
    {
        LOGERR_r("excute sql failed: %s, GuildId <%lu>",
                    m_poMysqlHandler->GetLastErrMsg(), ullGuildId);
        return ERR_DB_ERROR;
    }

    int iEffectRow = m_poMysqlHandler->StoreResult();
    if (iEffectRow < 0 )
    {
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

    DT_GUILD_BASE_INFO& rstBaseInfo = rstGuildWholeData.m_stBaseInfo;
    rstBaseInfo.m_ullGuildId = ullGuildId;
    StrCpy( rstBaseInfo.m_szName, oRowIter.GetField(i++)->GetString(), MAX_NAME_LENGTH);
    rstBaseInfo.m_wVersion = (uint16_t)oRowIter.GetField(i++)->GetInteger();
    rstGuildWholeData.m_stGlobalBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstGuildWholeData.m_stGlobalBlob.m_szData, MAX_LEN_GUILD_GLOBAL);
    rstGuildWholeData.m_stMemberBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstGuildWholeData.m_stMemberBlob.m_szData, MAX_LEN_GUILD_MEMBER);
	rstGuildWholeData.m_stApplyBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstGuildWholeData.m_stApplyBlob.m_szData, MAX_LEN_GUILD_REPLAY);
    rstGuildWholeData.m_stReplayBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstGuildWholeData.m_stReplayBlob.m_szData, MAX_LEN_GUILD_REPLAY);
    rstGuildWholeData.m_stBossBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstGuildWholeData.m_stBossBlob.m_szData, MAX_LEN_GUILD_BOSS);
	rstGuildWholeData.m_stSocietyBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstGuildWholeData.m_stSocietyBlob.m_szData, MAX_LEN_GUILD_SOCIETY);

	return ERR_NONE;
}

int GuildTable::GetGuildWholeData(char* pszName, DT_GUILD_WHOLE_DATA& rstGuildWholeData)
{
    m_poMysqlHandler->FormatSql( "SELECT GuildId, Version, "
                                 "%s, "
                                 "%s, "
                                 "%s, "
                                 "%s, "
								 "%s, "
                                 "%s "
                                 /*注意 逗号 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! "%s," */
                                 "FROM %s WHERE Name='%s' and DelFlag=0",
                                 BLOB_NAME_GUILD_GLOBAL, /*1*/
                                 BLOB_NAME_GUILD_MEMBER,/*2*/
                                 BLOB_NAME_GUILD_APPLY,/*3*/
                                 BLOB_NAME_GUILD_REPLAY,/*4*/
                                 BLOB_NAME_GUILD_BOSS,/*5*/
								 BLOB_NAME_GUILD_SCIENCE,/*6*/
                                 GUILD_TABLE_NAME,
                                 pszName);

    if (m_poMysqlHandler->Execute() < 0)
    {
        LOGERR_r("excute sql(%s) failed: %s, GuildName <%s>", m_poMysqlHandler->GetSql(), m_poMysqlHandler->GetLastErrMsg(), pszName);
        return ERR_DB_ERROR;
    }

    int iEffectRow = m_poMysqlHandler->StoreResult();
    if (iEffectRow < 0 )
    {
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

    DT_GUILD_BASE_INFO& rstBaseInfo = rstGuildWholeData.m_stBaseInfo;
    rstBaseInfo.m_ullGuildId = (uint64_t)oRowIter.GetField(i++)->GetInteger();
    rstBaseInfo.m_wVersion = (uint16_t)oRowIter.GetField(i++)->GetInteger();
    rstGuildWholeData.m_stGlobalBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstGuildWholeData.m_stGlobalBlob.m_szData, MAX_LEN_GUILD_GLOBAL);
    rstGuildWholeData.m_stMemberBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstGuildWholeData.m_stMemberBlob.m_szData, MAX_LEN_GUILD_MEMBER);
	rstGuildWholeData.m_stApplyBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstGuildWholeData.m_stApplyBlob.m_szData, MAX_LEN_GUILD_REPLAY);
    rstGuildWholeData.m_stReplayBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstGuildWholeData.m_stReplayBlob.m_szData, MAX_LEN_GUILD_REPLAY);
    rstGuildWholeData.m_stBossBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstGuildWholeData.m_stBossBlob.m_szData, MAX_LEN_GUILD_BOSS);
	rstGuildWholeData.m_stSocietyBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstGuildWholeData.m_stSocietyBlob.m_szData, MAX_LEN_GUILD_SOCIETY);

	return ERR_NONE;
}

