#pragma once

#include "common_proto.h"
#include "singleton.h"
#include "ss_proto.h"
#include "RoleTable.h"
#include <vector>
#include "../thread/DBWorkThread.h"

using namespace std;
using namespace PKGMETA;

#define CMD_ADD_PROP				"add_prop"
#define CMD_SET_LV					"set_lv"
#define CMD_SET_EXP					"set_exp"
#define CMD_SET_AP					"set_ap"
#define CMD_BLACK_ROOM              "black_room"
#define CMD_GET_UIN                 "get_uin"
#define CMDG_GET_ROLE_NAME          "get_role_name"


class GmMgr
{
public:
    GmMgr() {}
    virtual ~GmMgr() {}
    bool Init();

    int HandleMsgCmd(CDBWorkThread* poWorkThread, SSPKG& stSsReqPkg);
    int ParseCmd(char* pszCmd, char const* pszDelims = " ");

    //  Gm
    int GmSetLv(uint64_t ullUin);          //设置等级
    int GmSetExp(uint64_t ullUin);         //设置经验
    int GmSetAp(uint64_t ullUin);          //设置体力
    int GmBlackRoom(uint64_t ullUin);      //设置小黑屋
    int GmGetUin(CDBWorkThread* poWorkThread, SSPKG& stSsReqPkg);         //获取Uin
    int GmGetRoleName(CDBWorkThread* poWorkThread, SSPKG& stSsReqPkg);

private:
    vector<char*> m_ArgsMap;
    char pszCmd[20];
    DT_ROLE_MAJESTY_BLOB m_stMajestyBlob;
    DT_ROLE_MAJESTY_INFO m_stMajestyInfo;
   
    RoleTable* m_poRoleTable;
    PKGMETA::SSPKG 	m_stSsPkg;
};


