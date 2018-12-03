#pragma once

/*
角色表db操作api, 注意所有api必须是Reentrant的
*/
#include "mysql/MysqlHandler.h"
#include "ss_proto.h"
#include "define.h"
#include "singleton.h"
#include "../cfg/ClusterDBSvrCfgDesc.h"


class GuildExpeditionPlayerTable
{

public:
	GuildExpeditionPlayerTable();
	virtual ~GuildExpeditionPlayerTable();

	virtual bool Init(CMysqlHandler* poMysqlHandler, const char* pstTableName, CLUSTERDBSVRCFG* m_pstConfig, char* szBinBuff);
	int UptData(PKGMETA::DT_GUILD_EXPEDITION_PLAYER_DATA* pstData, int8_t bNum);
	int GetData(uint64_t ullKeyList[], int8_t bNum, OUT PKGMETA::DT_GUILD_EXPEDITION_PLAYER_DATA* pstData, OUT int8_t* pOutNum);
	//int DelData(uint64_t ullKeyList[], uint8_t bNum);
private:


private:
	CMysqlHandler m_stMysqlHandler;
	char m_szTableName[255];
	char* m_szBinBuf;
	CLUSTERDBSVRCFG *	m_pstConfig;
};

