#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "../cfg/SdkDMSvrCfgDesc.h"

class SdkDMSvrMsgLayer : public CMsgLayerBase, public TSingleton<SdkDMSvrMsgLayer>
{
public:
	SdkDMSvrMsgLayer();
	~SdkDMSvrMsgLayer(){}

	bool Init();

	/*
		���ش���İ�����
		<0: error
	*/
	int DealPkg();
	int SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg);

private:
	void _RegistServerMsg();

private:
	SDKDMSVRCFG*	m_pstConfig;
	PKGMETA::SSPKG 	m_stSsRecvPkg;
};



