#pragma once

#include "mysql/DataTable.h"
#include "common_proto.h"
#include "define.h"
#include "cfg/FriendSvrCfgDesc.h"

using namespace PKGMETA;

#define BLOB_NAME_FRIEND_APPLY 	    "ApplyBlob"
#define BLOB_NAME_FRIEND_AGREE	    "AgreeBlob"
#define BLOB_NAME_FRIEND_PLAYER	    "PlayerBlob"
#define BLOB_NAME_FRIEND_SEND		"SendBlob"
#define BLOB_NAME_FRIEND_RECV		"RecvBlob"

class FriendTable : public CDataTable
{
public:
    static const int APPLY_INFO_BINBUF_SIZE = MAX_LEN_FRIEND_APPLY_BLOB * 2 + 1;
    static const int AGREE_INFO_BINBUF_SIZE = MAX_LEN_FRIEND_AGREE_BLOB * 2 + 1;
	static const int SEND_INFO_BINBUF_SIZE = MAX_LEN_FRIEND_SEND_BLOB * 2 + 1;
	static const int RECV_INFO_BINBUF_SIZE = MAX_LEN_FRIEND_RECV_BLOB * 2 + 1;
public:
    FriendTable();
    virtual ~FriendTable();

    virtual bool CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, FRIENDSVRCFG*	m_pstConfig);
    int CheckExist(uint64_t llUin);         //Return = 0#没有|大于0#有|小于0#错误
    int CreateFriend(DT_FRIEND_WHOLE_DATA& rstWholeData);
    int GetFriendWholeData(uint64_t llUin, OUT DT_FRIEND_WHOLE_DATA& rstWholeData);
    int GetFriendWholeData(const char*  cszName, OUT DT_FRIEND_WHOLE_DATA& rstWholeData);
    int SaveWholeData( IN DT_FRIEND_WHOLE_DATA & rstWholeData );
private:
    int _ConvertWholeDataToSql(DT_FRIEND_WHOLE_DATA& rstWholeData, char* pszSql, int iSqlSize);
     void _ConvertBaseInfoToSql(DT_FRIEND_BASE_INFO& rstBaseInfo, char* pszSql, int iSqlSize);
     int _ConvertBlobInfoToSql(char* pszData, int iLen, int iType, char* pszSql, int iSqlSize);

private:
    DT_FRIEND_PLAYER_BLOB m_stInitPlayerBlob;	//这个是新玩家创建记录时用的
    DT_FRIEND_APPLY_BLOB m_stInitApplyBlob;
    DT_FRIEND_AGREE_BLOB m_stInitAgreeBlob;
	DT_FRIEND_SEND_AP_BLOB m_stInitSendBlob;
	DT_FRIEND_RECV_AP_BLOB m_stInitRecvBlob;
    char* m_szApplyBinBuf;
    char* m_szAgreeBinBuf;
	char* m_szSendBinBuf;
	char* m_szRecvBinBuf;
    char* m_szPlayerBinBuf;
    FRIENDSVRCFG*	m_pstConfig;
};


