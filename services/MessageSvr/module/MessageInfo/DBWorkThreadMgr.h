#pragma once

#include "define.h"
#include "singleton.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "LogMacros.h"
#include "../../cfg/MessageSvrCfgDesc.h"
#include "DBWorkThread.h"
using namespace PKGMETA;


class DBWorkThreadMgr : public TSingleton<DBWorkThreadMgr>
{
public:
	bool Init(MESSAGESVRCFG*	m_pstConfig);
	void SendReq(DT_MESSAGE_DB_REQ& rstDBReq);				//发送请求
	void HandleDBThreadRsp();									//处理收到的所有线程的回复.
    void Fini();
private:
	int m_iWorkThreadNum;
	CDBWorkThread* m_astWorkThreads;
	DT_MESSAGE_DB_RSP m_stDBRsp;
};

