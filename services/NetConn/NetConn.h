#ifndef _NET_CONN_H_
#define _NET_CONN_H_

#include "CEpoll.h"
#include "cfg/NetConnCfgDesc.h"
#include "LogMacros.h"
#include "AppFrame.h"

class CConnBase;
class CNetConn : public CAppFrame
{
public:
	CNetConn();
	virtual ~CNetConn();

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual int GetProcID() { return m_stConfig.m_iProcID; }
	
protected:
	virtual void _SetAppCfg();
	virtual bool _ReadCfg();

public:
	NETCONNCFG& GetConfig() { return m_stConfig; }
	
private:
	NETCONNCFG m_stConfig;
	CConnBase* m_poConnCom;
};

#endif

