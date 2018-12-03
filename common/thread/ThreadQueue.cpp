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

    //������Ϣ���п��೤�ȣ����ж�ʣ��ռ��Ƿ�
    int iRoom = this->_GetFreeRoom();
    if (iRoom < iHeadLen + (int)dwLen)
    {
        return false;
    }

    // ������ĩβʣ��ռ��Ƿ��ܱ�������������
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

// д����, retval: THREAD_Q_ERRNO
int CThreadQueue::WriteMsg(char* pszData, unsigned int dwLen )
{
	assert( m_pszBuf );

	int iRoom = 0;
	int iTailRoom = 0;
	int iPkgLen = 0;
	int iHeadLen = 0;

	iHeadLen = sizeof(SThreadQMsgHead);
	SThreadQMsgHead stMsgHead;

	//������Ϣ���п��೤�ȣ����ж�ʣ��ռ��Ƿ��㹻�������Ϣ
	iRoom = this->_GetFreeRoom();
	iPkgLen = iHeadLen + (int)dwLen;
	if( iRoom < iPkgLen )
	{
		return THREAD_Q_FULL;
	}

	// ������ĩβʣ��ռ��Ƿ��ܱ�������������,���β��ʣ��ռ䲻������������Ϣ����βָ�������������ǰ��
	iTailRoom = m_pstThreadQHead->m_dwQueueSize - m_pstThreadQHead->m_dwTail;
	if( iTailRoom < iPkgLen )
	{
		if( iTailRoom > iHeadLen )
		{
			// �����βʣ��ռ仹���Ա���һ��α����Ϣ(�ڱ�)�������һ������Ϣ
			stMsgHead.m_dwLen = 0;
			stMsgHead.m_bSentinel = true;
			memcpy( m_pszQueue + m_pstThreadQHead->m_dwTail, &stMsgHead, iHeadLen );
		}

		m_pstThreadQHead->m_dwTail = 0;

		// ���¼������ʣ��ռ䣬�����ʣ��տ��Ƿ��㹻����һ��������Ϣ
		iRoom = (int)m_pstThreadQHead->m_dwHead - (int)m_pstThreadQHead->m_dwTail - 1;
		if (iRoom < iPkgLen)
		{
			return THREAD_Q_FULL;
		}
	}

	// ����������Ϣ���ȱ���msgͷ����Ϣ
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


// ������, retval: THREAD_Q_ERRNO
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
	���ӿ���ReadMsg�Ĳ�֮ͬ������:
	ReadMsg�����ݶ����ж�ȡ�����ݺ󣬻Ὣ���ݴӶ������Ƴ���
	���ӿڽ���ֻ�Ƿ��������ڶ����еĴ洢λ�úͳ��ȣ������Ὣ���ݴӶ������Ƴ���
	�������ݱ����������ص���DeleteMsg�����ݴӶ������Ƴ���
	����DeleteMsg�����Ƕ�ȡ��ͬһ����Ϣ��
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

	//����ͷָ�뵽����ĩβ�Ƿ��㹻����һ����Ϣ
	this->_CheckQueueHeadVar();

	// �ٴμ�����ݶ������Ƿ�������
	iDataLen = this->_GetLeftDataLen();
	if( iDataLen <= 0 )
	{
		return THREAD_Q_EMPTY;
	}

	// get head
	SThreadQMsgHead* pstMsgHead = (SThreadQMsgHead*)( m_pszQueue + m_pstThreadQHead->m_dwHead );

	// ������ݳ���������������ݳ����Ƿ���Ч
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

	// �������ݵ�λ�ü�������Ϣ
	*ppszData = m_pszQueue + m_pstThreadQHead->m_dwHead + iHeadLen;
	*pdwLen = pstMsgHead->m_dwLen;

	return THREAD_Q_SUCESS;
}


/* ǰ��: _CheckQueueHeadVar
   iPkgLen ������HeadLen��BodyLen
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

	// �޸�ͷָ�룬�Խ����ݰ��Ƴ�
	m_pstThreadQHead->m_dwHead = ( m_pstThreadQHead->m_dwHead +  iPkgLen ) % m_pstThreadQHead->m_dwQueueSize;

	return THREAD_Q_SUCESS;
}


// ������ݶ�����ز����Ƿ���Ҫ����
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
	������ͷ����Ϣ�Ƴ�
	���ӿ���PeekMsg�ӿ����ʹ�ôӶ����ж�ȡ���ݽ��д���
	���ڶ�ȡ������ֱ�ӱ��������ݶ����У���˿��Լ���һ�����ݿ������̡�
*/
int CThreadQueue::PopMsg()
{
    if (m_pstThreadQHead->m_dwHead == m_pstThreadQHead->m_dwTail)
    {
        return THREAD_Q_EMPTY;
    }

	// ���ͷָ���Ƿ���Ҫ�ƶ�
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

    //������ʣ��ռ䲻�㣬��Ҫ������ͷ����Ϣ����
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

