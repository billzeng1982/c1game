#pragma once

/*
    角色表db操作api, 注意所有api必须是Reentrant的
*/
#include "mysql/MysqlHandler.h"
#include "ss_proto.h"
#include "define.h"
#include "singleton.h"
#include "../cfg/MineDBSvrCfgDesc.h"


class MineOreTable
{

public:
    MineOreTable();
    virtual ~MineOreTable();

    virtual bool Init( CMysqlHandler* poMysqlHandler, MINEDBSVRCFG* m_pstConfig, char* szBinBuff);
    int UptData(PKGMETA::DT_MINE_ORE_DATA* pstData, uint8_t bNum);
    int GetData(uint64_t ullKeyList[], uint8_t bNum, OUT PKGMETA::DT_MINE_ORE_DATA* pstData, OUT uint8_t* pOutNum);
    int DelData(uint64_t ullKeyList [], uint8_t bNum);
private:


private:
    CMysqlHandler m_stMysqlHandler;
    char m_szTableName[255];
    char* m_szBinBuf;
    MINEDBSVRCFG *	m_pstConfig;
};

