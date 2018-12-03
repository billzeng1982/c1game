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
	MapSeq2Block m_oSendedMap; // �����ѷ���δ�յ�ack�İ�, resend ����
	MapSeq2Block m_oRecvMap;   // ������ǰ���İ�(SeqNum��Ծ)

	uint64_t m_ullTimestampMs; // ������������ack
	std::list<uint32_t> m_oAckList;
};

