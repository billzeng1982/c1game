#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "../cfg/AccountSvrCfgDesc.h"

class AccountSvrMsgLayer : public CMsgLayerBase, public TSingleton<AccountSvrMsgLayer>
{
public:
	AccountSvrMsgLayer();
	~AccountSvrMsgLayer(){}

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
    uint64_t _CreateUin(uint32_t dwSvrId);
    uint64_t m_ullLastCreateUinTime;
    uint32_t m_dwSeq;
private:
	ACCOUNTSVRCFG*	m_pstConfig;
	PKGMETA::SSPKG 	m_stSsRecvPkg;
};



