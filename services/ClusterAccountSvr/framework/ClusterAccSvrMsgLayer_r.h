#pragma once

// ���̰߳汾

#include "MsgLayerBase_r.h"

class ClusterAccSvrMsgLayer_r : public MsgLayerBase_r
{
public:
	ClusterAccSvrMsgLayer_r(){}
	virtual ~ClusterAccSvrMsgLayer_r() {}

	virtual bool Init();

protected:
	virtual void _RegistServerMsg();
};


