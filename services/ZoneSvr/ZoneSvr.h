#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/ZoneSvrCfgDesc.h"
#include "mng/GameTimerMgr_PQ.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"
#include "UdpRelay.h"

using namespace PKGMETA;

class ZoneSvr: public CAppFrame, public TSingleton<ZoneSvr> {
public:
	ZoneSvr();
	virtual ~ZoneSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();

	virtual int GetProcID() { return m_stConfig.m_iProcID; }
	ZONESVRCFG& GetConfig() { return m_stConfig; }

	static void ReleaseTimer(GameTimer* pTimer);

private:
	virtual void _SetAppCfg();
	virtual bool _ReadCfg();
	void _ReadInfoFromFile();
	void _WriteInfoToFile();
	void _NotifyClusterGate(uint8_t bOnline);
    void _NotifyDirSvr(uint8_t bOnline);
    void _InitSvrInfo();

public:
	GameTimerMgr_PQ m_oLogicTimerMgr;
	uint64_t m_ullSvrOfflineLastTime;	// 最后一次关闭的时间戳ms

private:
	static const int m_iLogicTimerMaxNum = 256;	// 用于server逻辑更新数据
	PKGMETA::SSPKG m_stSsPkg;
	uint32_t m_dwIntervalPass;
	uint64_t m_ullLastUptTime;
	ZONESVRCFG m_stConfig;
    UdpRelayClt m_oUdpRelayClt;
    DT_SERVER_INFO m_stServerInfo;
    MyTdrBuf m_stSvrInfoSendBuf;
};

