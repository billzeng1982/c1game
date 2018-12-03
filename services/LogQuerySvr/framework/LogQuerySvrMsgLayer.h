#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "LogQuerySvrCfgDesc.h"

class LogQuerySvrMsgLayer : public CMsgLayerBase, public TSingleton<LogQuerySvrMsgLayer>
{
public:
	LogQuerySvrMsgLayer();
	virtual ~LogQuerySvrMsgLayer(){}

	virtual bool Init();

	virtual int DealPkg();

	int SendToIdipAgent(PKGMETA::SSPKG& rstSsPkg);

private:
	virtual void _RegistServerMsg();

private:
    LOGQUERYSVRCFG* m_pstConfig;
};



