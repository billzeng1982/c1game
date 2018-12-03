#pragma once

// 多线程版本
#include "MsgBase_r.h"
#include "../thread/ThreadQueue.h"

class MsgLayerBase_r
{
public:
	MsgLayerBase_r()
	{
		//m_poRecvQ = NULL;
		//m_poSendQ = NULL;
	}

	virtual ~MsgLayerBase_r() {}
	virtual bool Init() = 0;
		
	IMsgBase_r* GetServerMsgHandler(int iMsgId);

	//int SendMsg( char* pszData, unsigned int dwLen );

protected:
	virtual void _RegistServerMsg(){}
	
protected:
	MsgHandlerMap_r_t m_oSsMsgHandlerMap;
	//CThreadQueue* m_poRecvQ;
	//CThreadQueue* m_poSendQ;
};


