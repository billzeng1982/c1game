#pragma once
#include <stdarg.h>
#include "define.h"
#include "ss_proto.h"
#include "singleton.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../gamedata/GameDataMgr.h"
#include "player/Player.h"
#include "player/PlayerMgr.h"
#include "player/PlayerData.h"

using namespace PKGMETA;

class Message : public TSingleton<Message>
{
public:
    Message() {};
    virtual ~Message(){};
public:
    bool Init();
    int Send(PlayerData* pstData, CS_PKG_MESSAGE_SEND_REQ& rstCsPkgBodyReq);
    int InitOneRecord(PlayerData* pstData, uint8_t bChannel, DT_MESSAGE_ONE_RECORD_INFO& rstRecord);
    int GetBox(PlayerData* pstData, CS_PKG_MESSAGE_GET_BOX_REQ& rstPkgBodyReq);
    int DelPrivateBox(uint64_t ullPlayerUin);
	int MessageSendCheck(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint8_t bChannel);
	//检查时间聊天发送条件
    int IsSendWorldMessageOk(PlayerData* pstData);     

    void GetWorldBox(DT_MESSAGE_WORLD_BOX_INFO& rstMessageWorld) { memcpy(&rstMessageWorld, &m_MessageWorld, sizeof(DT_MESSAGE_WORLD_BOX_INFO)); }
    void GetSysBox(DT_MESSAGE_SYS_BOX_INFO& rstMessageSys) { memcpy(&rstMessageSys, &m_MessageSys, sizeof(DT_MESSAGE_SYS_BOX_INFO)); }

    /*
        自动发送消息说明:
        DT_MESSAGE_ONE_RECORD_INFO结构中的m_dwRecordType
         =>1000为ResMessage中的ID,Record为相应参数|0#表示Record为普通聊天字符串
         Record参数格式为 "Key1=Value2|Key2=Value2"
         Key
            --需要前台特殊处理的Key
            GCardId#武将Id,查表显示武将名字
            PropId#道具Id,查表显示
            ----其他Key随意取,能帮助理解就行
            Name#

    */
    void AutoSendWorldMessage(PlayerData* pstData, uint32_t dwMsgId, const char* pszFmt = "", ...);
    void AutoSendSysMessage(uint32_t dwMsgId, const char* pszFmt = "", ...);
private:
    int _SendPrivateMessage(PlayerData* pstData, DT_MESSAGE_ONE_RECORD_INFO& rstRecord);    //发送私聊
    int _SendGuildMessage(PlayerData* pstData, DT_MESSAGE_ONE_RECORD_INFO& rstRecord);      //发送公会聊天
    int _SendWorldMessage(PlayerData* pstData, DT_MESSAGE_ONE_RECORD_INFO& rstRecord, bool bIsAutoSend = false, bool bIsHorn = false);      //世界聊天
    int _SendSysMessage(DT_MESSAGE_ONE_RECORD_INFO& rstRecord);        //系统消息
    
private:
    SCPKG m_stScPkg;
    SSPKG m_stSsPkg;
    DT_MESSAGE_WORLD_BOX_INFO m_MessageWorld;
    DT_MESSAGE_SYS_BOX_INFO m_MessageSys;
    uint32_t m_dwWordCD;			//世界频道冷却时间
	uint32_t m_dwWordLvLimit;		//世界频道等级限制
    DT_MESSAGE_ONE_RECORD_INFO m_stRecord;
};

