#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "../cfg/AsyncPvpSvrCfgDesc.h"

class AsyncPvpSvrMsgLayer : public CMsgLayerBase, public TSingleton<AsyncPvpSvrMsgLayer>
{
public:
	AsyncPvpSvrMsgLayer();
	~AsyncPvpSvrMsgLayer(){}

	bool Init();

	/*
		���ش���İ�����
		<0: error
	*/
	int DealPkg();

	IMsgBase* GetMsgHandler( int iMsgID );
	int SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToMailSvr(PKGMETA::SSPKG& rstSsPkg);

private:
	void _RegistServerMsg();

private:
	ASYNCPVPSVRCFG*	m_pstConfig;
};

