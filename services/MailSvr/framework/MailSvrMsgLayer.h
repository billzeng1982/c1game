#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase_c.h"
#include "MsgLayerBase_c.h"
#include "MailSvrCfgDesc.h"

class MailSvrMsgLayer : public CMsgLayerBase_c, public TSingleton<MailSvrMsgLayer>
{
public:
	MailSvrMsgLayer();
	~MailSvrMsgLayer(){}

	bool Init();

	/*
		���ش���İ�����
		<0: error
	*/
	int DealPkg();

	IMsgBase_c* GetMsgHandler( int iMsgID );
	int SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg);
	int SendToMailDBSvr(PKGMETA::SSPKG& rstSsPkg);

    SSPKG& GetSsPkg() { return m_stSsPkg; }
private:
	void _RegistServerMsg();
	void _ForwardToWorkThread( MyTdrBuf* pstTdrBuf );

private:
	MAILSVRCFG*	m_pstConfig;
    SSPKG m_stSsPkg;
};

