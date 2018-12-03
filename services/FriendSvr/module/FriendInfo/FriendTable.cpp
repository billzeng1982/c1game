#include "FriendTable.h"
#include "LogMacros.h"
#include "comm_func.h"
#include "strutil.h"
#include "TableDefine.h"
#include "hash_func.h"
#include "PKGMETA_metalib.h"
using namespace PKGMETA;

FriendTable::FriendTable()
{
    m_szApplyBinBuf = NULL;
    m_szAgreeBinBuf = NULL;
    m_szPlayerBinBuf = NULL;
	m_szSendBinBuf = NULL;
	m_szRecvBinBuf = NULL;
}

FriendTable::~FriendTable()
{
    SAFE_DEL_ARRAY(m_szApplyBinBuf);
    SAFE_DEL_ARRAY(m_szAgreeBinBuf);
    SAFE_DEL_ARRAY(m_szPlayerBinBuf);
	SAFE_DEL_ARRAY(m_szSendBinBuf);
	SAFE_DEL_ARRAY(m_szRecvBinBuf);
}


bool FriendTable::CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, FRIENDSVRCFG*	pstConfig)
{
    if (NULL == pstConfig)
    {
        return false;
    }
    m_pstConfig = pstConfig;
    if(!CDataTable::Init(pszTableName, poMysqlHandler, m_pstConfig->m_iFriendTableNum))
    {
        return false;
    }

    m_szApplyBinBuf = new char[APPLY_INFO_BINBUF_SIZE];
    if (!m_szApplyBinBuf )
    {
        return false;
    }

    m_szAgreeBinBuf = new char[AGREE_INFO_BINBUF_SIZE];
    if (!m_szAgreeBinBuf )
    {
        return false;
    }

	m_szSendBinBuf = new char[SEND_INFO_BINBUF_SIZE];
	if (!m_szApplyBinBuf )
	{
		return false;
	}

	m_szRecvBinBuf = new char[RECV_INFO_BINBUF_SIZE];
	if (!m_szAgreeBinBuf )
	{
		return false;
	}
    DT_FRIEND_AGREE_INFO stAgree = { 0 }; 
    DT_FRIEND_APPLY_INFO stApply = { 0 };
	DT_FRIEND_SEND_AP_INFO stSend = { 0 };
	DT_FRIEND_RECV_AP_INFO stRecv = { 0 };
    size_t ullUseSize = 0;
    //申请列表 整理
    int iRet = stApply.pack((char*)m_stInitPlayerBlob.m_szData, MAX_LEN_FRIEND_APPLY_BLOB, &ullUseSize);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack m_stInitPlayerBlob failed, Ret=%d", iRet);
        return false;
    }
    m_stInitApplyBlob.m_iLen = (int)ullUseSize;

    //好友列表  整理
    iRet = stAgree.pack((char*)m_stInitAgreeBlob.m_szData, MAX_LEN_FRIEND_AGREE_BLOB, &ullUseSize);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack m_stInitAgreeBlob failed, Ret=%d", iRet);
        return false;
    }
    m_stInitAgreeBlob.m_iLen = (int)ullUseSize;

	//已送过体力的好友列表 整理
	iRet = stSend.pack((char*)m_stInitSendBlob.m_szData, MAX_LEN_FRIEND_SEND_BLOB, &ullUseSize);
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("pack m_stInitSendBlob failed, Ret=%d", iRet);
		return ERR_SYS;
	}
	m_stInitSendBlob.m_iLen = (int)ullUseSize;

	//该列表中的好友赠送的体力尚未被该玩家收取 整理
	iRet = stRecv.pack((char*)m_stInitRecvBlob.m_szData, MAX_LEN_FRIEND_RECV_BLOB, &ullUseSize);
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("pack m_stInitRecvBlob failed, Ret=%d", iRet);
		return ERR_SYS;
	}
	m_stInitRecvBlob.m_iLen = (int)ullUseSize;

    return true;
}

int FriendTable::CheckExist(uint64_t ullUin)
{
    //int iTableNum = GET_TABLE_NUM(ullUin);
    m_poMysqlHandler->FormatSql("SELECT PlayerBlob FROM %s WHERE Uin=%lu", FRIEND_TABLE_NAME, ullUin);
    if (m_poMysqlHandler->Execute() < 0)
    {
        return ERR_DB_ERROR;
    }

    return m_poMysqlHandler->StoreResult();
}


int FriendTable::CreateFriend(DT_FRIEND_WHOLE_DATA& rstWholeData)
{
    //检查是否存在 外部已检查了
    int iRet = ERR_NONE;
    rstWholeData.m_stAgreeBlob = m_stInitAgreeBlob;
    rstWholeData.m_stApplyBlob = m_stInitApplyBlob;
	rstWholeData.m_stSendBlob = m_stInitSendBlob;
	rstWholeData.m_stRecvBlob = m_stInitRecvBlob;
    //int iTableNum = GET_TABLE_NUM(rstWholeData.m_stBaseInfo.m_ullUin);
    char* pszSql = m_poMysqlHandler->GetSql();
    pszSql[0] = '\0';

    StrCat(pszSql, DB_SQL_STR_SIZE, "INSERT INTO %s SET ", FRIEND_TABLE_NAME);

    iRet = this->_ConvertWholeDataToSql(rstWholeData, pszSql, DB_SQL_STR_SIZE );
    if (iRet < 0)
    {
        LOGERR_r("Uin<%lu> CreateFriend:error", rstWholeData.m_stBaseInfo.m_ullUin);
        return iRet;
    }

    if (m_poMysqlHandler->Execute(pszSql) < 0)
    {
        LOGERR_r("Uin<%lu> excute sql failed: %s, SqlStr<%s>", rstWholeData.m_stBaseInfo.m_ullUin, m_poMysqlHandler->GetLastErrMsg(), pszSql);
        return ERR_DB_ERROR;
    }

    return ERR_NONE;
}

int FriendTable::SaveWholeData( IN DT_FRIEND_WHOLE_DATA & rstWholeData )
{
    if (0 == CheckExist(rstWholeData.m_stBaseInfo.m_ullUin))
    {// 检查,不存在 创建
        return CreateFriend(rstWholeData);
    }
    //int iTableNum = GET_TABLE_NUM(rstWholeData.m_stBaseInfo.m_ullUin);

    int iRet = 0;
    char* pszSql = m_poMysqlHandler->GetSql();
    pszSql[0] = '\0';

    // base info
    StrCat( pszSql, DB_SQL_STR_SIZE, "UPDATE %s SET ", FRIEND_TABLE_NAME);
    iRet = this->_ConvertWholeDataToSql( rstWholeData, pszSql, DB_SQL_STR_SIZE );
    if( iRet < 0 ) return iRet;
    StrCat( pszSql, DB_SQL_STR_SIZE, "WHERE Uin=%lu", rstWholeData.m_stBaseInfo.m_ullUin );

    if( m_poMysqlHandler->Execute( pszSql ) < 0 || 
        m_poMysqlHandler->AffectedRows() != 1 )
    {
        LOGERR_r( "Save role data failed! Uin <%lu>, SqlStr<%s>", rstWholeData.m_stBaseInfo.m_ullUin, m_poMysqlHandler->GetLastErrMsg());
        return ERR_DB_ERROR;
    }
    return ERR_NONE;
}



int FriendTable::_ConvertWholeDataToSql(DT_FRIEND_WHOLE_DATA& rstWholeData, char* pszSql, int iSqlSize)
{
    assert(pszSql);
    int iRet = 0;

    this->_ConvertBaseInfoToSql(rstWholeData.m_stBaseInfo, pszSql, iSqlSize);
    SQL_ADD_DELIMITER(pszSql, iSqlSize);

    iRet = this->_ConvertBlobInfoToSql( (char*)rstWholeData.m_stApplyBlob.m_szData, rstWholeData.m_stApplyBlob.m_iLen, FRIEND_DATA_TYPE_APPLY, pszSql, iSqlSize );
    if( iRet < 0 )
    {
        return iRet;
    }
    SQL_ADD_DELIMITER(pszSql, iSqlSize);
    iRet = this->_ConvertBlobInfoToSql( (char*)rstWholeData.m_stAgreeBlob.m_szData, rstWholeData.m_stAgreeBlob.m_iLen, FRIEND_DATA_TYPE_AGREE, pszSql, iSqlSize );
    if( iRet < 0 )
    {
        return iRet;
    }
	SQL_ADD_DELIMITER(pszSql, iSqlSize);
	iRet = this->_ConvertBlobInfoToSql( (char*)rstWholeData.m_stSendBlob.m_szData, rstWholeData.m_stSendBlob.m_iLen, FRIEND_DATA_TYPE_SEND, pszSql, iSqlSize );
	if( iRet < 0 )
	{
		return iRet;
	}
	SQL_ADD_DELIMITER(pszSql, iSqlSize);
	iRet = this->_ConvertBlobInfoToSql( (char*)rstWholeData.m_stRecvBlob.m_szData, rstWholeData.m_stRecvBlob.m_iLen, FRIEND_DATA_TYPE_RECV, pszSql, iSqlSize );
	if( iRet < 0 )
	{
		return iRet;
	}
    return ERR_NONE;
}

void FriendTable::_ConvertBaseInfoToSql(DT_FRIEND_BASE_INFO& rstBaseInfo, char* pszSql, int iSqlSize)
{
    assert(pszSql);
    StrCat(pszSql, iSqlSize, "Uin=%lu, Name='%s', Version=%d",
           rstBaseInfo.m_ullUin,
           rstBaseInfo.m_szName, PKGMETA::MetaLib::getVersion());
    return;
}

int FriendTable::_ConvertBlobInfoToSql(char* pszData, int iLen, int iType, char* pszSql, int iSqlSize)
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
        case FRIEND_DATA_TYPE_APPLY:
            pszBinBuf = m_szApplyBinBuf;
            iBufSize = APPLY_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_FRIEND_APPLY;
            break;
        case FRIEND_DATA_TYPE_AGREE:
            pszBinBuf = m_szAgreeBinBuf;
            iBufSize = AGREE_INFO_BINBUF_SIZE;
            pszBlobName = BLOB_NAME_FRIEND_AGREE;
            break;
		case FRIEND_DATA_TYPE_SEND:
			pszBinBuf = m_szSendBinBuf;
			iBufSize = SEND_INFO_BINBUF_SIZE;
			pszBlobName = BLOB_NAME_FRIEND_SEND;
			break;
		case FRIEND_DATA_TYPE_RECV:
			pszBinBuf = m_szRecvBinBuf;
			iBufSize = RECV_INFO_BINBUF_SIZE;
			pszBlobName = BLOB_NAME_FRIEND_RECV;
			break;
        default:
            LOGERR_r( "_ConvertBlobInfoToSql: type error <%d>", iType );
            return ERR_SYS;
    }

    if (!m_poMysqlHandler->BinToStr(pszData, iLen, pszBinBuf, iBufSize))
    {
        LOGERR_r( "_ConvertBlobInfoToSql failed, type <%d>", iType );
        return ERR_SYS;
    }

    StrCat(pszSql, iSqlSize, "`%s`='%s'", pszBlobName, pszBinBuf);

    return ERR_NONE;
}


int FriendTable::GetFriendWholeData(uint64_t ullUin, OUT DT_FRIEND_WHOLE_DATA& rstWholeData)
{
    //int iTableNum = GET_TABLE_NUM(ullUin);
    m_poMysqlHandler->FormatSql( "SELECT Name, Version, "
                                 "%s, "
								 "%s, "
								 "%s, "
                                 "%s "
                                 /*注意 逗号 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! "%s," */
                                 "FROM %s WHERE Uin=%lu ",
                                 BLOB_NAME_FRIEND_APPLY, /*1*/
                                 BLOB_NAME_FRIEND_AGREE,/*2*/
								 BLOB_NAME_FRIEND_SEND,
								 BLOB_NAME_FRIEND_RECV,
                                 FRIEND_TABLE_NAME,
                                 ullUin);
    if (m_poMysqlHandler->Execute( ) < 0 )
    {
        LOGERR_r("GetFriendWholeData excute sql failed: %s, Uin <%lu>",
                    m_poMysqlHandler->GetLastErrMsg(), ullUin);
        return ERR_DB_ERROR;
    }

    int iEffectRow = m_poMysqlHandler->StoreResult();
    if (iEffectRow < 0 )
    {
        LOGERR_r("Uin<%lu> GetFriendWholeData iEffectRow error ", ullUin);
        return ERR_DB_ERROR;
    }
    if (0 == iEffectRow)
    {
        LOGERR_r("Uin<%lu> GetFriendWholeData not found ", ullUin);
        return ERR_NOT_FOUND;
    }

    assert(1 == iEffectRow);

    CMysqlRowIter& oRowIter = m_poMysqlHandler->GetRowIter();
    int i = 0;
    oRowIter.Begin(m_poMysqlHandler->GetResult());

    DT_FRIEND_BASE_INFO& rstBaseInfo = rstWholeData.m_stBaseInfo;
    rstBaseInfo.m_ullUin= ullUin;
    StrCpy( rstBaseInfo.m_szName, oRowIter.GetField(i++)->GetString(), MAX_NAME_LENGTH);
    rstBaseInfo.m_wVersion = oRowIter.GetField(i++)->GetInteger();
    rstWholeData.m_stApplyBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstWholeData.m_stApplyBlob.m_szData, MAX_LEN_FRIEND_APPLY_BLOB);
    rstWholeData.m_stAgreeBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstWholeData.m_stAgreeBlob.m_szData, MAX_LEN_FRIEND_AGREE_BLOB);
	rstWholeData.m_stSendBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstWholeData.m_stSendBlob.m_szData, MAX_LEN_FRIEND_SEND_BLOB);
	rstWholeData.m_stRecvBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstWholeData.m_stRecvBlob.m_szData, MAX_LEN_FRIEND_RECV_BLOB);

    return ERR_NONE;
}

int FriendTable::GetFriendWholeData(const char*  cszName, OUT DT_FRIEND_WHOLE_DATA& rstWholeData)
{
    
    //for (int iTableNum = 1; iTableNum<=m_iTableNum; iTableNum++)
    {
        //int iTableNum = m_pstConfig->m_iSvrID;
        m_poMysqlHandler->FormatSql( "SELECT Uin, Version,"
            "%s, "
			"%s, "
			"%s, "
            "%s "
            /*注意 逗号 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! "%s," */
            "FROM %s WHERE Name='%s' ",
            BLOB_NAME_FRIEND_APPLY, /*1*/
            BLOB_NAME_FRIEND_AGREE,/*2*/
			BLOB_NAME_FRIEND_SEND,
			BLOB_NAME_FRIEND_RECV,
            FRIEND_TABLE_NAME,
             cszName);
        if (m_poMysqlHandler->Execute( ) < 0 )
        {
            LOGERR_r("GetFriendWholeData excute sql failed: %s, Name<%s>",
                m_poMysqlHandler->GetLastErrMsg(), cszName);
            return ERR_DB_ERROR;
        }
        if (m_poMysqlHandler->StoreResult() > 0 )
        {
            CMysqlRowIter& oRowIter = m_poMysqlHandler->GetRowIter();
            int i = 0;
            oRowIter.Begin(m_poMysqlHandler->GetResult());

            DT_FRIEND_BASE_INFO& rstBaseInfo = rstWholeData.m_stBaseInfo;
            StrCpy( rstBaseInfo.m_szName, cszName, MAX_NAME_LENGTH);
            rstBaseInfo.m_ullUin = oRowIter.GetField(i++)->GetBigInteger();
            rstBaseInfo.m_wVersion = oRowIter.GetField(i++)->GetInteger();
            rstWholeData.m_stApplyBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstWholeData.m_stApplyBlob.m_szData, MAX_LEN_FRIEND_APPLY_BLOB);
            rstWholeData.m_stAgreeBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstWholeData.m_stAgreeBlob.m_szData, MAX_LEN_FRIEND_AGREE_BLOB);
			rstWholeData.m_stSendBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstWholeData.m_stSendBlob.m_szData, MAX_LEN_FRIEND_SEND_BLOB);
			rstWholeData.m_stRecvBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstWholeData.m_stRecvBlob.m_szData, MAX_LEN_FRIEND_RECV_BLOB);
            return ERR_NONE;
        }
    }
    LOGERR_r("Name<%s> GetFriendWholeData not found ", cszName);
    return ERR_NOT_FOUND;

}

