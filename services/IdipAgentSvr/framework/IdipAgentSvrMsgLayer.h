#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "IdipAgentSvrCfgDesc.h"

class IdipAgentSvrMsgLayer : public CMsgLayerBase, public TSingleton<IdipAgentSvrMsgLayer>
{
public:
	IdipAgentSvrMsgLayer();
	virtual ~IdipAgentSvrMsgLayer(){}

	virtual bool Init();

	virtual int DealPkg();

	int SendToQuerySvr(PKGMETA::SSPKG& rstSsPkg);
	int SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg);
	int SendToRoleSvr(PKGMETA::SSPKG& rstSsPkg);
	IMsgBase* GetMsgHandler(int iMsgID);

private:
	virtual void _RegistServerMsg();
	void _ForwardToWorkThread(MyTdrBuf* pstTdrBuf);
	int _HandleWorkThreadRsp();

private:
	IDIPAGENTSVRCFG* m_pstConfig;
};

