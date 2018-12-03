#pragma once

#include "singleton.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "cs_proto.h"
#include "../player/Player.h"

class FightSvrMsgLayer : public CMsgLayerBase, public TSingleton<FightSvrMsgLayer>
{
public:
	FightSvrMsgLayer();
	~FightSvrMsgLayer(){}

	virtual bool Init();
	virtual int DealPkg();

	int SendToClient( Player* poPlayer, PKGMETA::SCPKG* pstScPkg, char cSessCmd = PKGMETA::CONNSESSION_CMD_INPROC );
	int SendToClient( const PKGMETA::CONNSESSION* pstConnSess, PKGMETA::SCPKG* pstScPkg, uint16_t wVersion, char cSessCmd = PKGMETA::CONNSESSION_CMD_INPROC);
	int SendToClientWithoutAck( Player* poPlayer, PKGMETA::SCPKG* pstScPkg, char cSessCmd = PKGMETA::CONNSESSION_CMD_INPROC );
	int SendToClientWithoutAck( const PKGMETA::CONNSESSION* pstConnSess, PKGMETA::SCPKG* pstScPkg, uint16_t wVersion, char cSessCmd = PKGMETA::CONNSESSION_CMD_INPROC);
	int SendToClusterGate(PKGMETA::SSPKG& rstSsPkg);
	int SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg);
protected:
	virtual void _RegistClientMsg();
	virtual void _RegistServerMsg();

	void _DealConnPkg();

private:
	int m_iConnID;
};

