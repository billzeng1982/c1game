#pragma once

#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase_c.h"
#include "DynMempool.h"
#include "coroutine.h"

using namespace PKGMETA;

class CMsgLayerBase_c;

struct CoArgs_SS
{
    IMsgBase_c* m_oMsgBase;
    SSPKG* m_pstSsPkg;
    CMsgLayerBase_c* m_pstMsgLayer;
};

class CMsgLayerBase_c
{
 private:
    const static int MAX_COROUTINE_NUM = 100;
    const static int SSPKG_INIT_NUM = 20;
    const static int SSPKG_DELTA_NUM = 10;

public:
	CMsgLayerBase_c(size_t uSsSize);
	virtual ~CMsgLayerBase_c(){}

	IMsgBase_c* GetServerMsgHandler(int iMsgId);

	int SendToServer(int iAddr, SSPKG& rstSsPkg);
	int SendToServer(int iAddr, MyTdrBuf* pstTdrBuf);

	virtual int DealPkg() = 0;
	virtual bool Init() = 0;

	SSPKG* UnpackSsPkg( MyTdrBuf* pstPkgBuf );
	MyTdrBuf* GetSsSendBuf() { return &m_stSsSendBuf; }

	bool RefreshBusChannel() { return m_oCommBusLayer.RefreshHandle(); }
	CCommBusLayer& GetCommBusLayer() { return m_oCommBusLayer; }
	void DealSvrPkg(SSPKG& rstSsPkg);

    CoroutineEnv* GetCoroutineEnv() { return &m_oCoEnv; }

    void ReleaseSsPkg(SSPKG* pstSsPkg) { m_oSsPkgPool.Release(pstSsPkg); }

protected:
	virtual void _RegistServerMsg(){}

	bool _Init(int iBusCGIMKey, int iProcID);

    //Used to Deal Msg
	void _DealSvrPkg();

protected:
	MsgHandlerMap_c_t m_oSsMsgHandlerMap;

    CCommBusLayer m_oCommBusLayer;

	SSPKG m_stSsRecvPkg;
	MyTdrBuf m_stSsSendBuf;

	int m_iProcId;
	int m_iBusGCIMKey;

    CoroutineEnv m_oCoEnv;

    CoArgs_SS m_oCoArgs;

    DynMempool<SSPKG> m_oSsPkgPool;
};