#ifndef _CONN_UDP_H_
#define _CONN_UDP_H_

#include "CEpoll.h"
#include "LogMacros.h"
#include "CommBusLayer.h"
#include "mempool.h"
#include "NetUtil.h"
#include "UDPBlockPool.h"
#include "../ConnBase.h"
#include "UDPTimer.h"
#include "GameTimerMgr_PQ.h"
#include "UDPClientBuffIndex.h"

class CConnUDP : public CConnBase
{
public:
	CConnUDP(NETCONNCFG* pstConfig);
	virtual ~CConnUDP();

	static const int ACK_BUF_SIZE = 256;
    static const int ClOSE_BUF_SIZE = 256;

public:
	bool Create();
	bool CreateClientPool();
	int HandleNetIO();
	int HandleBusMsg();
	void LogStatsInfo();
	void Update(bool bIdle);

	static void ReleaseTimer(GameTimer* pTimer);

protected:
	void InitUDPParam();
	void InitPortList();
	void InitClientParam(SClient* pstClient, HANDLE iCltHnd, struct timeval* pstCurTime);
	bool AcceptConn();
	int NotifyClientConn(SClient* pstClient, uint16_t wPort);
	int OnReadable(SClient* pstClient);
	/*int OnWritable(SClient* pstClient);*/
	int HandleCltPkgForFilter(SClient* pstClient, uint16_t wMsgID);
	int HandleCltPkgForLogin(SClient* pstClient, uint16_t wMsgID);
	int HandleCltPkg(SClient* pstClient);
	int SendToLogicSvr(SClient* pstClient, char* pszPkg, char chSessCmd);
	int SendToLogicSvr(PKGMETA::CONNSESSION& rstSession, char* pszPkg);
	int SendToClient(SClient* pstClient, char* pszScPkg);
	int SendToClientWithoutAck(SClient* pstClient, char* pszScPkg);
	int SendAckToClient(SClient* pstClient);
	void DealLogicSvrPkg();
	void DealLogicSvrPkg(PKGMETA::CONNSESSION& rstSession, size_t uSessLen, MyTdrBuf* pstTdrBuf);
	void CloseClient(SClient* pstClient, char chStopReason, bool bNtfZoneSvr = true);
	void ConnStopNotifyLogicSvr(SClient* pstClient, char chStopReason);
	bool CheckCltPkgFreq(SClient* pstClient);
	int OnAccountLogin(SClient* pstClient);
	int OnReconnect(SClient* pstClient);
    void RTOEstimator(UDPBlock *pBlock, uint16_t wRTT);
    void ChgSendBufferRTO(UDPClientBuffIndex* pClientBuffIndex, uint16_t wNewRTO);
    void QuicklyResend(SClient* pstClient, uint16_t m_wCurRecvedSeq);
private:
	int _HandleAckPkg( SClient* pstClient, PKGMETA::COM_PKG_UDP_ACK_NTF& rstAckPkg );

private:
	NETCONNCFG* m_pstConfig;
	CEpoll m_oEpoll;
	SClient m_stCliListen;

	struct timeval m_tvConnTimePerSec;
	int m_iConnNumPerSec; 					// 每秒连接数
	CCommBusLayer m_oBusLayer;				// bus通信

	UDPBlockPool m_oUDPBlockPool;
	std::list<uint16_t> m_oPortFreeList;

	GameTimerMgr_PQ m_oUDPTimerMgr;
	CMemPool<SClient> m_oClientPool; 					// 存在于共享内存上
	CMemPool<SClient>::UsedIterator m_oClientPoolIter;		// update iterator;
	
	PKGMETA::CSPkg m_stCsPkg;
	PKGMETA::SCPkg m_stScPkg;

	MyTdrBuf m_stCsSendBuf;
	MyTdrBuf m_stScAckBuf; // 仅用做发送ack
	MyTdrBuf m_stScCloseBuf; //CloseClient用

	SLogicSvrConnStatsInfo m_stProcStatsInfo;

	PKGMETA::CONNSESSION m_stSessionToSvr;
	PKGMETA::SCPKG m_stScPKGNotifyConn;
	MyTdrBuf m_stTdrBufListen;

	UDPInitParam m_stListenParam;
	UDPInitParam m_stClientParam;
};

#endif

