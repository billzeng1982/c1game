#include "RoleTable.h"
#include "LogMacros.h"
#include "comm_func.h"
#include "strutil.h"
#include "TableDefine.h"
#include "../RoleSvr.h"

using namespace PKGMETA;

RoleTable::RoleTable()
{
    m_szMajestyBinBuf = NULL;
    m_szEquipBinBuf = NULL;
    m_szGCardBinBuf = NULL;
    m_szPropsBinBuf = NULL;
    m_szELOBinBuf = NULL;
    m_szTaskBinBuf = NULL;
    m_szMSkillBinBuf = NULL;
    m_szEquipPageBinBuf = NULL;
    m_szPveBinBuf = NULL;
    m_szMiscBinBuf = NULL;
    m_szGuildBinBuf = NULL;
    m_szTacticsBinBuf = NULL;
}

RoleTable::~RoleTable()
{
    SAFE_DEL_ARRAY(m_szMajestyBinBuf);
    SAFE_DEL_ARRAY(m_szEquipBinBuf);
    SAFE_DEL_ARRAY(m_szGCardBinBuf);
    SAFE_DEL_ARRAY(m_szPropsBinBuf);
    SAFE_DEL_ARRAY(m_szELOBinBuf);
    SAFE_DEL_ARRAY(m_szTaskBinBuf);
    SAFE_DEL_ARRAY(m_szMSkillBinBuf);
    SAFE_DEL_ARRAY(m_szEquipPageBinBuf);
    SAFE_DEL_ARRAY(m_szPveBinBuf);
    SAFE_DEL_ARRAY(m_szMiscBinBuf);
    SAFE_DEL_ARRAY(m_szGuildBinBuf);
    SAFE_DEL_ARRAY(m_szTacticsBinBuf);
}

bool RoleTable::CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, ROLESVRCFG* pstConfig)
{
    if (NULL == pstConfig)
    {
        return false;
    }
    m_pstConfig = pstConfig;
    CDataTable::Init( pszTableName, poMysqlHandler, m_pstConfig->m_iRoleTableNum );

    m_szMajestyBinBuf = new char[MAJESTY_INFO_BINBUF_SIZE];
    if( !m_szMajestyBinBuf ) return false;

    m_szEquipBinBuf = new char[EQUIP_INFO_BINBUF_SIZE];
    if( !m_szEquipBinBuf ) return false;

    m_szGCardBinBuf = new char[GCARD_INFO_BINBUF_SIZE];
    if( !m_szGCardBinBuf ) return false;

    m_szMSkillBinBuf = new char[MSKILL_INFO_BINBUF_SIZE];
    if( !m_szMSkillBinBuf ) return false;

    m_szPropsBinBuf = new char[PROPS_INFO_BINBUF_SIZE];
    if( !m_szPropsBinBuf ) return false;

	m_szItemsBinBuf = new char[ITEMS_INFO_BINBUF_SIZE];
	if( !m_szItemsBinBuf ) return false;

    m_szELOBinBuf = new char[ELO_INFO_BINBUF_SIZE];
    if( !m_szELOBinBuf ) return false;

    m_szTaskBinBuf = new char[TASK_INFO_BINBUF_SIZE];
    if( !m_szTaskBinBuf ) return false;

    m_szPveBinBuf = new char[PVE_INFO_BINBUF_SIZE];
    if( !m_szPveBinBuf ) return false;

    m_szMiscBinBuf = new char[MISC_INFO_BINBUF_SIZE];
    if( !m_szMiscBinBuf ) return false;

    m_szGuildBinBuf = new char[GUILD_INFO_BINBUF_SIZE];
    if( !m_szGuildBinBuf ) return false;

    m_szTacticsBinBuf = new char[TACTICS_INFO_BINBUF_SIZE];
    if( !m_szTacticsBinBuf ) return false;

    return true;
}

int RoleTable::CreateRole( IN PKGMETA::DT_ROLE_WHOLE_DATA& rstRoleWholeData )
{
    //int iTableNum = GET_TABLE_NUM(rstRoleWholeData.m_stBaseInfo.m_ullUin);


    // check is exist
    int iRet = this->CheckExist( rstRoleWholeData.m_stBaseInfo.m_ullUin );
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
    StrCat( pszSql, DB_SQL_STR_SIZE, "INSERT INTO %s SET ", ROLE_TABLE_NAME );

    iRet = this->_ConvertWholeRoleToSql( rstRoleWholeData, pszSql, DB_SQL_STR_SIZE );
    if( iRet < 0 ) return iRet;

    if( m_poMysqlHandler->Execute( pszSql ) < 0 )
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg() );
        return ERR_DB_ERROR;
    }

    return ERR_NONE;
}

int RoleTable::SaveRoleWholeData( IN PKGMETA::DT_ROLE_WHOLE_DATA& rstRoleWholeData )
{
    //int iTableNum = GET_TABLE_NUM(rstRoleWholeData.m_stBaseInfo.m_ullUin);


    int iRet = 0;
    char* pszSql = m_poMysqlHandler->GetSql();
    pszSql[0] = '\0';

    // base info
    StrCat( pszSql, DB_SQL_STR_SIZE, "UPDATE %s SET ", ROLE_TABLE_NAME );
    iRet = this->_ConvertWholeRoleToSql( rstRoleWholeData, pszSql, DB_SQL_STR_SIZE );
    if( iRet < 0 ) return iRet;
    StrCat( pszSql, DB_SQL_STR_SIZE, "WHERE Uin=%lu", rstRoleWholeData.m_stBaseInfo.m_ullUin );

    if( m_poMysqlHandler->Execute( pszSql ) < 0 ||
        m_poMysqlHandler->AffectedRows() != 1 )
    {
        LOGERR_r( "Save role data failed! Uin <%lu>, RoleName <%s>", rstRoleWholeData.m_stBaseInfo.m_ullUin, rstRoleWholeData.m_stBaseInfo.m_szRoleName );
        return ERR_DB_ERROR;
    }

    return ERR_NONE;
}

// ×ª»»whole roleÎªsql string
int RoleTable::_ConvertWholeRoleToSql(IN PKGMETA::DT_ROLE_WHOLE_DATA& rstRoleWholeData, INOUT char* pszSql, int iSqlSize)
{
    assert( pszSql );

    int iRet = 0;

    this->_ConvertBaseInfoToSql( rstRoleWholeData.m_stBaseInfo, pszSql, iSqlSize );
    SQL_ADD_DELIMITER( pszSql, iSqlSize );

    iRet = this->_ConvertBlobInfoToSql( (char*)rstRoleWholeData.m_stMajestyBlob.m_szData, rstRoleWholeData.m_stMajestyBlob.m_iLen, ROLE_DATA_TYPE_MAJESTY, pszSql, iSqlSize );
    if( iRet < 0 )
        return iRet;

    SQL_ADD_DELIMITER( pszSql, iSqlSize );

    iRet = this->_ConvertBlobInfoToSql( (char*)rstRoleWholeData.m_stEquipBlob.m_szData, rstRoleWholeData.m_stEquipBlob.m_iLen, ROLE_DATA_TYPE_EQUIP, pszSql, iSqlSize );
    if( iRet < 0 )
        return iRet;

    SQL_ADD_DELIMITER( pszSql, iSqlSize );

    iRet = this->_ConvertBlobInfoToSql( (char*)rstRoleWholeData.m_stGCardBlob.m_szData, rstRoleWholeData.m_stGCardBlob.m_iLen, ROLE_DATA_TYPE_GCARD, pszSql, iSqlSize );
    if( iRet < 0 )
        return iRet;

    SQL_ADD_DELIMITER( pszSql, iSqlSize );

    iRet = this->_ConvertBlobInfoToSql( (char*)rstRoleWholeData.m_stPropsBlob.m_szData, rstRoleWholeData.m_stPropsBlob.m_iLen, ROLE_DATA_TYPE_PROPS, pszSql, iSqlSize );
    if( iRet < 0 )
        return iRet;

	SQL_ADD_DELIMITER( pszSql, iSqlSize );

	iRet = this->_ConvertBlobInfoToSql( (char*)rstRoleWholeData.m_stItemsBlob.m_szData, rstRoleWholeData.m_stItemsBlob.m_iLen, ROLE_DATA_TYPE_ITEMS, pszSql, iSqlSize );
	if( iRet < 0 )
		return iRet;

    SQL_ADD_DELIMITER( pszSql, iSqlSize );

    iRet = this->_ConvertBlobInfoToSql( (char*)rstRoleWholeData.m_stELOBlob.m_szData, rstRoleWholeData.m_stELOBlob.m_iLen, ROLE_DATA_TYPE_ELO, pszSql, iSqlSize );
    if( iRet < 0 )
        return iRet;

    SQL_ADD_DELIMITER( pszSql, iSqlSize );

    iRet = this->_ConvertBlobInfoToSql( (char*)rstRoleWholeData.m_stTaskBlob.m_szData, rstRoleWholeData.m_stTaskBlob.m_iLen, ROLE_DATA_TYPE_TASK, pszSql, iSqlSize );
    if( iRet < 0 )
        return iRet;

    SQL_ADD_DELIMITER( pszSql, iSqlSize );
    iRet = this->_ConvertBlobInfoToSql( (char*)rstRoleWholeData.m_stMSkillBlob.m_szData, rstRoleWholeData.m_stMSkillBlob.m_iLen, ROLE_DATA_TYPE_MSKILL, pszSql, iSqlSize );
    if( iRet < 0 )
        return iRet;

    SQL_ADD_DELIMITER( pszSql, iSqlSize );
    iRet = this->_ConvertBlobInfoToSql( (char*)rstRoleWholeData.m_stPveBlob.m_szData, rstRoleWholeData.m_stPveBlob.m_iLen, ROLE_DATA_TYPE_PVE, pszSql, iSqlSize );
    if( iRet < 0 )
        return iRet;

    SQL_ADD_DELIMITER( pszSql, iSqlSize );
    iRet = this->_ConvertBlobInfoToSql( (char*)rstRoleWholeData.m_stMiscBlob.m_szData, rstRoleWholeData.m_stMiscBlob.m_iLen, ROLE_DATA_TYPE_MISC, pszSql, iSqlSize );
    if( iRet < 0 )
        return iRet;

    SQL_ADD_DELIMITER( pszSql, iSqlSize );
    iRet = this->_ConvertBlobInfoToSql( (char*)rstRoleWholeData.m_stGuildBlob.m_szData, rstRoleWholeData.m_stGuildBlob.m_iLen, ROLE_DATA_TYPE_GUILD, pszSql, iSqlSize );
    if( iRet < 0 )
        return iRet;
    
    SQL_ADD_DELIMITER( pszSql, iSqlSize );
    iRet = this->_ConvertBlobInfoToSql( (char*)rstRoleWholeData.m_stTacticsBlob.m_szData, rstRoleWholeData.m_stTacticsBlob.m_iLen, ROLE_DATA_TYPE_TACTICS, pszSql, iSqlSize );
    if( iRet < 0 )
        return iRet;
    return ERR_NONE;
}

void RoleTable::_ConvertBaseInfoToSql( IN PKGMETA::DT_ROLE_BASE_INFO& rstBaseInfo, INOUT char* pszSql, int iSqlSize )
{
    assert( pszSql );

    StrCat(pszSql, iSqlSize, "Uin=%lu, RoleName='%s', FirstLoginTime=%lu, LastLoginTime=%lu, BlackRoomTime=%lu, BagSeq=%u, Version=%u  ",
            rstBaseInfo.m_ullUin,
            rstBaseInfo.m_szRoleName,
            rstBaseInfo.m_llFirstLoginTime,
            rstBaseInfo.m_llLastLoginTime,
            rstBaseInfo.m_llBlackRoomTime,
            rstBaseInfo.m_dwBagSeq,
            rstBaseInfo.m_wVersion);

    return;
}

int RoleTable::_ConvertBlobInfoToSql( char* pszData, int iLen, int iType, INOUT char* pszSql, int iSqlSize )
{
    if( !pszData || iLen < 0 )
    {
        return ERR_SYS;
    }

    char* pszBinBuf = NULL;
    int iBufSize = 0;
    const char* pszBlobName = NULL;

    switch( iType )
    {
        case ROLE_DATA_TYPE_MAJESTY:
            pszBinBuf = m_szMajestyBinBuf;
            iBufSize = MAJESTY_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_ROLE_MAJESTY;
            break;
        case ROLE_DATA_TYPE_EQUIP:
            pszBinBuf = m_szEquipBinBuf;
            iBufSize = EQUIP_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_ROLE_EQUIP;
            break;
        case ROLE_DATA_TYPE_GCARD:
            pszBinBuf = m_szGCardBinBuf;
            iBufSize = GCARD_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_ROLE_GCARD;
            break;
        case ROLE_DATA_TYPE_PROPS:
            pszBinBuf = m_szPropsBinBuf;
            iBufSize = PROPS_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_ROLE_PROPS;
            break;
		case ROLE_DATA_TYPE_ITEMS:
			pszBinBuf = m_szItemsBinBuf;
			iBufSize = ITEMS_INFO_BINBUF_SIZE;
			pszBlobName = BLOB_NAME_ROLE_ITEMS;
			break;
        case ROLE_DATA_TYPE_ELO:
            pszBinBuf = m_szELOBinBuf;
            iBufSize = ELO_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_ROLE_ELO;
            break;
        case ROLE_DATA_TYPE_TASK:
            pszBinBuf = m_szTaskBinBuf;
            iBufSize = TASK_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_ROLE_TASK;
            break;
        case ROLE_DATA_TYPE_MSKILL:
            pszBinBuf = m_szMSkillBinBuf;
            iBufSize = MSKILL_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_ROLE_MSKILL;
            break;
        case ROLE_DATA_TYPE_PVE:
            pszBinBuf = m_szPveBinBuf;
            iBufSize = PVE_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_ROLE_PVE;
            break;
        case ROLE_DATA_TYPE_MISC:
            pszBinBuf = m_szMiscBinBuf;
            iBufSize = MISC_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_ROLE_MISC;
            break;
        case ROLE_DATA_TYPE_GUILD:
            pszBinBuf = m_szGuildBinBuf;
            iBufSize = GUILD_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_ROLE_GUILD;
            break;
        case ROLE_DATA_TYPE_TACTICS:
            pszBinBuf = m_szTacticsBinBuf;
            iBufSize = TACTICS_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_ROLE_TACTICS;
            break;
        default:
            return ERR_SYS;
    }

    if( !m_poMysqlHandler->BinToStr( pszData, iLen, pszBinBuf, iBufSize ) )
    {
        LOGERR_r( "convert to mysql bin failed, type <%d>", iType );
        return ERR_SYS;
    }

    StrCat( pszSql, iSqlSize, "`%s`='%s'", pszBlobName, pszBinBuf );

    return ERR_NONE;
}

int RoleTable::CheckExist( uint64_t ullUin )
{
    LOGRUN_r("Check role in");
    //int iTableNum = GET_TABLE_NUM(ullUin);

    m_poMysqlHandler->FormatSql( "SELECT Uin FROM %s WHERE Uin=%lu", ROLE_TABLE_NAME, ullUin );
    if( m_poMysqlHandler->Execute() < 0 )
    {
        return ERR_DB_ERROR;
    }

    return m_poMysqlHandler->StoreResult();
}

int RoleTable::CheckExist(char* pszName)
{
    LOGRUN_r("Check role in");

    //int iTableNum = m_pstConfig->m_iSvrID;
    m_poMysqlHandler->FormatSql("SELECT RoleName FROM %s WHERE RoleName=\'%s\'", ROLE_TABLE_NAME, pszName);
    if ( m_poMysqlHandler->Execute() < 0 )
    {
        LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }

    if (m_poMysqlHandler->StoreResult() != 0)
    {
        return ERR_ALREADY_EXISTED;
    }

    return ERR_NONE;
}

int RoleTable::UptRoleName(uint64_t ullUin, char* pszName)
{
    int iRet = CheckExist(pszName);
    if (iRet != ERR_NONE)
    {
        return iRet;
    }

    //int iTableNum = GET_TABLE_NUM(ullUin);

    char* pszSql = m_poMysqlHandler->GetSql();
    pszSql[0] = '\0';

    StrCat(pszSql, DB_SQL_STR_SIZE, "UPDATE %s SET RoleName='%s' WHERE Uin=%lu",
            ROLE_TABLE_NAME, pszName, ullUin);

    if (m_poMysqlHandler->Execute( pszSql ) < 0)
    {
        LOGERR_r("Update RoleName failed! Uin(%lu), RoleName(%s) ErrInfo <%s> \n Sql : \n %s",
                   ullUin, pszName, m_poMysqlHandler->GetLastErrMsg(), pszSql);
        return ERR_DB_ERROR;
    }

    return ERR_NONE;
}

int RoleTable::GetRoleWholeData( uint64_t ullUin, OUT PKGMETA::DT_ROLE_WHOLE_DATA& rstRoleWholeData )
{
    LOGRUN_r("GetWholeRoleData in");
    if( 0 == ullUin )
    {
        return ERR_WRONG_PARA;
    }

    //int iTableNum = GET_TABLE_NUM(ullUin);
    m_poMysqlHandler->FormatSql( "SELECT RoleName, BagSeq, FirstLoginTime, LastLoginTime, BlackRoomTime, Version, "
                                 "%s, " /*1*/
                                 "%s, " /*2*/
                                 "%s, " /*3*/
                                 "%s, " /*4*/
                                 "%s, " /*5*/
                                 "%s, " /*6*/
                                 "%s, " /*7*/
                                 "%s, " /*8*/
                                 "%s, " /*9*/
                                 "%s, " /*10*/
                                 "%s, " /*11*/
                                 "%s "  /*12*/
                                 /*×¢Òâ ¶ººÅ !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! "%s," */
                                 "FROM %s WHERE Uin=%lu",
                                 BLOB_NAME_ROLE_MAJESTY, /*1*/
                                 BLOB_NAME_ROLE_EQUIP,  /*2*/
                                 BLOB_NAME_ROLE_GCARD,  /*3*/
                                 BLOB_NAME_ROLE_PROPS,  /*4*/
								 BLOB_NAME_ROLE_ITEMS,  /*5*/
                                 BLOB_NAME_ROLE_ELO,    /*6*/
                                 BLOB_NAME_ROLE_TASK,   /*7*/
                                 BLOB_NAME_ROLE_MSKILL,   /*8*/
                                 BLOB_NAME_ROLE_PVE,   /*9*/
                                 BLOB_NAME_ROLE_MISC,   /*10*/
                                 BLOB_NAME_ROLE_GUILD,   /*11*/
                                 BLOB_NAME_ROLE_TACTICS,    /*12*/
                                ROLE_TABLE_NAME, ullUin );
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
    oRowIter.Begin( m_poMysqlHandler->GetResult() );

    DT_ROLE_BASE_INFO& rstBaseInfo = rstRoleWholeData.m_stBaseInfo;
    rstBaseInfo.m_ullUin = ullUin;
    StrCpy( rstBaseInfo.m_szRoleName, oRowIter.GetField(i++)->GetString(), MAX_NAME_LENGTH );
    rstBaseInfo.m_dwBagSeq = (uint32_t)oRowIter.GetField(i++)->GetInteger();
    rstBaseInfo.m_llFirstLoginTime = (int64_t)oRowIter.GetField(i++)->GetInteger();
    rstBaseInfo.m_llLastLoginTime = (int64_t)oRowIter.GetField(i++)->GetInteger();
    rstBaseInfo.m_llBlackRoomTime = (int64_t)oRowIter.GetField(i++)->GetInteger();
    rstBaseInfo.m_wVersion = (uint16_t)oRowIter.GetField(i++)->GetInteger();
    rstRoleWholeData.m_stMajestyBlob.m_iLen = oRowIter.GetField(i++)->GetBinData( (char*)rstRoleWholeData.m_stMajestyBlob.m_szData, MAX_LEN_ROLE_MAJESTY);
    rstRoleWholeData.m_stEquipBlob.m_iLen = oRowIter.GetField(i++)->GetBinData( (char*)rstRoleWholeData.m_stEquipBlob.m_szData, MAX_LEN_ROLE_EQUIP);
    rstRoleWholeData.m_stGCardBlob.m_iLen = oRowIter.GetField(i++)->GetBinData( (char*)rstRoleWholeData.m_stGCardBlob.m_szData, MAX_LEN_ROLE_GCARD);
    rstRoleWholeData.m_stPropsBlob.m_iLen = oRowIter.GetField(i++)->GetBinData( (char*)rstRoleWholeData.m_stPropsBlob.m_szData, MAX_LEN_ROLE_PROPS);
	rstRoleWholeData.m_stItemsBlob.m_iLen = oRowIter.GetField(i++)->GetBinData( (char*)rstRoleWholeData.m_stItemsBlob.m_szData, MAX_LEN_ROLE_ITEMS);
    rstRoleWholeData.m_stELOBlob.m_iLen = oRowIter.GetField(i++)->GetBinData( (char*)rstRoleWholeData.m_stELOBlob.m_szData, MAX_LEN_ROLE_ELO);
    rstRoleWholeData.m_stTaskBlob.m_iLen = oRowIter.GetField(i++)->GetBinData( (char*)rstRoleWholeData.m_stTaskBlob.m_szData, MAX_LEN_ROLE_TASK);
    rstRoleWholeData.m_stMSkillBlob.m_iLen = oRowIter.GetField(i++)->GetBinData( (char*)rstRoleWholeData.m_stMSkillBlob.m_szData, MAX_LEN_ROLE_MSKILL);
    rstRoleWholeData.m_stPveBlob.m_iLen = oRowIter.GetField(i++)->GetBinData( (char*)rstRoleWholeData.m_stPveBlob.m_szData, MAX_LEN_ROLE_PVE);
    rstRoleWholeData.m_stMiscBlob.m_iLen = oRowIter.GetField(i++)->GetBinData( (char*)rstRoleWholeData.m_stMiscBlob.m_szData, MAX_LEN_ROLE_MISC);
    rstRoleWholeData.m_stGuildBlob.m_iLen = oRowIter.GetField(i++)->GetBinData( (char*)rstRoleWholeData.m_stGuildBlob.m_szData, MAX_LEN_ROLE_GUILD);
    rstRoleWholeData.m_stTacticsBlob.m_iLen = oRowIter.GetField(i++)->GetBinData( (char*)rstRoleWholeData.m_stTacticsBlob.m_szData, MAX_LEN_ROLE_TACTICS);

    LOGRUN_r("GetWholeRoleData %s gcard blob len-%d", rstBaseInfo.m_szRoleName, rstRoleWholeData.m_stGCardBlob.m_iLen);
    return ERR_NONE;
}

int RoleTable::UptRoleData( IN PKGMETA::SS_PKG_ROLE_UPDATE_REQ& rstRoleUptReq )
{
    if( rstRoleUptReq.m_nCount <= 0 )
    {
        return ERR_WRONG_PARA;
    }

    //int iTableNum = GET_TABLE_NUM(rstRoleUptReq.m_ullUin);

    int iRet = 0;
    char* pszSql = m_poMysqlHandler->GetSql();
    int iSqlSize = DB_SQL_STR_SIZE;
    pszSql[0] = '\0';
    PKGMETA::DT_ROLE_UPDATE_DATA* pstUptData = NULL;

    // base info
    StrCat( pszSql, DB_SQL_STR_SIZE, "UPDATE %s SET ", ROLE_TABLE_NAME);
    for( int i = 0; i < rstRoleUptReq.m_nCount; i++ )
    {
        pstUptData = &rstRoleUptReq.m_astRoleUpdate[i].m_stData;
        if( ROLE_DATA_TYPE_BASE  == rstRoleUptReq.m_astRoleUpdate[i].m_bType )
        {
             this->_ConvertBaseInfoToSql( pstUptData->m_stBaseInfo, pszSql, iSqlSize );
        }
        else
        {
            char* pData = (char*)pstUptData + sizeof(int32_t);
            int iLen = *(int*)( pstUptData );
            iRet = this->_ConvertBlobInfoToSql( pData, iLen, rstRoleUptReq.m_astRoleUpdate[i].m_bType, pszSql, iSqlSize );
            if( iRet < 0 )
                return iRet;
        }
        if( i != rstRoleUptReq.m_nCount - 1 )
            SQL_ADD_DELIMITER( pszSql, iSqlSize );
    }

    if( rstRoleUptReq.m_ullUin > 0 )
    {
        StrCat( pszSql, DB_SQL_STR_SIZE, "WHERE Uin=%lu", rstRoleUptReq.m_ullUin );
    }else
    {
        StrCat( pszSql, DB_SQL_STR_SIZE, "WHERE binary RoleName='%s'", rstRoleUptReq.m_szRoleName );
    }

    if( m_poMysqlHandler->Execute( pszSql ) < 0 )
    {
        LOGERR_r( "Update role data failed! Uin <%lu>, RoleName <%s> ErrInfo <%s> \n Sql : \n %s", rstRoleUptReq.m_ullUin, rstRoleUptReq.m_szRoleName, m_poMysqlHandler->GetLastErrMsg(), pszSql );
        return ERR_DB_ERROR;
    }

    return ERR_NONE;
}
int RoleTable::GetRolePropsInfo(uint64_t ullUin, OUT PKGMETA::DT_ROLE_PROPS_BLOB& stPropsBlob)
{
	m_poMysqlHandler->FormatSql(
		"SELECT  "
		BLOB_NAME_ROLE_PROPS /*1*/
		/*×¢Òâ ¶ººÅ !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! "%s," */
		" FROM %s WHERE Uin=%lu ",
		ROLE_TABLE_NAME, ullUin);
	if (m_poMysqlHandler->Execute() < 0)
	{
		LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg());
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

	CMysqlRowIter& oRowIter = m_poMysqlHandler->GetRowIter();
	oRowIter.Begin(m_poMysqlHandler->GetResult());
	stPropsBlob.m_iLen = oRowIter.GetField(0)->GetBinData((char*)stPropsBlob.m_szData, MAX_LEN_ROLE_MAJESTY);
	return ERR_NONE;
}
int RoleTable::GetRoleMajestyInfo(uint64_t ullUin, OUT PKGMETA::DT_ROLE_MAJESTY_BLOB& rstMajestyBlob)
{
    //int iTableNum = GET_TABLE_NUM(ullUin);
    m_poMysqlHandler->FormatSql(
        "SELECT  "
        BLOB_NAME_ROLE_MAJESTY /*1*/
        /*×¢Òâ ¶ººÅ !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! "%s," */
        " FROM %s WHERE Uin=%lu ",
        ROLE_TABLE_NAME, ullUin );
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

    CMysqlRowIter& oRowIter = m_poMysqlHandler->GetRowIter();
    oRowIter.Begin( m_poMysqlHandler->GetResult() );
    rstMajestyBlob.m_iLen = oRowIter.GetField(0)->GetBinData( (char*)rstMajestyBlob.m_szData, MAX_LEN_ROLE_MAJESTY);
    return ERR_NONE;
}

int RoleTable::SaveRoleMajestyInfo(uint64_t ullUin, IN PKGMETA::DT_ROLE_MAJESTY_BLOB& stMajestyBlob)
{
    //int iTableNum = GET_TABLE_NUM(ullUin);


    int iRet = 0;
    char* pszSql = m_poMysqlHandler->GetSql();
    pszSql[0] = '\0';

    StrCat( pszSql, DB_SQL_STR_SIZE, "UPDATE %s SET ", ROLE_TABLE_NAME );
    int iSqlSize = DB_SQL_STR_SIZE;
    iRet = this->_ConvertBlobInfoToSql( (char*)stMajestyBlob.m_szData, stMajestyBlob.m_iLen, ROLE_DATA_TYPE_MAJESTY, pszSql, iSqlSize );
    if( iRet < 0 )
        return iRet;

    StrCat( pszSql, DB_SQL_STR_SIZE, " WHERE Uin=%lu ", ullUin);

    if( m_poMysqlHandler->Execute( pszSql ) < 0 ||
        m_poMysqlHandler->AffectedRows() != 1 )
    {
        LOGERR_r( "Gm:Save role data failed! Uin <%lu>", ullUin);
        return ERR_DB_ERROR;
    }

    return ERR_NONE;
}
int RoleTable::SaveRolePropsInfo(uint64_t ullUin, IN PKGMETA::DT_ROLE_PROPS_BLOB& stPropsBlob)
{

	int iRet = 0;
	char* pszSql = m_poMysqlHandler->GetSql();
	pszSql[0] = '\0';

	StrCat(pszSql, DB_SQL_STR_SIZE, "UPDATE %s SET ", ROLE_TABLE_NAME);
	int iSqlSize = DB_SQL_STR_SIZE;
	iRet = this->_ConvertBlobInfoToSql((char*)stPropsBlob.m_szData, stPropsBlob.m_iLen, ROLE_DATA_TYPE_PROPS, pszSql, iSqlSize);
	if (iRet < 0)
		return iRet;

	StrCat(pszSql, DB_SQL_STR_SIZE, " WHERE Uin=%lu ", ullUin);

	if (m_poMysqlHandler->Execute(pszSql) < 0 ||
		m_poMysqlHandler->AffectedRows() != 1)
	{
		LOGERR_r("Gm:Save props data failed! Uin <%lu>", ullUin);
		return ERR_DB_ERROR;
	}
	return ERR_NONE;
}
int RoleTable::SaveRoleBackRoomTime(uint64_t ullUin , uint64_t tTime)
{
    //int iTableNum = GET_TABLE_NUM(ullUin);
    char* pszSql = m_poMysqlHandler->GetSql();
    pszSql[0] = '\0';
    StrCat( pszSql, DB_SQL_STR_SIZE,
            "UPDATE %s SET BlackRoomTime=%lu WHERE Uin=%lu ",
            ROLE_TABLE_NAME, tTime, ullUin);
    if( m_poMysqlHandler->Execute( pszSql ) < 0 ||
        m_poMysqlHandler->AffectedRows() != 1 )
    {
        LOGERR_r( "Gm:Save role data failed! Uin <%lu>", ullUin);
        return ERR_DB_ERROR;
    }
    return ERR_NONE;
}

int RoleTable::GetRoleUin(const char* szName, char* szResult, size_t size)
{

    //for (int iTableNum = 1; iTableNum <= m_iRoleTableNum; iTableNum++)
    {
        //int iTableNum = m_pstConfig->m_iSvrID;
        m_poMysqlHandler->FormatSql( "SELECT Uin,RoleName "
            "FROM %s WHERE RoleName='%s' ",
            ROLE_TABLE_NAME,
            szName);
        if (m_poMysqlHandler->Execute( ) < 0 )
        {
            LOGERR_r("excute sql failed: %s, Uin <%s>",
                m_poMysqlHandler->GetLastErrMsg(), szName);
            return ERR_DB_ERROR;
        }
        if (m_poMysqlHandler->StoreResult() > 0 )
        {
            CMysqlRowIter& oRowIter = m_poMysqlHandler->GetRowIter();
            oRowIter.Begin(m_poMysqlHandler->GetResult());
            snprintf(szResult, size,  "%lu::%s", (uint64_t)oRowIter.GetField(0)->GetBigInteger(), oRowIter.GetField(1)->GetString());
            return ERR_NONE;
        }
    }

    return ERR_NOT_FOUND;
}

int RoleTable::GetRoleUin(const char* szName, uint64_t& ullUin)
{
    //int iTableNum = m_pstConfig->m_iSvrID;
    m_poMysqlHandler->FormatSql( "SELECT Uin "
        "FROM %s WHERE RoleName='%s' ",
        ROLE_TABLE_NAME,
        szName);
    if (m_poMysqlHandler->Execute( ) < 0 )
    {
        LOGERR_r("excute sql failed: %s, Uin <%s>",
            m_poMysqlHandler->GetLastErrMsg(), szName);
        return ERR_DB_ERROR;
    }
    if (m_poMysqlHandler->StoreResult() > 0 )
    {
        CMysqlRowIter& oRowIter = m_poMysqlHandler->GetRowIter();
        oRowIter.Begin(m_poMysqlHandler->GetResult());
        ullUin = (uint64_t)oRowIter.GetField(0)->GetBigInteger();
        return ERR_NONE;
    }
    return ERR_NOT_FOUND;
}
int RoleTable::DelRoleProps(uint64_t ullUin, uint32_t itemID, int count)
{
	bzero(&m_stPropsBlob, sizeof(m_stPropsBlob));
	bzero(&m_stPropsInfo, sizeof(m_stPropsInfo));
	GetRolePropsInfo(ullUin, m_stPropsBlob);

	int iRet = 0;
	iRet = m_stPropsInfo.unpack((char *)m_stPropsBlob.m_szData, sizeof(m_stPropsBlob.m_szData));
	if (iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR("unpack DT_ROLE_PROPS_BLOB failed!");
		return ERR_SYS;
	}
	int iPropkindNum = m_stPropsInfo.m_iCount;
	for (int i = 0; i < iPropkindNum; i++)
	{
		if ( m_stPropsInfo.m_astData[i].m_dwId == itemID )
		{
			if ( m_stPropsInfo.m_astData[i].m_dwNum >= count )
				m_stPropsInfo.m_astData[i].m_dwNum -= count;
			else
				m_stPropsInfo.m_astData[i].m_dwNum = 0;
		}
	}
	bzero(&m_stPropsBlob, sizeof(m_stPropsBlob));
	size_t ulUseSize = 0;

	iRet = m_stPropsInfo.pack((char*)m_stPropsBlob.m_szData, sizeof(m_stPropsBlob.m_szData), &ulUseSize);
	if (iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR("pack DT_PROPS_BLOB failed!");
		return false;
	}
	m_stPropsBlob.m_iLen = (int)ulUseSize;
	iRet = SaveRolePropsInfo(ullUin, m_stPropsBlob);

	return iRet;
}

int RoleTable::GetRoleName(uint64_t ullUin, char* szResult, size_t size)
{
    //int iTableNum = GET_TABLE_NUM(ullUin);
    m_poMysqlHandler->FormatSql( "SELECT Uin,RoleName "
        /*×¢Òâ ¶ººÅ !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! "%s," */
        "FROM %s WHERE Uin='%lu' ",
        ROLE_TABLE_NAME,
         ullUin);

    if (m_poMysqlHandler->Execute( ) < 0 )
    {
        LOGERR_r("excute sql failed: %s, Uin <%lu>",
            m_poMysqlHandler->GetLastErrMsg(), ullUin);
        return ERR_DB_ERROR;
    }

    if (m_poMysqlHandler->StoreResult() > 0 )
    {
        CMysqlRowIter& oRowIter = m_poMysqlHandler->GetRowIter();
        oRowIter.Begin(m_poMysqlHandler->GetResult());
        snprintf(szResult, size, "%lu::%s", (uint64_t)oRowIter.GetField(0)->GetBigInteger(), oRowIter.GetField(1)->GetString());
        return ERR_NONE;
    }

    return ERR_NOT_FOUND;
}
