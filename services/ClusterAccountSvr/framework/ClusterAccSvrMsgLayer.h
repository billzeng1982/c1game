#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "../cfg/ClusterAccountSvrCfgDesc.h"
#include "../http/HttpControler.h"

class ClusterAccSvrMsgLayer : public CMsgLayerBase, public TSingleton<ClusterAccSvrMsgLayer>
{
public:
	ClusterAccSvrMsgLayer();
	~ClusterAccSvrMsgLayer(){}

	bool Init();

	/*
		返回处理的包数量
		<0: error
	*/
	int DealPkg();

	IMsgBase* GetMsgHandler( int iMsgID );

	bool ForwardToWorkThread( MyTdrBuf* pstTdrBuf );

private:
	CLUSTERACCOUNTSVRCFG*	m_pstConfig;
	PKGMETA::SSPKG 		m_stSsRecvPkg;

	HttpControler			m_oHttpControler;
};


