#ifndef _CONN_BASE_H_
#define _CONN_BASE_H_

#include "cfg/NetConnCfgDesc.h"
#include "ConnUtil.h"
#include "MyTdrBuf.h"

//#define CONN_CLIENT_CHK_FREQ 500 //ms
#define CONN_CLIENT_CHK_FREQ 20 //ms

class CConnBase
{
public:
	CConnBase(){}
	virtual ~CConnBase(){}

public:
	virtual bool Create(){ return false; }
	virtual bool CreateClientPool(){ return false; }
	virtual int HandleNetIO(){ return 0; }
	virtual int HandleBusMsg(){ return 0; }
	virtual void LogStatsInfo(){}
	virtual void Update(bool bIdle){}
	
protected:	
	virtual bool AcceptConn(){ return false; }
	virtual int OnReadable(SClient* pstClient){ return 0; }
	virtual int OnWritable(SClient* pstClient){ return 0; }
	virtual int HandleCltPkg(SClient* pstClient){ return 0; }
	virtual int SendToLogicSvr(SClient* pstClient, char* pszPkg, char chSessCmd){ return 0; }
	virtual int SendToLogicSvr(PKGMETA::CONNSESSION& rstSession, char* pszPkg){ return 0; }
	//virtual int SendToClient(SClient* pstClient, char* pszScPkg, bool bCloseAftSnd = false){ return 0; }
	virtual void DealLogicSvrPkg(){}
	virtual void DealLogicSvrPkg(PKGMETA::CONNSESSION& rstSession, size_t uSessLen, MyTdrBuf* pstTdrBuf){}
	virtual void CloseClient(SClient* pstClient, char chStopReason, bool bNtfZoneSvr = true){}
	virtual void ConnStopNotifyLogicSvr(SClient* pstClient, char chStopReason){}
	virtual bool CheckCltPkgFreq(SClient* pstClient){ return false; }
	virtual int OnAccountLogin(SClient* pstClient){ return 0; }
	virtual int OnReconnect(SClient* pstClient){ return 0; }
};

#endif


