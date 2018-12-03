#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "../cfg/MineDBSvrCfgDesc.h"
#include "../thread/DBWorkThread.h"
class MineDBSvrMsgLayer : public CMsgLayerBase, public TSingleton<MineDBSvrMsgLayer>
{
public:
	MineDBSvrMsgLayer();
	virtual ~MineDBSvrMsgLayer(){}

	virtual bool Init();
    bool Fini();

	virtual int DealPkg();
    void _ForwardToWorkThread(MyTdrBuf* pstTdrBuf);
    CDBWorkThread* GetWorkThread(int iPos) { return &m_astWorkThreads[iPos]; } //调用者负责合法性检查

private:
	MINEDBSVRCFG* m_pstConfig;
    CDBWorkThread* m_astWorkThreads;
    uint32_t m_iThreadHandlePkgNum;
};

