#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "../cfg/FriendSvrCfgDesc.h"

class FriendSvrMsgLayer : public CMsgLayerBase, public TSingleton<FriendSvrMsgLayer>
{
public:
	FriendSvrMsgLayer();
	~FriendSvrMsgLayer(){}

	bool Init();

	/*
		返回处理的包数量
		<0: error
	*/
	int DealPkg();

	IMsgBase* GetMsgHandler( int iMsgID );
	int SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg);
	
private:
	void _RegistServerMsg();
	int _GetZoneSvrProcId() { return m_pstConfig->m_iZoneSvrID; }

private:
	FRIENDSVRCFG*	m_pstConfig;
};

