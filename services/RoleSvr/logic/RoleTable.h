#pragma once

/*
    角色表db操作api, 注意所有api必须是Reentrant的
*/

#include "mysql/DataTable.h"
#include "ss_proto.h"
#include "define.h"
#include "cfg/RoleSvrCfgDesc.h"

using namespace PKGMETA;

#define BLOB_NAME_ROLE_MAJESTY 		"MajestyBlob"
#define BLOB_NAME_ROLE_EQUIP 		"EquipBlob"
#define BLOB_NAME_ROLE_GCARD 		"GCardBlob"
#define BLOB_NAME_ROLE_MSKILL 		"MSkillBlob"
#define BLOB_NAME_ROLE_PROPS 		"PropsBlob"
#define BLOB_NAME_ROLE_ITEMS 		"ItemsBlob"
#define BLOB_NAME_ROLE_ELO 			"ELOBlob"
#define BLOB_NAME_ROLE_TASK			"TaskBlob"
#define BLOB_NAME_ROLE_PVE			"PveBlob"
#define BLOB_NAME_ROLE_MISC			"MiscBlob"
#define BLOB_NAME_ROLE_GUILD		"GuildBlob"
#define BLOB_NAME_ROLE_TACTICS		"TacticsBlob"


class RoleTable : public CDataTable
{
public:
    static const int MAJESTY_INFO_BINBUF_SIZE 		= PKGMETA::MAX_LEN_ROLE_MAJESTY*2 + 1;
    static const int EQUIP_INFO_BINBUF_SIZE 		= PKGMETA::MAX_LEN_ROLE_EQUIP*2 + 1;
    static const int GCARD_INFO_BINBUF_SIZE 		= PKGMETA::MAX_LEN_ROLE_GCARD*2 + 1;
    static const int MSKILL_INFO_BINBUF_SIZE 		= PKGMETA::MAX_LEN_ROLE_MSKILL*2 + 1;
    static const int PROPS_INFO_BINBUF_SIZE 		= PKGMETA::MAX_LEN_ROLE_PROPS*2 + 1;
	static const int ITEMS_INFO_BINBUF_SIZE 		= PKGMETA::MAX_LEN_ROLE_ITEMS*2 + 1;
    static const int ELO_INFO_BINBUF_SIZE 			= PKGMETA::MAX_LEN_ROLE_ELO*2 + 1;
    static const int TASK_INFO_BINBUF_SIZE 			= PKGMETA::MAX_LEN_ROLE_TASK*2 + 1;
    static const int PVE_INFO_BINBUF_SIZE 			= PKGMETA::MAX_LEN_ROLE_PVE*2 + 1;
    static const int MISC_INFO_BINBUF_SIZE 			= PKGMETA::MAX_LEN_ROLE_MISC*2 + 1;
    static const int GUILD_INFO_BINBUF_SIZE         = PKGMETA::MAX_LEN_ROLE_GUILD*2 + 1;
    static const int TACTICS_INFO_BINBUF_SIZE       = PKGMETA::MAX_LEN_ROLE_TACTICS*2 + 1;

public:
    RoleTable();
    virtual ~RoleTable();

    int CreateRole( IN PKGMETA::DT_ROLE_WHOLE_DATA& rstRoleWholeData );
    virtual bool CInit( const char* pszTableName, CMysqlHandler* poMysqlHandler, ROLESVRCFG* m_pstConfig);

    // 0 - no exist, 1 - exist, < 0 error
    int CheckExist(uint64_t ullUin);
    int CheckExist(char* pszName);
    int ConvertToMysqlBin( char* pszData, int iLen, int iType );
    int SaveRoleWholeData( IN PKGMETA::DT_ROLE_WHOLE_DATA& rstRoleWholeData );
    int GetRoleWholeData( uint64_t ullUin, OUT PKGMETA::DT_ROLE_WHOLE_DATA& rstRoleWholeData );
    int UptRoleData( IN PKGMETA::SS_PKG_ROLE_UPDATE_REQ& rstRoleUptReq );
    int UptRoleName(uint64_t ullUin, char* pszName);

public:
    //Gm 操作
    int GetRoleBaseInfo(uint64_t ullUin, OUT PKGMETA::DT_ROLE_BASE_INFO& rstBaseInfo);
    int GetRoleMajestyInfo(uint64_t ullUin, OUT PKGMETA::DT_ROLE_MAJESTY_BLOB& stMajestyBlob);

	int GetRolePropsInfo(uint64_t ullUin, OUT PKGMETA::DT_ROLE_PROPS_BLOB& stPropsBlob);

    int SaveRoleMajestyInfo(uint64_t ullUin, IN PKGMETA::DT_ROLE_MAJESTY_BLOB& stMajestyBlob);
    int SaveRoleBackRoomTime(uint64_t ullUin , uint64_t tTime);

	int SaveRolePropsInfo(uint64_t ullUin, IN PKGMETA::DT_ROLE_PROPS_BLOB& stPropsBlob);

    int GetRoleUin(const char* szName, char* szResult,size_t size);
    int GetRoleUin(const char* szName, uint64_t& ullUin);

    int GetRoleName(uint64_t ullUin, char* szResult,size_t size);

	int DelRoleProps(uint64_t ullUin, uint32_t itemID, int count);

private:
    // 转换whole role为sql string
    int _ConvertWholeRoleToSql( IN PKGMETA::DT_ROLE_WHOLE_DATA& rstRoleWholeData, INOUT char* pszSql, int iSqlSize );
    void _ConvertBaseInfoToSql( IN PKGMETA::DT_ROLE_BASE_INFO& rstBaseInfo, INOUT char* pszSql, int iSqlSize );
    int _ConvertBlobInfoToSql( char* pszData, int iLen, int iType, INOUT char* pszSql, int iSqlSize );

private:
    char* m_szMajestyBinBuf;
    char* m_szEquipBinBuf;
    char* m_szGCardBinBuf;
    char* m_szMSkillBinBuf;
    char* m_szPropsBinBuf;
	char* m_szItemsBinBuf;
    char* m_szELOBinBuf;
    char* m_szTaskBinBuf;
    char* m_szEquipPageBinBuf;
    char* m_szPveBinBuf;
    char* m_szMiscBinBuf;
    char* m_szGuildBinBuf;
    char* m_szTacticsBinBuf;
private:
    DT_ROLE_BASE_INFO stBaseInfo;
    DT_ROLE_MAJESTY_INFO stMajestyInfo;
    DT_ROLE_MAJESTY_BLOB stMajestyBlob;

	DT_ROLE_PROPS_INFO m_stPropsInfo;
	DT_ROLE_PROPS_BLOB m_stPropsBlob;
    int m_iRoleTableNum;
    ROLESVRCFG* m_pstConfig;
};

