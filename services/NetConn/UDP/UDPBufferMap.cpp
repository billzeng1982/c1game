#include "UDPBufferMap.h"

using namespace std;

UDPBufferMap::UDPBufferMap()
{ 
    m_iMaxNum = 200; 
}

UDPBufferMap::~UDPBufferMap()
{
    this->Clear();
}

bool UDPBufferMap::Init(int iMaxNum)
{
    m_iMaxNum = iMaxNum;
    return true;
}
	
int UDPBufferMap::Add(UDPBlock* pBlock, uint32_t dwSeq)
{
    if ((int)m_oSeqMap.size() < m_iMaxNum)
    {
        std::pair<std::map<uint32_t, UDPBlock*>::iterator, bool> stRet = m_oSeqMap.insert(pair<uint32_t, UDPBlock*>(dwSeq, pBlock));
        if (!stRet.second)
        {
            return -1;
        }
    }
    else
    {
        // overflow buffer
        return -1;
    }

    return 0;
}

void UDPBufferMap::Del(uint32_t dwSeq)
{
    m_oSeqMap.erase(dwSeq);
    return;
}

void UDPBufferMap::Del(map<uint32_t, UDPBlock*>::iterator iterBlock)
{
    m_oSeqMap.erase(iterBlock);
    return;
}

map<uint32_t, UDPBlock*>::iterator UDPBufferMap::Find(uint32_t dwSeq)
{
    return m_oSeqMap.find(dwSeq);
}

map<uint32_t, UDPBlock*>::iterator UDPBufferMap::Begin()
{
    return m_oSeqMap.begin();
}

map<uint32_t, UDPBlock*>::iterator UDPBufferMap::End()
{
    return m_oSeqMap.end();
}

void UDPBufferMap::Clear()
{
    m_oSeqMap.clear();
}

int UDPBufferMap::Count()
{
    return (int)m_oSeqMap.size();
}


