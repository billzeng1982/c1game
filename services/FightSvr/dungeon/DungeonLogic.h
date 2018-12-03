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
	// ͬ��ս��ʱ��
	void SyncDungeonTime(Dungeon* poDungeon);

	// ��ʱЭ��
	void SyncServerTime(Dungeon* poDungeon);

	//����ѡ�˿�ʼ��Ϣ
	void SendChooseStart(Dungeon* poDungeon);

	//���ͻغϿ�ʼ��Ϣ
	void SendTurnStart(Dungeon* poDungeon);

	//ѡ��غϽ���,�л�����һ�غ�
	void ChgToNextTurn(Dungeon* poDungeon);

	//����ս����ʼ��Ϣ(�ͻ����յ�����Ϣ��ʼ������Դ����ʼ��ս������)
	void SendFightStart(Dungeon* poDungeon);

	//���͸�����ʼ��Ϣ(˫���ͻ��˼�����ɣ���ʽ��ս���ʹ���Ϣ)
	void SendDungeonStart(Dungeon* poDungeon, int16_t nErrNo);

	// ����������Ϣ
	void SendTroopDead(Dungeon* poDungeon, Troop* poDeadTroop, FightObj* poSource, uint8_t bDeadDelay);

	// ����ս��������Ϣ
	void SendFightSettle(Dungeon* poDungeon, int8_t chWinGroup, int8_t chEndReason);

	// ���͵���������Ϣ
	void SendSoloSettle(Dungeon* poDungeon);

	// ������ع����У����ߵ����
	void HandleFightOffLine(Dungeon* poDungeon);

	// ����Ͷ����Ϣ
	void HandleFightSurrender(Dungeon* poDungeon, int8_t chSurrenderGroup);

private:
	PKGMETA::SCPKG m_stScPkg;
	PKGMETA::SSPKG m_stSsPkg;
};

// Dungeon callback
void UpdateDungeon(IObject* pObj, int iDeltaTime);

