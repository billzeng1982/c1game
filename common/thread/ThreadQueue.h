/************************************************************

可用作多线程ringbuffer队列, 线程间生产者消费者通信
支持PeekMsg, 因此,left room不足以容纳整个包时, 必须从头开始
注意内部格式为: 头部 + 包体(buf)

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
	unsigned int  m_dwQueueSize; // 数据容量
	volatile unsigned int m_dwHead; // read pointer
	volatile unsigned int m_dwTail; // write pointer
	key_t	m_iShmKey;	// 等于0则在堆上分配
};

struct SThreadQMsgHead
{
	unsigned int m_dwLen; // 消息长度, 不包含head长度
	bool m_bSentinel; // 是否为哨兵
};

#pragma pack()

enum THREAD_Q_ERRNO
{
	THREAD_Q_SUCESS = 1,
	THREAD_Q_EMPTY = 0,	// 队列空

	THREAD_Q_FULL = -1,
	THREAD_Q_READ_BUFF_LIMITED = -2,  //  读数据缓冲区不够
	THREAD_Q_DATA_CONFUSE = -3, // 数据错乱
	THREAD_Q_NULL = -4,  // buf为null
};


class CThreadQueue
{
public:
    CThreadQueue();
	~CThreadQueue();
	bool Init( unsigned int dwQueueSize, key_t iShmKey = 0 );
	void Fini();
    void Clear();

    //队列中是否有足够的空间
    bool HasEnoughRoom(unsigned int dwLen);

	int WriteMsg(char* pszData, unsigned int dwLen);
	int ReadMsg(char* pszData, INOUT unsigned int* pdwLen);

	/*
		本接口与ReadMsg的不同之处在于:
		ReadMsg从数据队列中读取出数据后，会将数据从队列中移除；
		本接口仅仅只是返回数据在队列中的存储位置和长度，并不会将数据从队列中移除，
        当此数据被处理后，请务必调用PopMsg()将数据从队列中移除，
        否则DeleteMsg将总是读取出同一个消息。
	*/
	int PeekMsg( char **ppszData, unsigned int* pdwLen );
	bool IsEmpty() { return m_pstThreadQHead->m_dwHead == m_pstThreadQHead->m_dwTail; }

	/*
		将队列头部消息移除
		本接口与PeekMsg接口配合使用从队列中读取数据进行处理，
		由于读取的数据直接保存在数据队列中，因此可以减少一次数据拷贝过程。
	*/
	int PopMsg();

    /*
        本接口与WriteMsg不同之处在于:
        当队列空间不足时，本接口会自动将队列头的数据Pop，直到
        队列剩余空间可以可以写入本次操作的数据
    */
    int WriteMsg_F(char* pszData, unsigned int dwLen);

private:
	int _GetFreeRoom();
	int _GetLeftDataLen();
	// iPkgLen 包含了HeadLen和BodyLen
	int _MoveToNextMsg( int iPkgLen );

	// 检查数据队列相关参数是否需要调整
	void _CheckQueueHeadVar();

	/*
	    注意读数据时, 出错清空队列一定是tail赋值给head!
	    reader修改Head
	    writer修改tail
	*/
	void _ReaderClearQueue() {  m_pstThreadQHead->m_dwHead = m_pstThreadQHead->m_dwTail; }

private:
	char* m_pszBuf;
	SThreadQueueHead* m_pstThreadQHead;
   	char*	m_pszQueue;
};


// 返回队列已有数据大小
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

//返回队列剩余空间大小
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

