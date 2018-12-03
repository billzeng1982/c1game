#pragma once

#include "redis/RedisSyncHandler.h"
#include "common_proto.h"
#include <string.h>
#include <stdio.h>
#include "strutil.h"
#include "../cfg/SerialNumSvrCfgDesc.h"
#include "../logic/SerialNumTable.h"
#include "ThreadFrame.h"

using namespace PKGMETA;

class RedisWorkThread : public CThreadFrame
{
public:
	RedisWorkThread();
	~RedisWorkThread(){}

public:
	void SendPkg(SSPKG& rstSsPkg);
	SerialNumTable* GetSerialNumTable() { return &m_stSerialTable; }
	SSPKG& GetSendSsPkg() { return m_stSsSendPkg; }

protected:
	virtual bool _ThreadAppInit(void* pvAppData);
	virtual void _ThreadAppFini();
	virtual int _ThreadAppProc();
	int _HandleSvrMsg();
	int _CheckRedisConn();

protected:
	SSPKG				m_stSsRecvPkg;
	SSPKG				m_stSsSendPkg;
	RedisSyncHandler	m_oRedisHandler;
	SERIALNUMSVRCFG*	m_pstConfig;
	SerialNumTable		m_stSerialTable;
	MyTdrBuf			m_stSendBuf;
	time_t				m_tLastPingTime;
};
