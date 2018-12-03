#include "pal/pal.h"
#include "ThreadQueue.h"

CThreadQueue::CThreadQueue()
{
	m_pszBuf = NULL;
	m_pstThreadQHead = NULL;
	m_pszQueue = NULL;
}


CThreadQueue::~CThreadQueue()
{
	this->Fini();
}


bool CThreadQueue::Init( unsigned int dwQueueSize, key_t iShmKey )
{
	if( 0 == dwQueueSize )
	{
		assert( false );
		return false;
	}

	uint32_t dwSize = dwQueueSize + sizeof(SThreadQueueHead);
	int iNew = 0;

	if( 0 == iShmKey )
	{
		m_pszBuf = new char[dwSize];
		iNew = 1;
	}
	else
	{
		iNew = ::GetShm( (void**)&m_pszBuf, iShmKey, (int)dwSize, (0666 | IPC_CREAT) );
	}

	if( !m_pszBuf || iNew < 0 )
	{
		return false;
	}

	m_pstThreadQHead = (SThreadQueueHead*)m_pszBuf;
	m_pszQueue = m_pszBuf + sizeof( SThreadQueueHead );

	if( 1 == iNew )
	{
		m_pstThreadQHead->m_dwHead = 0;
		m_pstThreadQHead->m_dwTail = 0;
		m_pstThreadQHead->m_dwQueueSize = dwQueueSize;
		m_pstThreadQHead->m_iShmKey = iShmKey;
	}

	return true;
}


void CThreadQueue::Fini()
{
	if( m_pszBuf )
	{
		if( 0 == m_pstThreadQHead->m_iShmKey )
		{
			delete[] m_pszBuf;
		}else
		{
			::DetachShm(m_pszBuf);
		}

		m_pszBuf = NULL;
		m_pstThreadQHead = NULL;
		m_pszQueue = NULL;
	}
}


void CThreadQueue::Clear()
{
    m_pstThreadQHead->m_dwHead = 0;
    m_pstThreadQHead->m_dwTail = 0;
}

bool CThreadQueue::HasEnoughRoom(unsigned int dwLen)
{
    int iHeadLen = sizeof(SThreadQMsgHead);

    //计算消息队列空余长度，并判断剩余空间是否够
    int iRoom = this->_GetFreeRoom();
    if (iRoom < iHeadLen + (int)dwLen)
    {
        return false;
    }

    // 检查队列末尾剩余空间是否能保存下整个数据
    int iTailRoom = m_pstThreadQHead->m_dwQueueSize - m_pstThreadQHead->m_dwTail;
    if (iTailRoom < iHeadLen + (int)dwLen)
    {
        iRoom = (int)m_pstThreadQHead->m_dwHead - 1;
        if (iRoom < iHeadLen + (int)dwLen)
        {
            return false;
        }
    }

    return true;
}

// 写队列, retval: THREAD_Q_ERRNO
int CThreadQueue::WriteMsg(char* pszData, unsigned int dwLen )
{
	assert( m_pszBuf );

	int iRoom = 0;
	int iTailRoom = 0;
	int iPkgLen = 0;
	int iHeadLen = 0;

	iHeadLen = sizeof(SThreadQMsgHead);
	SThreadQMsgHead stMsgHead;

	//计算消息队列空余长度，并判断剩余空间是否足够保存此消息
	iRoom = this->_GetFreeRoom();
	iPkgLen = iHeadLen + (int)dwLen;
	if( iRoom < iPkgLen )
	{
		return THREAD_Q_FULL;
	}

	// 检查队列末尾剩余空间是否能保存下整个数据,如果尾部剩余空间不够保存完整消息，则将尾指针调整到队列最前面
	iTailRoom = m_pstThreadQHead->m_dwQueueSize - m_pstThreadQHead->m_dwTail;
	if( iTailRoom < iPkgLen )
	{
		if( iTailRoom > iHeadLen )
		{
			// 如果队尾剩余空间还可以保存一个伪造消息(哨兵)，则放置一个假消息
			stMsgHead.m_dwLen = 0;
			stMsgHead.m_bSentinel = true;
			memcpy( m_pszQueue + m_pstThreadQHead->m_dwTail, &stMsgHead, iHeadLen );
		}

		m_pstThreadQHead->m_dwTail = 0;

		// 重新计算队列剩余空间，并检查剩余空空是否足够保存一个完整消息
		iRoom = (int)m_pstThreadQHead->m_dwHead - (int)m_pstThreadQHead->m_dwTail - 1;
		if (iRoom < iPkgLen)
		{
			return THREAD_Q_FULL;
		}
	}

	// 保存整个消息，先保存msg头部信息
	stMsgHead.m_dwLen = dwLen;
	stMsgHead.m_bSentinel = false;
	char* pch = m_pszQueue + m_pstThreadQHead->m_dwTail;
	memcpy( pch, &stMsgHead, iHeadLen );
	pch += iHeadLen;

	// store data
	memcpy( pch, pszData, dwLen );

	m_pstThreadQHead->m_dwTail = (m_pstThreadQHead->m_dwTail + iPkgLen) % m_pstThreadQHead->m_dwQueueSize;

	return THREAD_Q_SUCESS;
}


// 读队列, retval: THREAD_Q_ERRNO
int CThreadQueue::ReadMsg(char* pszData, INOUT unsigned int* pdwLen)
{
	assert( m_pszBuf );
	assert( pszData && pdwLen );

	unsigned int dwDataLen = 0;
	char* pszGetData = NULL;

	int iRet = this->PeekMsg( &pszGetData, &dwDataLen );
	if( iRet != THREAD_Q_SUCESS )
	{
		return iRet; // maybe empty
	}

	if( *pdwLen < dwDataLen )
	{
		return THREAD_Q_READ_BUFF_LIMITED;
	}

	// copy body
	memcpy( pszData, pszGetData, dwDataLen );
	*pdwLen = dwDataLen;

	return this->PopMsg();
}

/*
	本接口与ReadMsg的不同之处在于:
	ReadMsg从数据队列中读取出数据后，会将数据从队列中移除；
	本接口仅仅只是返回数据在队列中的存储位置和长度，并不会将数据从队列中移除，
	当此数据被处理后，请务必调用DeleteMsg将数据从队列中移除，
	否则DeleteMsg将总是读取出同一个消息。
*/
int CThreadQueue::PeekMsg( char **ppszData, unsigned int* pdwLen )
{
	assert( m_pszBuf );
	assert( ppszData && pdwLen );

	int iPkgLen = 0;
	int iDataLen = 0;

	*ppszData = NULL;
	*pdwLen = 0;

	if( this->IsEmpty() )
	{
		return THREAD_Q_EMPTY;
	}

	if( m_pstThreadQHead->m_dwHead >= m_pstThreadQHead->m_dwQueueSize ||
		m_pstThreadQHead->m_dwTail >= m_pstThreadQHead->m_dwQueueSize )
	{
		this->_ReaderClearQueue();
		return THREAD_Q_DATA_CONFUSE;
	}

	//检查从头指针到队列末尾是否足够保存一条消息
	this->_CheckQueueHeadVar();

	// 再次检查数据队列中是否有数据
	iDataLen = this->_GetLeftDataLen();
	if( iDataLen <= 0 )
	{
		return THREAD_Q_EMPTY;
	}

	// get head
	SThreadQMsgHead* pstMsgHead = (SThreadQMsgHead*)( m_pszQueue + m_pstThreadQHead->m_dwHead );

	// 检查数据长度与与队列中数据长度是否有效
	int iHeadLen = sizeof(SThreadQMsgHead);
	iPkgLen = pstMsgHead->m_dwLen + iHeadLen;
	if( iPkgLen <= 0 || (int)m_pstThreadQHead->m_dwQueueSize < iPkgLen )
	{
		this->_ReaderClearQueue();
		return THREAD_Q_DATA_CONFUSE;
	}
	if( iDataLen < iPkgLen )
	{
		this->_ReaderClearQueue();
		return THREAD_Q_DATA_CONFUSE;
	}

	// 设置数据的位置及长度信息
	*ppszData = m_pszQueue + m_pstThreadQHead->m_dwHead + iHeadLen;
	*pdwLen = pstMsgHead->m_dwLen;

	return THREAD_Q_SUCESS;
}


/* 前置: _CheckQueueHeadVar
   iPkgLen 包含了HeadLen和BodyLen
*/
int CThreadQueue::_MoveToNextMsg( int iPkgLen )
{
	int iDataLen = this->_GetLeftDataLen();
	if( 0 == iDataLen )
	{
		// empty
		return THREAD_Q_EMPTY;
	}

	if( iDataLen < iPkgLen )
	{
		return THREAD_Q_DATA_CONFUSE;
	}

	if( iPkgLen < 0 || (int)m_pstThreadQHead->m_dwQueueSize < iPkgLen  )
	{
		assert( false );
		this->_ReaderClearQueue();
		return THREAD_Q_DATA_CONFUSE;
	}

	// 修改头指针，以将数据包移除
	m_pstThreadQHead->m_dwHead = ( m_pstThreadQHead->m_dwHead +  iPkgLen ) % m_pstThreadQHead->m_dwQueueSize;

	return THREAD_Q_SUCESS;
}


// 检查数据队列相关参数是否需要调整
void CThreadQueue::_CheckQueueHeadVar()
{
	int iTailLen = (int)m_pstThreadQHead->m_dwQueueSize - (int)m_pstThreadQHead->m_dwHead;
	if( iTailLen <= (int)sizeof(SThreadQMsgHead) )
	{
		m_pstThreadQHead->m_dwHead = 0;
	}else
	{
		SThreadQMsgHead* pstMsgHead = (SThreadQMsgHead*)( m_pszQueue + m_pstThreadQHead->m_dwHead );
		if( pstMsgHead->m_bSentinel )
		{
			assert( 0 == pstMsgHead->m_dwLen );
			m_pstThreadQHead->m_dwHead = 0;
		}
	}
}


/*
	将队列头部消息移除
	本接口与PeekMsg接口配合使用从队列中读取数据进行处理，
	由于读取的数据直接保存在数据队列中，因此可以减少一次数据拷贝过程。
*/
int CThreadQueue::PopMsg()
{
    if (m_pstThreadQHead->m_dwHead == m_pstThreadQHead->m_dwTail)
    {
        return THREAD_Q_EMPTY;
    }

	// 检查头指针是否需要移动
	this->_CheckQueueHeadVar();

	int iDataLen = this->_GetLeftDataLen();
	if( iDataLen <= 0 )
	{
		return THREAD_Q_EMPTY;
	}

	int iHeadLen = sizeof(SThreadQMsgHead);
	if( iDataLen < iHeadLen )
	{
		this->_ReaderClearQueue();
		return THREAD_Q_DATA_CONFUSE;
	}

	SThreadQMsgHead* pstMsgHead = (SThreadQMsgHead*)( m_pszQueue + m_pstThreadQHead->m_dwHead );
	int iPkgLen = pstMsgHead->m_dwLen + iHeadLen;
	int iRet = this->_MoveToNextMsg(iPkgLen);
	if( iRet != THREAD_Q_SUCESS )
	{
		this->_ReaderClearQueue();
	}

	return iRet;
}


int CThreadQueue::WriteMsg_F(char* pszData, unsigned int dwLen)
{
    if (this->HasEnoughRoom(dwLen))
    {
        return WriteMsg(pszData, dwLen);
    }

    //队列中剩余空间不足，需要将队列头的消息出队
    while(true)
    {
        int iRet = this->PopMsg();
        if (iRet != THREAD_Q_SUCESS)
        {
            this->Clear();
            break;
        }

        if (this->HasEnoughRoom(dwLen))
        {
            break;
        }
    }

    return WriteMsg(pszData, dwLen);
}

