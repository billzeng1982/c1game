#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "../cfg/SerialNumSvrCfgDesc.h"

class SerialNumSvrMsgLayer : public CMsgLayerBase, public TSingleton<SerialNumSvrMsgLayer>
{
public:
	SerialNumSvrMsgLayer();
	~SerialNumSvrMsgLayer(){}

	bool Init();

	/*
		返回处理的包数量
		<0: error
	*/
	int DealPkg();
	IMsgBase* GetMsgHandler(int iMsgID);

private:
	void _RegistServerMsg();
	void _ForwardToWorkThread(MyTdrBuf* pstTdrBuf);

private:
	SERIALNUMSVRCFG*  m_pstConfig;
	PKGMETA::SSPKG 	m_stSsRecvPkg;
	int m_iCursor;
};



