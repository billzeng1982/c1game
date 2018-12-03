#ifndef _CONN_TCP_H_
#define _CONN_TCP_H_

#include "CEpoll.h"
#include "LogMacros.h"
#include "CommBusLayer.h"
#include "mempool.h"
#include "NetUtil.h"
#include "../ConnBase.h"
#include "cs_proto.h"
#include "../framework/MsgBroadcaster.h"

class CConnTCP : public CConnBase
{
public:
	CConnTCP(NETCONNCFG* pstConfig);
	virtual ~CConnTCP(){}

public:
	bool Create();
	int HandleNetIO();
	int HandleBusMsg();
	void LogStatsInfo(){}
	void Update(bool bIdle);

	CMemPool<SClient>* GetClientPool() { return &m_oClientPool; }

	int SendToClient(SClient* pstClient, char* pszScPkg, bool bCloseAftSnd = false);

protected:
    bool CreateClientPool();
	void InitClientParam(SClient* pstClient, HANDLE iCltHnd, struct timeval* pstCurTime);
	bool AcceptConn();
	int OnReadable(SClient* pstClient);
	int OnWritable(SClient* pstClient);
	int HandleCltPkg(SClient* pstClient);
	int SendToLogicSvr(SClient* pstClient, char* pszPkg, char chSessCmd);
	int SendToLogicSvr(PKGMETA::CONNSESSION& rstSession, char* pszPkg);
	void DealLogicSvrPkg();
	void DealLogicSvrPkg(PKGMETA::CONNSESSION& rstSession, size_t uSessLen, MyTdrBuf* pstTdrBuf);
	void CloseClient(SClient* pstClient, char chStopReason, bool bNtfZoneSvr = true);
	void ConnStopNotifyLogicSvr(SClient* pstClient, char chStopReason);
	bool CheckCltPkgFreq(SClient* pstClient);
	int OnAccountLogin(SClient* pstClient);

    int GenEncryptKey(SClient* pstClient, IN char* pszScPkg, OUT char* pszBuff);
    int EncryptPkg(SClient* pstClient, IN char* pszScPkg, OUT char* pszBuff);
    int Decrypt(SClient* pstClient);
    int CachePkg(SClient* pstClient, char* pszScPkg, int iPkgLen);

	// not used
	int OnReconnect(SClient* pstClient);

private:
	NETCONNCFG* m_pstConfig;
	CEpoll m_oEpoll;
	SClient m_stListen;

    uint64_t m_ullLastUptTimeMs;

	struct timeval m_tvConnTimePerSec;
	int m_iConnNumPerSec; 						// 每秒连接数
	CCommBusLayer m_oBusLayer;					// bus通信
	CMemPool<SBuffBlock> m_oBuffBlockPool; 		// 存在于堆空间上
	CMemPool<SClient> m_oClientPool; 			// 存在于共享内存上
	CMemPool<SClient>::UsedIterator m_oClientUptIter;		// update iterator;
	MyTdrBuf m_stCsSendBuf; 					// [sizeof(CSPKG)*2]
	SLogicSvrConnStatsInfo m_stProcStatsInfo;
	PKGMETA::SCPkg m_stScPkg;
	MyTdrBuf m_stScSendBuf;

	MsgBroadcaster m_oMsgBroadcaster;
};

#endif
