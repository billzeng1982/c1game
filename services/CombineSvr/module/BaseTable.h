#pragma once
#include <string.h>
#include <set>
#include "strutil.h"
#include "mysql/MysqlHandler.h"
#include "../cfg/CombineSvrCfgDesc.h"
#include "common_proto.h"
#include "TableDefine.h"
#include "LogMacros.h"

//	重命名的表:	
//		TABLE_COLUMN里,第一个写待重命名的字段,第二个写记录唯一ID,然后依次写字段,Blob集中放到最后,方便批量处理
//		如果Blob数量增加,数量宏也要增加（只针对需要重命名的表）
//	不需要重命名的表:
//		把所要拷贝的字段依次写对应的TABLE_COLUMN宏里


//不需要重命名的表
#define TABLE_COLUMN_ACCOUNT			"`Uin`, `AccountType`, `AccountName`, `PassWord`, `ChannelID`, `CreateTime`, `BanTime`"
#define TABLE_COLUMN_ASYNCPVP_TEAM		"`Uin`, `Version`, `TeamBlob`"
#define TABLE_COLUMN_ASYNCPVP_PLAYER	"`Uin`, `Version`, `DataBlob`"
#define TABLE_COLUMN_CLONEBATTLE_TEAM	"`Id`, `Version`, `TeamBlob`"
#define TABLE_COLUMN_GUILDPLAYER		"`Uin`,`GuildId`,`Version`,`ApplyBlob`"
#define TABLE_COLUMN_MAIL_PRI			"`Uin`,`PriSeq`,`PubSeq`,`Version`,`MailBoxPriBlob`"
#define TABLE_COLUMN_MAIL_PUB			"`Id`,`Version`,`DelFlag`,`StartTime`,`EndTime`,`MailDataBlob`"
#define TABLE_COLUMN_MESSAGE			"`Uin`,`Channel`,`DelFlag`,`Version`,`MessageBoxBlob`"
#define TABLE_COLUMN_MINE_ORE			"`Uid`,`Version`,`OreBlob`"
#define TABLE_COLUMN_MINE_PLAYER		"`Uid`,`Version`,`PlayerBlob`"



//需要重命名的表
#define TABLE_COLUMN_ROLE				"`RoleName`, `Uin`, `FirstLoginTime`, `LastLoginTime`, `BlackRoomTime`, `BagSeq`, `Version`, `MajestyBlob`, \
`EquipBlob`, `GCardBlob`, `PropsBlob`, `ItemsBlob`, `ELOBlob`, `TaskBlob`, `MSkillBlob`, `PveBlob`, `MiscBlob`, `GuildBlob`, `TacticsBlob` "
#define TABLE_COLUMN_FRIEND				"`Name`, `Uin`,`Version`,`ApplyBlob`,`AgreeBlob`,`SendBlob`,`RecvBlob`,`PlayerBlob`"
#define TABLE_COLUMN_GUILD				"`Name`, `GuildId`,`DelFlag`, `Version`,`MemberBlob`,`GlobalBlob`,`ApplyBlob`,`ReplayBlob`,`BossBlob`,`ScienceBlob` "

//重命名表的blob数量
#define ROLE_BLOB_COUNT		12		//Blob的数量
#define GUILD_BLOB_COUNT	6		//Blob的数量
#define FRIEND_BLOB_COUNT	5		//Blob的数量


using namespace PKGMETA;
using namespace std;

struct TableMsgRsp
{
	int m_iError;
	char m_szTableName[255];
	int m_iTableSvrId;	//表所属的服务器的ID
};

struct ThreadInitPara
{
	COMBINESVRCFG* m_pstCombineSvrCfg;
	const char* m_pstTableColumn;
	const char* m_pstTableName;
	int m_iTableSvrId;	//表所属的服务器的ID
};

class BaseTable
{
public:
	static const int MAX_BINBUF_SIZE = 5 * 1024 * 1024;
public:
	BaseTable() 
	{
		m_szTableName[0] = 0; 
		m_iTableSvrId = 0;
		m_poMysqlHandler = NULL;
		m_szTableColumn[0] = 0;
	}

	virtual ~BaseTable() { m_poMysqlHandler = NULL; }
	virtual int  Work() { return 0; }
	virtual bool Init(CMysqlHandler* poMysqlHandle, int iTableSvrId, const char* pstTableName, const char* pstTableColumn)
	{ 
		if (!poMysqlHandle || !pstTableName || !pstTableColumn)
		{
			//LOGERR_r("BaseTable::Init error!");
			return false;
		}
		StrCpy(m_szTableName, pstTableName, sizeof(m_szTableName));
		StrCpy(m_szTableColumn, pstTableColumn, sizeof(m_szTableColumn));
		m_iTableSvrId = iTableSvrId;
		m_poMysqlHandler = poMysqlHandle;
		return true;
	}
	virtual void Fini() {}


	const char* GetNewName(const char* szOldName, uint64_t ullUin)
	{
		if (m_RoleNameSet.find(szOldName) == m_RoleNameSet.end())
		{
			StrCpy(m_szRename, szOldName, MAX_NAME_LENGTH);
		}
		else
		{
			//重名 修改
			snprintf(m_szRename, MAX_NAME_LENGTH, "%s_%d", szOldName, m_iTableSvrId);
			//记录重命名的玩家Uin
			m_RenameUinSet.insert(ullUin);
			LOGWARN_r("%s=[%lu] name=%s   count=%lu", m_szTableName, ullUin, m_szRename, (uint64_t)m_RenameUinSet.size());
		}
		return m_szRename;
	}



	int GetLocalAllName(const char* pstColumnName, int iDelta)
	{

		int iIdStart = 0;
		int iSelectNum = 0;
		while (1)
		{
			//" Select `RoleName` From table where `RoleName` != '' limit 23,10"
			m_poMysqlHandler->FormatSql("SELECT `%s` FROM `%s` where  `%s` !='' limit %d, %d ",
				pstColumnName, m_szTableName, pstColumnName, iIdStart, iIdStart + iDelta);
			if (m_poMysqlHandler->Execute() < 0)
			{
				LOGERR_r("excute sql failed: %s", m_poMysqlHandler->GetLastErrMsg());
				return ERR_DB_ERROR;
			}
			iSelectNum = m_poMysqlHandler->StoreResult();
			if (iSelectNum == 0)
			{
				break;
			}
			iIdStart += iDelta;
			CMysqlRowIter& oRowIter = m_poMysqlHandler->GetRowIter();
			for (oRowIter.Begin(m_poMysqlHandler->GetResult()); !oRowIter.IsEnd(); oRowIter.Next())
			{
				m_RoleNameSet.insert(oRowIter.GetField(0)->GetString());
			}
		}
		//LOGRUN_r("GetLocalAllName  %s OK, total count<%d>", m_szTableName, iIdStart + iSelectNum);
		return ERR_NONE;
	}

	// 转换whole role为sql string
	int _ConvertWholeBinToSql(int iBlobCount ,IN CMysqlRowIter& oRowIter, int iIndex, INOUT char* pszSql, int iSqlSize)
	{
		assert(pszSql);

		int iRet = 0;
		///转换顺序需要和 Select中的顺序一致

		//数量是blob的数量
		for (int i = 0; i < iBlobCount; i++)
		{
			SQL_ADD_DELIMITER(pszSql, iSqlSize);
			// 		m_BlobSize = 0;
			// 		if (oRowIter.GetFieldLength(iIndex) != 0)
			// 		{
			// 			m_BlobSize = oRowIter.GetField(iIndex)->GetBinData(m_szBlobBuf, MAX_BINBUF_SIZE);
			// 		}
			// 		iRet = this->_ConvertBlobInfoToSql(m_szBlobBuf, m_BlobSize, pszSql, iSqlSize);

			//这样写可以少一次数据拷贝
			if (oRowIter.GetFieldLength(iIndex) != 0)
			{
				iRet = this->_ConvertBlobInfoToSql(oRowIter.GetField(iIndex)->GetBinDataPoint(), oRowIter.GetField(iIndex)->GetBinDataSize(), pszSql, iSqlSize);
			}
			else
			{
				char temp;
				iRet = this->_ConvertBlobInfoToSql(&temp, 0, pszSql, iSqlSize);
			}

			if (iRet < 0)
			{
				LOGERR_r("convert to mysql bin failed blob index<%d>", i);
				return iRet;
			}

			iIndex++;
		}
		return ERR_NONE;

	}

	int _ConvertBlobInfoToSql(char* pszData, int iLen, INOUT char* pszSql, int iSqlSize)
	{
		if (!pszData || iLen < 0)
		{
			return ERR_SYS;
		}
		if (iLen == 0)
		{
			//空的Blob
			StrCat(pszSql, iSqlSize, " NULL ");
			return ERR_NONE;
		}

		if (!m_poMysqlHandler->BinToStr(pszData, iLen, m_szMaxBinBuf, MAX_BINBUF_SIZE))
		{
			LOGERR_r("convert to mysql bin failed");
			return ERR_SYS;
		}

		StrCat(pszSql, iSqlSize, "'%s'", m_szMaxBinBuf);

		return ERR_NONE;
	}


protected:
	char m_szTableName[255];
	int m_iTableSvrId;	//表所属的服务器的ID
	char m_szTableColumn[1024];
	CMysqlHandler* m_poMysqlHandler;

	set<string> m_RoleNameSet;
	set<string>::iterator m_RoleNameSetIter;
	set<uint64_t> m_RenameUinSet;

	char m_szRename[255];
	char* m_szMaxBinBuf;	//由子类初始化
};


