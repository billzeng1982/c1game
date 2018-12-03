#pragma once

// 多线程版本
#include "MsgLayerBase_r.h"

class MailDBSvrMsgLayer_r : public MsgLayerBase_r
{
public:
	MailDBSvrMsgLayer_r(){}
	virtual ~MailDBSvrMsgLayer_r() {}

	virtual bool Init();

protected:
	virtual void _RegistServerMsg();
};


