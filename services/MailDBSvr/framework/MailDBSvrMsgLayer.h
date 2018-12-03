#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "../cfg/MailDBSvrCfgDesc.h"

class MailDBSvrMsgLayer : public CMsgLayerBase, public TSingleton<MailDBSvrMsgLayer>
{
public:
	MailDBSvrMsgLayer();
	~MailDBSvrMsgLayer(){}

	bool Init();

	/*
		返回处理的包数量
		<0: error
	*/
	int DealPkg();

	IMsgBase* GetMsgHandler( int iMsgID );
	int SendToMailSvr(PKGMETA::SSPKG& rstSsPkg);
	
private:
	void _RegistServerMsg();
	void _ForwardToWorkThread( MyTdrBuf* pstTdrBuf );

private:
	MAILDBSVRCFG*	m_pstConfig;
	PKGMETA::SSPKG 	m_stSsRecvPkg;
};

