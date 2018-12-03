#pragma once

#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"

using namespace std;
using namespace PKGMETA;

class AP : public TSingleton<AP> 
{
public:
	AP();
	virtual ~AP();

	bool Init();
	bool CreatePlayerData(PlayerData* pstData);					// 创建新玩家数据
	bool InitPlayerData(PlayerData* pstData);						// 非初始化登陆调用
	void UpdatePlayerData(PlayerData* pstData); 					// 刷新玩家数据

	int PurchaseAp(PlayerData* pstData, uint32_t dwNum, SC_PKG_PURCHASE_RSP& rstScPkgBodyRsp);
	int HandleMajestLvUp(PlayerData* pstData);						// 主公升级增加经验
	void SendSynMsg(PlayerData* pstData);							// 推送同步消息

public:
	bool IsEnough(PlayerData* pstData, uint32_t dwValue);
	uint32_t GetAPCurValue(PlayerData* pstData);
	uint32_t Add(PlayerData* pstData, int32_t iValue);
	uint32_t AddNoLimit(PlayerData* pstData, int32_t iValue);
	
private:
	PKGMETA::SCPKG m_stScPkg;
	uint64_t m_ullUpdateInterval;
	
};

