/*******************************************************************
 *  Copyright(c) 2015-2016 Company Raye
 *  All rights reserved.
 *
 *  文件名称: Player.h Player.cpp
 *  简要描述: 保存玩家的状态属性以及初始化，更新，改变接口
 ******************************************************************/
#pragma once

#include "define.h"
#include "macros.h"
#include "list.h"
#include "list_i.h"
#include "strutil.h"
#include "cs_proto.h"
#include "ss_proto.h"
#include "common_proto.h"
#include "comm_func.h"
#include "ov_res_public.h"
#include "PlayerData.h"
#include "ThreadQueue.h"

#define MAC_ADDR_LEN (128)
#define PHONE_TYPE_LEN (128)

using namespace PKGMETA;
using namespace std;

class Player
{
public:
    Player();
    ~Player(){}

    void Reset();
    void InitNewPlayer();
    bool InitFromDB(DT_ROLE_WHOLE_DATA& rstRoleWholeData);
    void OnUpdate();
    void UptRoleDataToDB();

private:
    bool _UpdateAfterInitFromDB();				// 玩家登陆从数据库读取数据后，需要更新的逻辑

public:
    int GetState() { return m_iCurState; }
    void SetState(int iState) { m_iCurState = iState;}
    bool IsCurState(int iState);

	//从SDk获取的玩家SdkUserName需要转化成游戏服务器保存的AccountName
	void SetAccountName(const char* pszSdkUserName) {
		char Key[MAX_NAME_LENGTH] = { 0 };
		snprintf(Key, MAX_NAME_LENGTH, "%s_%u", pszSdkUserName, m_dwLoginSvrId);
		Key[MAX_NAME_LENGTH - 1] = 0;
		StrCpy(m_szAccountName, Key, MAX_NAME_LENGTH);
	}
    char* GetAccountName() { return m_szAccountName; }

	void SetSdkUserName(const char* pszSdkUserName){ StrCpy(m_szSdkUserName, pszSdkUserName, MAX_NAME_LENGTH); }
	const char* GetSdkUserName() { return m_szSdkUserName; }

    void SetRoleName(const char* pszName)
    {
         StrCpy(m_oPlayerData.GetRoleBaseInfo().m_szRoleName, pszName, MAX_NAME_LENGTH);
         this->GetPlayerData().SetMajestyName(pszName);
    }
    char* GetRoleName() { return m_oPlayerData.GetRoleBaseInfo().m_szRoleName; }
    PlayerData& GetPlayerData() { return m_oPlayerData; }

    CONNSESSION* GetConnSession() { return &m_stConnSession; }
    void SetConnSession(const CONNSESSION* pstConnSession) { m_stConnSession = *pstConnSession; }
    void ClearConnSession() { bzero(&m_stConnSession, sizeof(m_stConnSession)); }
    uint32_t GetConnSessionID() { return m_stConnSession.m_dwSessionId; }

	char* GetMacAddr() {return m_szMacAddr; }
	void SetMacAddr(const char* pszMacAddr) { StrCpy(m_szMacAddr, pszMacAddr, MAX_NAME_LENGTH); }

    char* GetPhoneType() {return m_szPhoneType; }
    void SetPhoneType(const char* pszPhoneType) { StrCpy(m_szPhoneType, pszPhoneType, PHONE_TYPE_LEN); }

	char* GetOSType() {return m_szOSType; }
    void SetOSType(const char* pszOSType) { StrCpy(m_szOSType, pszOSType, MAX_NAME_LENGTH); }

    char* GetChannelName() {return m_szChannelName; }
    void SetChannelName(const char* pszChannelName) { StrCpy(m_szChannelName, pszChannelName, MAX_NAME_LENGTH); }

    char* GetSubChannelName() { return m_szSubChannelName; }
    void SetSubChannelName(const char* pszSubChannelName) { StrCpy(m_szSubChannelName, pszSubChannelName, MAX_NAME_LENGTH); }

    uint64_t GetUin() { return m_oPlayerData.GetRoleBaseInfo().m_ullUin; }
    void SetUin(uint64_t ullUin) { m_oPlayerData.GetRoleBaseInfo().m_ullUin = ullUin; m_oPlayerData.m_ullUin = ullUin;}
    uint8_t GetAccountType() { return m_bAccountType; }
    void SetAccountType(uint8_t bType) { m_bAccountType = bType;  }
    uint16_t GetRoleLv() { return m_oPlayerData.GetMajestyInfo().m_wLevel; }
	uint16_t GetRoleIconId() { return m_oPlayerData.GetMajestyInfo().m_wIconId; }
    uint8_t GetLoginType() { return m_bLoginType; }
    void SetLoginType(uint8_t bType) { m_bLoginType = bType;  }

	uint32_t GetLoginSvrId() { return m_dwLoginSvrId; }
	void SetLoginSvrId(uint32_t dwSvrId) { m_dwLoginSvrId = dwSvrId; }

    void SetLastUptDbTime(time_t llLastUptDbTime) { m_ullLastUptDbTime = (uint64_t)llLastUptDbTime; }
    uint64_t GetLastUptDbTime() { return m_ullLastUptDbTime; }

    void SetLastLoginTime(time_t llLastLoginTime) { m_oPlayerData.GetRoleBaseInfo().m_llLastLoginTime = llLastLoginTime; }
    uint32_t GetOnlineTokenId() { return m_dwOnlineTokenId; }
    void SetOnlineTokenId(uint32_t dwVal) { m_dwOnlineTokenId = dwVal;}

    uint32_t GetPkgSeqNo() { return m_dwPkgSeqNo; }
    void SetPkgSeqNo(uint32_t dwSeqNo) { m_dwPkgSeqNo = dwSeqNo; }

    time_t GetEntryReconnTime() { return m_llEntryReconnTime; }
    void SetEntryReconnTime(time_t llTime) { m_llEntryReconnTime = llTime; }

    void SetProtocolVersion(uint16_t wVersion) { m_wVersion = wVersion;}
    uint16_t GetProtocolVersion() { return m_wVersion; }
    int InitGuildMemInfo(DT_ONE_GUILD_MEMBER& GuildMemInfo);  //初始与公会无关的基本信息

	uint32_t GetLeaderValue() { return m_oPlayerData.m_dwLeaderValue; }
	void SetLeaderValue(uint32_t dwLeaderValue) { m_oPlayerData.m_dwLeaderValue = dwLeaderValue; }

    //PVP状态
    int GetFightState() { return m_iFightState; }
    void SetFightState(int iState) { m_iFightState = iState; }

    //缓存发送的包
    void SaveSendPkg(char* pszBuffer, uint32_t dwLen);

    //玩家登录后处理
    void AfterRoleLogin();

public:
    CONNSESSION m_stConnSession;
    CThreadQueue m_oSendBuffer;
    uint32_t m_dwRecvClientPkgSeq;

private:
    PlayerData m_oPlayerData;

    //当前状态
    int m_iCurState;

    char m_szAccountName[MAX_NAME_LENGTH];
	char m_szSdkUserName[MAX_NAME_LENGTH];
    // 进入重连状态的时间，等于0为无效时间
    time_t m_llEntryReconnTime;
	char m_szMacAddr[MAC_ADDR_LEN];				//客户端的MAC地址
    char m_szPhoneType[PHONE_TYPE_LEN];         //客户端手机型号
    char m_szChannelName[MAX_NAME_LENGTH];      //渠道号
    uint32_t m_dwOnlineTokenId;
    uint16_t m_wVersion;						// 客户端最高版本
    uint8_t m_bAccountType;
    uint8_t m_bLoginType;                       // 登录类型,与AccountType类型一致
	uint32_t m_dwLoginSvrId;                    // 登录的服务器Id
    uint32_t m_dwPkgSeqNo;
    uint64_t m_ullLastUptDbTime;
    int m_iFightState;                          //是否在战斗中
    char m_szSubChannelName[MAX_NAME_LENGTH];   //子渠道号
    char m_szOSType[MAX_NAME_LENGTH];      		//设备系统平台
};

