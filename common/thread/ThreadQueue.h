/************************************************************

���������߳�ringbuffer����, �̼߳�������������ͨ��
֧��PeekMsg, ���,left room����������������ʱ, �����ͷ��ʼ
ע���ڲ���ʽΪ: ͷ�� + ����(buf)

traits:
	lock-free, byte-level, ring buffer, shm-supported
***********************************************************/

#ifndef _THREAD_QUEUE_H_
#define _THREAD_QUEUE_H_

#include "shmop.h"
#include "define.h"

#pragma pack(1)

struct SThreadQueueHead
{
	unsigned int  m_dwQueueSize; // ��������
	volatile unsigned int m_dwHead; // read pointer
	volatile unsigned int m_dwTail; // write pointer
	key_t	m_iShmKey;	// ����0���ڶ��Ϸ���
};

struct SThreadQMsgHead
{
	unsigned int m_dwLen; // ��Ϣ����, ������head����
	bool m_bSentinel; // �Ƿ�Ϊ�ڱ�
};

#pragma pack()

enum THREAD_Q_ERRNO
{
	THREAD_Q_SUCESS = 1,
	THREAD_Q_EMPTY = 0,	// ���п�

	THREAD_Q_FULL = -1,
	THREAD_Q_READ_BUFF_LIMITED = -2,  //  �����ݻ���������
	THREAD_Q_DATA_CONFUSE = -3, // ���ݴ���
	THREAD_Q_NULL = -4,  // bufΪnull
};


class CThreadQueue
{
public:
    CThreadQueue();
	~CThreadQueue();
	bool Init( unsigned int dwQueueSize, key_t iShmKey = 0 );
	void Fini();
    void Clear();

    //�������Ƿ����㹻�Ŀռ�
    bool HasEnoughRoom(unsigned int dwLen);

	int WriteMsg(char* pszData, unsigned int dwLen);
	int ReadMsg(char* pszData, INOUT unsigned int* pdwLen);

	/*
		���ӿ���ReadMsg�Ĳ�֮ͬ������:
		ReadMsg�����ݶ����ж�ȡ�����ݺ󣬻Ὣ���ݴӶ������Ƴ���
		���ӿڽ���ֻ�Ƿ��������ڶ����еĴ洢λ�úͳ��ȣ������Ὣ���ݴӶ������Ƴ���
        �������ݱ����������ص���PopMsg()�����ݴӶ������Ƴ���
        ����DeleteMsg�����Ƕ�ȡ��ͬһ����Ϣ��
	*/
	int PeekMsg( char **ppszData, unsigned int* pdwLen );
	bool IsEmpty() { return m_pstThreadQHead->m_dwHead == m_pstThreadQHead->m_dwTail; }

	/*
		������ͷ����Ϣ�Ƴ�
		���ӿ���PeekMsg�ӿ����ʹ�ôӶ����ж�ȡ���ݽ��д���
		���ڶ�ȡ������ֱ�ӱ��������ݶ����У���˿��Լ���һ�����ݿ������̡�
	*/
	int PopMsg();

    /*
        ���ӿ���WriteMsg��֮ͬ������:
        �����пռ䲻��ʱ�����ӿڻ��Զ�������ͷ������Pop��ֱ��
        ����ʣ��ռ���Կ���д�뱾�β���������
    */
    int WriteMsg_F(char* pszData, unsigned int dwLen);

private:
	int _GetFreeRoom();
	int _GetLeftDataLen();
	// iPkgLen ������HeadLen��BodyLen
	int _MoveToNextMsg( int iPkgLen );

	// ������ݶ�����ز����Ƿ���Ҫ����
	void _CheckQueueHeadVar();

	/*
	    ע�������ʱ, ������ն���һ����tail��ֵ��head!
	    reader�޸�Head
	    writer�޸�tail
	*/
	void _ReaderClearQueue() {  m_pstThreadQHead->m_dwHead = m_pstThreadQHead->m_dwTail; }

private:
	char* m_pszBuf;
	SThreadQueueHead* m_pstThreadQHead;
   	char*	m_pszQueue;
};


// ���ض����������ݴ�С
inline int CThreadQueue::_GetLeftDataLen()
{
	int iDataLen;

    iDataLen = (int)m_pstThreadQHead->m_dwTail - (int)m_pstThreadQHead->m_dwHead;

    if(iDataLen < 0)
    {
        iDataLen += (int)m_pstThreadQHead->m_dwQueueSize;
    }

    return iDataLen;
}

//���ض���ʣ��ռ��С
inline int CThreadQueue::_GetFreeRoom()
{
    int iFreeRoom;

    iFreeRoom = (int)m_pstThreadQHead->m_dwHead - (int)m_pstThreadQHead->m_dwTail -1;
    if( iFreeRoom < 0 )
    {
        iFreeRoom += m_pstThreadQHead->m_dwQueueSize;
    }

    return iFreeRoom;
}


#endif

