#include "CommonTable.h"
#include "LogMacros.h"

#include "TableDefine.h"

int CommonTable::Combine()
{
	m_poMysqlHandler->FormatSql("insert into `%s` (%s) select %s from `%s_%d` ",
		m_szTableName, m_szTableColumn, m_szTableColumn, m_szTableName, m_iTableSvrId);
	if (m_poMysqlHandler->Execute() < 0)
	{
		LOGERR_r("Table<%s> excute sql failed, sql:%s", m_szTableName, m_poMysqlHandler->GetSql());
		LOGERR_r("Table<%s> excute sql failed: %s", m_szTableName ,m_poMysqlHandler->GetLastErrMsg());
		return ERR_DB_ERROR;
	}
	//LOGRUN_r("Combine OK for %s_%d", m_szTableName, m_iTableSvrId);
	return ERR_NONE;
}

int CommonTable::Work()
{
	if (0 == Combine())
	{
		LOGRUN_r("[OK] Table %s_%d combine  OK!", m_szTableName, m_iTableSvrId);
		return 1;
	}
	else
	{
		LOGRUN_r("[ER] Table %s_%d combine  ERR!", m_szTableName, m_iTableSvrId);
		return -1;
	}
}

