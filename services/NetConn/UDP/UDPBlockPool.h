#ifndef _UDP_POOL_H_
#define _UDP_POOL_H_

#include "mempool.h"
#include "UDPBlock.h"
#include <map>

class UDPBlockPool
{
public:
	UDPBlockPool();
	virtual ~UDPBlockPool();

protected:
	CMemPool<char[32]> m_oBuffPool_32;
	CMemPool<char[64]> m_oBuffPool_64;
	CMemPool<char[128]> m_oBuffPool_128;
	CMemPool<char[256]> m_oBuffPool_256;
	CMemPool<char[512]> m_oBuffPool_512;
	CMemPool<char[1024]> m_oBuffPool_1024;
	CMemPool<char[8192]> m_oBuffPool_8192;
public:
	bool Init(int iBlockNum);

	UDPBlock* GetBlock(int iSize);
	void ReleaseBlock(UDPBlock* pBlock);
};

#endif

