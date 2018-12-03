#ifndef _CEPOLL_H_
#define _CEPOLL_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>

#include "define.h"

/*
	 The struct epoll_event is defined as :

       typedef union epoll_data {
           void    *ptr;
           int      fd;
           uint32_t u32;
           uint64_t u64;
       } epoll_data_t;

       struct epoll_event {
           uint32_t     events;    // Epoll events 
           epoll_data_t data;      // User data variable
       };
*/

#define EPOLL_DATA_TYPE_PTR 1
#define EPOLL_DATA_TYPE_FD 2

class CEpoll
{
public:
    CEpoll():m_iEpollFd(-1), m_pstEvents(NULL)
    {}

    ~CEpoll();
    
    bool    Create( int iMaxFd );
    void    Close();
    
    int     Wait( int iTimeInMs = 10 );

	bool    EpollAdd( int iSock, void *ptr, uint32_t uiEvent = EPOLLIN );
	
    bool    EpollMod( int iSock, void * ptr, uint32_t uiEvent );
	
    bool    EpollDel( int iSock );
    
    bool    IsEvReadable( int iPos ); // �¼��Ƿ�ɶ�
    bool    IsEvWritable( int iPos );   // �¼��Ƿ��д

	// ���epoll����ָ��
	void* 	GetEpollDataPtr( int iPos );

	// ���epool fd, ע��: ֻ��EpollAddʱ, ptrΪNULL, ���ܵ��ô˽ӿ�
	int 	GetEpollDataFd( int iPos );
	
	char* GetErrMsg() { return m_szErrMsg; }

private:

	char   m_szErrMsg[256];
    int    m_iMaxFd;
    int    m_iEpollFd;
    struct epoll_event *m_pstEvents;
};

#endif
