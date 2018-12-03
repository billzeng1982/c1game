#pragma once

/* 分片广播消息给所有玩家
*/
#include "ThreadQueue.h"
#include "ConnUtil.h"
#include "mempool.h"

class CConnTCP;

class MsgBroadcaster
{
	const static int BROADCAST_MSG_BUF_SIZE = 262144;

public:
	MsgBroadcaster()
	{
		m_poConnTcp = NULL;
	}
	~MsgBroadcaster(){}
	
	bool Init( CConnTCP* poConnTcp );
	void Update( bool bIdle );

	bool AddMsg( char* pMsgBuf, uint32_t dwMsgLen );

private:
	CThreadQueue m_oMsgQueue;
	CMemPool<SClient>::UsedIterator m_oClientIter;
	CConnTCP* m_poConnTcp;
};

