#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "../cfg/MessageSvrCfgDesc.h"

class MessageSvrMsgLayer : public CMsgLayerBase, public TSingleton<MessageSvrMsgLayer>
{
public:
	MessageSvrMsgLayer();
	~MessageSvrMsgLayer(){}

	bool Init();

	/*
		返回处理的包数量
		<0: error
	*/
	int DealPkg();
	int SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg);

private:
	void _RegistServerMsg();

private:
	MESSAGESVRCFG*	m_pstConfig;
	PKGMETA::SSPKG 	m_stSsRecvPkg;
};



