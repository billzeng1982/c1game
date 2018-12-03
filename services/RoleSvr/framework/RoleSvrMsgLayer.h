#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "../cfg/RoleSvrCfgDesc.h"

class RoleSvrMsgLayer : public CMsgLayerBase, public TSingleton<RoleSvrMsgLayer>
{
public:
	RoleSvrMsgLayer();
	~RoleSvrMsgLayer(){}

	bool Init();

	/*
		返回处理的包数量
		<0: error
	*/
	int DealPkg();

	IMsgBase* GetMsgHandler( int iMsgID );

private:
	void _RegistServerMsg();
	void _ForwardToWorkThread( MyTdrBuf* pstTdrBuf );

private:
	ROLESVRCFG*	m_pstConfig;
	PKGMETA::SSPKG 	m_stSsRecvPkg;
};



