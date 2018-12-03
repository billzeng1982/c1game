#pragma once

/*
	账号表操作api, 注意所有api必须是Reentrant的
*/
#include "define.h"
#include "mysql/DataTable.h"
#include "ss_proto.h"
#include "cfg/AccountSvrCfgDesc.h"

class AccountTable : public CDataTable
{
public:
	AccountTable(){ }
	virtual ~AccountTable(){}
    bool CInit(const char* pszTableName, CMysqlHandler* poMysqlHandler, ACCOUNTSVRCFG*	pstConfig);
	int GetAccountData( const char* pszAccoutName, const char* pszChannelID, uint64_t ullUin, PKGMETA::SSDT_ACCOUNT_DATA& rstAccData );
	int CreateNewAccount( const char* pszAccoutName, const char* pszChannelID, uint64_t ullNewUin, PKGMETA::SSDT_ACCOUNT_DATA& rstAccData );
	int CreateNewAccount( const char* pszAccoutName, uint64_t ullNewUin, const char* pszPassWord, PKGMETA::SSDT_ACCOUNT_DATA& rstAccData );
	int CreateNewAccount( const PKGMETA::DT_ACCOUNT_INFO* pstAccInfo, uint64_t ullNewUin, int iParaNum );
	//int GetUin( const char* pszAccoutName,  uint64_t &uin );

	int GmUpdateAccountBanTime(uint64_t ullUin, uint64_t BanTime);
private:
    ACCOUNTSVRCFG*	m_pstConfig;
};

