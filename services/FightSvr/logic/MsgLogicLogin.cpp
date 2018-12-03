#include "MsgLogicLogin.h"
#include "ObjectUpdatorMgr.h"
#include "LogMacros.h"
#include "ss_proto.h"
#include "strutil.h"
#include "../framework/FightSvrMsgLayer.h"
#include "../player/PlayerLogic.h"
#include "../player/PlayerMgr.h"
#include "../dungeon/DungeonMgr.h"
#include "../dungeon/Dungeon.h"
#include "../dungeon/DungeonLogic.h"

using namespace PKGMETA;

int FightLogin_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
	Player* poPlayer = NULL;
	CS_PKG_FIGHT_LOGIN_REQ& rstFightLoginReq = rstCsPkg.m_stBody.m_stFightLoginReq;
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_LOGIN_RSP;

	if (rstFightLoginReq.m_szAccountName[0] == '\0')
    {
		LOGERR("Player account name err.");
		m_stScPkg.m_stBody.m_stFightLoginRsp.m_nErrNo = ERR_WRONG_PARA;
		FightSvrMsgLayer::Instance().SendToClient(pstSession, &m_stScPkg, CONNSESSION_CMD_STOP);
		return -1;
	}

	PlayerMgr& roPlayerMgr = PlayerMgr::Instance();

	// ͬһsession�����ظ�����½��, ���������Ϊ
	poPlayer = roPlayerMgr.GetBySessionId(pstSession->m_dwSessionId);
	if (poPlayer != NULL)
    {
		LOGERR("Player account<%s> already existed!", poPlayer->m_szName);
		m_stScPkg.m_stBody.m_stFightLoginRsp.m_nErrNo = ERR_ALREADY_EXISTED;
		FightSvrMsgLayer::Instance().SendToClient(pstSession, &m_stScPkg, CONNSESSION_CMD_STOP);
		return -1;
	}

	Dungeon* poDungeon = DungeonMgr::Instance().GetById(rstFightLoginReq.m_dwDungeonId);
	if (poDungeon == NULL)
	{
		LOGERR("Player's dungeon not found.");
		m_stScPkg.m_stBody.m_stFightLoginRsp.m_nErrNo = ERR_NOT_FOUND;
		FightSvrMsgLayer::Instance().SendToClient(pstSession, &m_stScPkg, CONNSESSION_CMD_STOP);
		return -1;
	}

	poPlayer = roPlayerMgr.GetByUin(rstFightLoginReq.m_ullUin);
	if (poPlayer)
	{
		// �ѵ�¼��ֱ�Ӷ����ϵ�dungeon
		poPlayer->setDungeon(poDungeon);

		// ��Ҹ��»Ự��
		roPlayerMgr.DelFromSessionMap(poPlayer);

		poPlayer->m_stConnSession = *pstSession;
		poPlayer->m_stConnSession.m_chCmd = CONNSESSION_CMD_INPROC;
		poPlayer->setOnline(true);

		roPlayerMgr.AddToSessionMap(poPlayer);
	}
	else
	{
		poPlayer = roPlayerMgr.New();
		poPlayer->m_ullUin = rstFightLoginReq.m_ullUin;
		StrCpy(poPlayer->m_szName, rstFightLoginReq.m_szAccountName, MAX_NAME_LENGTH);

		// ����Ҹ�������
		poPlayer->setDungeon(poDungeon);

		// ��Ҹ��Ự��
		poPlayer->m_stConnSession = *pstSession;
		poPlayer->m_stConnSession.m_chCmd = CONNSESSION_CMD_INPROC;
		poPlayer->setOnline(true);

		// ��ʼPlayerMgrӳ���
		roPlayerMgr.AddIndex(poPlayer);
	}

    // ���ÿͻ�����߰汾��
    poPlayer->SetProtocolVersion(rstFightLoginReq.m_wVersion);

	// ���ص�¼�ɹ�
	m_stScPkg.m_stBody.m_stFightLoginRsp.m_nErrNo = ERR_NONE;
	FightSvrMsgLayer::Instance().SendToClient(&poPlayer->m_stConnSession, &m_stScPkg, poPlayer->GetProtocolVersion());

	LOGRUN("Player account<%s> login, uin=%lu", poPlayer->m_szName, poPlayer->m_ullUin);

	return 0;
}

int FightLogout_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
	Player* poPlayer = (Player*) pvPara;
	assert(poPlayer);

	PlayerLogic::Instance().PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_CONN);
	return 0;
}

// ս�����ؽ���
int FightLoadingProgress_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
	CS_PKG_FIGHT_LOADING_PROGRESS_NTF& rstFightLoadingNtf = rstCsPkg.m_stBody.m_stFightLoadingProgressNtf;

	Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
	if (NULL == poPlayer)
	{
		LOGERR("player not exsit");
		return -1;
	}

	Dungeon* poDungeon = poPlayer->getDungeon();
	if (NULL == poDungeon)
	{
		LOGERR("player not in dungeon");
		return -1;
	}

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_LOADING_PROGRESS_SYN;
	m_stScPkg.m_stBody.m_stFightLoadingProgressSyn.m_wLoadingProgress = rstFightLoadingNtf.m_wLoadingProgress;
	poDungeon->Broadcast(&m_stScPkg, poPlayer);

	return 0;
}

