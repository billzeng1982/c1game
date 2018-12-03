#include "UDPBlockPool.h"
#include "LogMacros.h"
#include <math.h>
#include "GameObjectPool.h"
#include "comm_func.h"

UDPBlockPool::UDPBlockPool()
{

}

UDPBlockPool::~UDPBlockPool()
{
    m_oBuffPool_32.CleanPool();
    m_oBuffPool_64.CleanPool();
    m_oBuffPool_128.CleanPool();
    m_oBuffPool_256.CleanPool();
    m_oBuffPool_512.CleanPool();
    m_oBuffPool_1024.CleanPool();
    m_oBuffPool_8192.CleanPool();
}

bool UDPBlockPool::Init(int iBlockNum)
{
	if (m_oBuffPool_32.CreatePool(iBlockNum*200) < 0)
        return false;

	if (m_oBuffPool_64.CreatePool(iBlockNum*50) < 0)
        return false;

	if (m_oBuffPool_128.CreatePool(iBlockNum*10) < 0)
        return false;

	if (m_oBuffPool_256.CreatePool(iBlockNum*4) < 0)
        return false;

	if (m_oBuffPool_512.CreatePool(iBlockNum*2) < 0)
        return false;

	if (m_oBuffPool_1024.CreatePool(iBlockNum) < 0)
        return false;

    if (m_oBuffPool_8192.CreatePool(iBlockNum/50) < 0)
        return false;

    return true;
}

UDPBlock* UDPBlockPool::GetBlock(int iSize)
{
    int iCeilSize = ceilingPowerOfTwo(iSize);

    if (iCeilSize > 8192)
    {
        LOGRUN("iCeilSize > 8192");
        return NULL;
    }

    if( iCeilSize < 32 )
    {
        iCeilSize = 32;
    }

    UDPBlock* pBlock = GET_GAMEOBJECT( UDPBlock, GAMEOBJ_UDPBLOCK );
    if (!pBlock)
    {
        return NULL;
    }
    else
    {
        pBlock->m_iLen = iCeilSize;
        switch(iCeilSize)
        {
        case 32:
            pBlock->m_pszBuff = *m_oBuffPool_32.NewData(&pBlock->m_iHnd);
            LOGWARN("m_oBuffPool_32 used num is %d", m_oBuffPool_32.GetUsedNum());
            bzero(pBlock->m_pszBuff, 32);
            break;

        case 64:
            pBlock->m_pszBuff = *m_oBuffPool_64.NewData(&pBlock->m_iHnd);
            LOGWARN("m_oBuffPool_64 used num is %d", m_oBuffPool_32.GetUsedNum());
            bzero(pBlock->m_pszBuff, 64);
            break;

        case 128:
            pBlock->m_pszBuff = *m_oBuffPool_128.NewData(&pBlock->m_iHnd);
            LOGWARN("m_oBuffPool_128 used num is %d", m_oBuffPool_32.GetUsedNum());
            bzero(pBlock->m_pszBuff, 128);
            break;

        case 256:
            pBlock->m_pszBuff = *m_oBuffPool_256.NewData(&pBlock->m_iHnd);
            LOGWARN("m_oBuffPool_256 used num is %d", m_oBuffPool_32.GetUsedNum());
            bzero(pBlock->m_pszBuff, 256);
            break;

        case 512:
            pBlock->m_pszBuff = *m_oBuffPool_512.NewData(&pBlock->m_iHnd);
            LOGWARN("m_oBuffPool_512 used num is %d", m_oBuffPool_32.GetUsedNum());
            bzero(pBlock->m_pszBuff, 512);
            break;

        case 1024:
            pBlock->m_pszBuff = *m_oBuffPool_1024.NewData(&pBlock->m_iHnd);
            LOGWARN("m_oBuffPool_1024 used num is %d", m_oBuffPool_32.GetUsedNum());
            bzero(pBlock->m_pszBuff, 1024);
            break;

        case 8192:
            pBlock->m_pszBuff = *m_oBuffPool_8192.NewData(&pBlock->m_iHnd);
            LOGWARN("m_oBuffPool_8192 used num is %d", m_oBuffPool_32.GetUsedNum());
            bzero(pBlock->m_pszBuff, 8192);
            break;

        default:
            RELEASE_GAMEOBJECT(pBlock);
            return NULL;
        }
    }

    return pBlock;
}

void UDPBlockPool::ReleaseBlock(UDPBlock* pBlock)
{
	if (pBlock == NULL)
	{
		return;
	}

    switch(pBlock->m_iLen)
    {
    case 32:
        m_oBuffPool_32.DeleteData(pBlock->m_iHnd);
        break;

    case 64:
        m_oBuffPool_64.DeleteData(pBlock->m_iHnd);
        break;

    case 128:
        m_oBuffPool_128.DeleteData(pBlock->m_iHnd);
        break;

    case 256:
        m_oBuffPool_256.DeleteData(pBlock->m_iHnd);
        break;

    case 512:
        m_oBuffPool_512.DeleteData(pBlock->m_iHnd);
        break;

    case 1024:
        m_oBuffPool_1024.DeleteData(pBlock->m_iHnd);
        break;

    case 8192:
        m_oBuffPool_8192.DeleteData(pBlock->m_iHnd);
        break;

    default:
        LOGERR("m_oBlockPool_xxx delete failed. len-%d", pBlock->m_iLen);
        break;
    }

    RELEASE_GAMEOBJECT(pBlock);
}

