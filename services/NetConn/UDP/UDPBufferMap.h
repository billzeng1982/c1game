#pragma once

#include "UDPBlock.h"
#include "define.h"
#include <map>

class UDPBufferMap
{
public:
	UDPBufferMap();
	virtual ~UDPBufferMap();

protected:
	int m_iMaxNum;

public:	
	std::map<uint32_t, UDPBlock*> m_oSeqMap;
	
public:
	bool Init(int iMaxNum);
	
	int Add(UDPBlock* pBlock, uint32_t dwSeq);
	void Del(uint32_t dwSeq);
	void Del(std::map<uint32_t, UDPBlock*>::iterator iterBlock);
	std::map<uint32_t, UDPBlock*>::iterator Find(uint32_t dwSeq);
	std::map<uint32_t, UDPBlock*>::iterator Begin();
	std::map<uint32_t, UDPBlock*>::iterator End();
	void Clear();
	int Count();
};

