#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "../cfg/ClusterDBSvrCfgDesc.h"
#include "../thread/DBWorkThread.h"
class ClusterDBSvrMsgLayer : public CMsgLayerBase, public TSingleton<ClusterDBSvrMsgLayer>
{
public:
	ClusterDBSvrMsgLayer();
	virtual ~ClusterDBSvrMsgLayer(){}

	virtual bool Init();
    bool Fini();

	virtual int DealPkg();
    void _ForwardToWorkThread(MyTdrBuf* pstTdrBuf);
    CDBWorkThread* GetWorkThread(int iPos) { return &m_astWorkThreads[iPos]; } //调用者负责合法性检查

private:
	CLUSTERDBSVRCFG* m_pstConfig;
    CDBWorkThread* m_astWorkThreads;
    int32_t m_iThreadHandlePkgNum;
};

