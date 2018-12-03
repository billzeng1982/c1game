#pragma once

/*
    角色表db操作api, 注意所有api必须是Reentrant的
*/

#include "ss_proto.h"
#include "mysql/MysqlHandler.h"
#include "define.h"
#include "cfg/LogQuerySvrCfgDesc.h"
#include "singleton.h"
#include "common_proto.h"

using namespace PKGMETA;

#define BLOB_NAME_ROLE_MAJESTY 		"MajestyBlob"
#define BLOB_NAME_ROLE_EQUIP 		"EquipBlob"
#define BLOB_NAME_ROLE_GCARD 		"GCardBlob"
#define BLOB_NAME_ROLE_MSKILL 		"MSkillBlob"
#define BLOB_NAME_ROLE_PROPS 		"PropsBlob"
#define BLOB_NAME_ROLE_ELO 			"ELOBlob"
#define BLOB_NAME_ROLE_TASK			"TaskBlob"
#define BLOB_NAME_ROLE_EQUIPPAGE	"EquipPageBlob"
#define BLOB_NAME_ROLE_PVE			"PveBlob"
#define BLOB_NAME_ROLE_MISC			"MiscBlob"
#define BLOB_NAME_ROLE_GUILD		"GuildBlob"


class RoleTableMgr : public CSingleton<RoleTableMgr>
{

public:
    RoleTableMgr();
    ~RoleTableMgr();

    bool Init(LOGQUERYSVRCFG* pstConfig);

public:

	int GetRoleMajestyInfo(uint64_t ullUin, OUT PKGMETA::DT_ROLE_MAJESTY_BLOB& rstMajestyBlob);
	int GetRoleGuildInfo(uint64_t ullUin, OUT PKGMETA::DT_ROLE_GUILD_BLOB& rstGuildBlob);
	int GetRoleGcardInfo(uint64_t ullUin, OUT PKGMETA::DT_ROLE_GCARD_BLOB& rstGcardBlob);
    int GetRoleUinByName(const char* szName, uint64_t& ullUin);
    int GetChannelName(uint64_t ullUin, OUT char* szChannelName, size_t size);//返回的是渠道号，在数据库中是varchar存储的

private:
	int _CheckMysqlConn();
    int _GetSvrId() {return m_pstConfig->m_iSvrId;}
private:
    CMysqlHandler m_stMysqlHandler;
    LOGQUERYSVRCFG* m_pstConfig;
};

