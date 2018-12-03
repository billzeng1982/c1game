#pragma once

#include "object.h"
#include "define.h"
#include "UDPBlock.h"
#include <list>
#include <map>

class UDPClientBuffIndex : public IObject
{
public:
	UDPClientBuffIndex(){};
	virtual ~UDPClientBuffIndex(){};

	virtual void Clear();
	typedef std::map<uint32_t, UDPBlock*> MapSeq2Block;

protected:
	void _Construct();

public:
	MapSeq2Block m_oSendedMap; // 索引已发送未收到ack的包, resend 缓冲
	MapSeq2Block m_oRecvMap;   // 索引提前到的包(SeqNum跳跃)

	uint64_t m_ullTimestampMs; // 用于批量发送ack
	std::list<uint32_t> m_oAckList;
};

