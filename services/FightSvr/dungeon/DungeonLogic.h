#pragma once

#include "singleton.h"
#include "cs_proto.h"
#include "ss_proto.h"
#include "Dungeon.h"

class DungeonLogic : public TSingleton<DungeonLogic>
{
public:
	DungeonLogic() {}
	~DungeonLogic() {}

public:
	// 同步战场时间
	void SyncDungeonTime(Dungeon* poDungeon);

	// 对时协议
	void SyncServerTime(Dungeon* poDungeon);

	//发送选人开始消息
	void SendChooseStart(Dungeon* poDungeon);

	//发送回合开始消息
	void SendTurnStart(Dungeon* poDungeon);

	//选择回合结束,切换到下一回合
	void ChgToNextTurn(Dungeon* poDungeon);

	//发送战斗开始消息(客户端收到此消息后开始加载资源，初始化战场副本)
	void SendFightStart(Dungeon* poDungeon);

	//发送副本开始消息(双方客户端加载完成，正式开战后发送此消息)
	void SendDungeonStart(Dungeon* poDungeon, int16_t nErrNo);

	// 发送死亡消息
	void SendTroopDead(Dungeon* poDungeon, Troop* poDeadTroop, FightObj* poSource, uint8_t bDeadDelay);

	// 发送战斗结算消息
	void SendFightSettle(Dungeon* poDungeon, int8_t chWinGroup, int8_t chEndReason);

	// 发送单挑结算消息
	void SendSoloSettle(Dungeon* poDungeon);

	// 处理加载过程中，掉线的情况
	void HandleFightOffLine(Dungeon* poDungeon);

	// 处理投降消息
	void HandleFightSurrender(Dungeon* poDungeon, int8_t chSurrenderGroup);

private:
	PKGMETA::SCPKG m_stScPkg;
	PKGMETA::SSPKG m_stSsPkg;
};

// Dungeon callback
void UpdateDungeon(IObject* pObj, int iDeltaTime);

