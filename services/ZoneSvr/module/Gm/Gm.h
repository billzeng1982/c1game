#pragma once

#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "../player/PlayerData.h"
#include <vector>
#include "GmFile.h"

using namespace std;
using namespace PKGMETA;

#define CMD_ADD_ITEM				"add_item"
#define CMD_SET_LV					"set_lv"
#define CMD_SET_EXP					"set_exp"
#define CMD_SET_AP					"set_ap"
#define CMD_BAN_ACCOUNT				"ban_account"
#define CMD_BLACK_ROOM              "black_room"
#define CMD_SET_GUILD_LV            "set_guild_lv"
#define CMD_SET_GUILD_FUND          "set_guild_fund"
#define CMD_SET_ACT                 "set_act"
#define CMD_GET_UIN                 "get_uin"
#define CMDG_GET_ROLE_NAME          "get_role_name"


#define CMD_ADD_ITEM_NUM 1
#define CMD_DEL_ITEM_NUM 2

#define GM_MAX_EXP  9999
#define GM_MAX_AP   999
#define GM_ACT_ON   1
#define GM_ACT_OFF  0


class GmMgr : public TSingleton<GmMgr>
{

public:
    GmMgr() {};
    virtual ~GmMgr() {}
    bool Init();
    void Fini();

	//GM工具的调试功能
    int HandleSsMsg(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp);
    int HandleMsgCmd(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp);
    int HandleMsgFunction(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp);
    int HandleMsgSendMail(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp);
    int ParseCmd(char* pszCmd, char const* pszDelims = " ");
    int HandleMsgMarqueeNotify(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp);
    int HandleMsgMarqueeOnTimeAdd(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp);
    int HandleMsgMarqueeOnTimeDel(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp);
    int HandleMsgGetMarqueeList(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp);
    int HandleMsgKickPlayer(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp);

	//客户端的GM调试功能
	int HandleCsMsg(PlayerData* pstData, CS_PKG_GM_REQ& rstGmReq, SC_PKG_GM_RSP& rstGmRsp);
    int HandleMsgDebug(PlayerData* pstData, CS_PKG_GM_REQ& rstGmReq, SC_PKG_GM_RSP& rstGmRsp);
    int HandleMsgPveDebug(PlayerData* pstData, CS_PKG_GM_REQ& rstGmReq, SC_PKG_GM_RSP& rstGmRsp);
    int HandleMsgMajestyLvUp(PlayerData* pstData, CS_PKG_GM_REQ& rstGmReq, SC_PKG_GM_RSP& rstGmRsp);
    int HandleMsgUltimateSkillPoint(PlayerData* pstData, CS_PKG_GM_REQ& rstGmReq, SC_PKG_GM_RSP& rstGmRsp);
    //Gm
    void GmCmdTest(PlayerData* pstData, char* pszCmd);
    int GmAddItem();

	//批量踢人
	void HanldeMsgMultKick(SS_PKG_GM_MULT_KICK_REQ& rstGmReq, SS_PKG_GM_MULT_KICK_RSP& rstGmRsp);


    int GmSetLv();
    int GmSetLv(uint64_t ullUin, char* pszRoleName, uint64_t ullLv);

    int GmSetExp();
    int GmSetAp();

    int GmBanAccount();
    int GmBanAccount(uint64_t ullUin, uint64_t ullTime);

    int GmBlackRoom();
    int GmBlackRoom(uint64_t ullUin, char* pszName, uint64_t ullTime);
    int GmSetGuildLv();
    int GmSetGuildFund();
    int GmOnAct();

    int GmSendOrder();
    int GmGetUin(SS_PKG_GM_RSP& rstGmRsp);
    int GmGetRoleName(SS_PKG_GM_RSP& rstGmRsp);

    int GmRecharge(uint64_t ullUin, uint64_t ullProductID);
	
	void SetSenderUin(uint64_t ullUin) { m_ullSenderUin = ullUin; }
/*int*/
private:
    void _AddDataForDebugAll(PlayerData* pstData);

private:
  	SCPKG m_stScPkg;
    SSPKG m_stSsPkg;
    vector<char*> m_ArgsMap;
    char m_CmdBuff[MAX_GM_CMD_NUM];
	uint64_t m_ullSenderUin;
};