#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "CEpoll.h"

CEpoll::~CEpoll()
{
	Close();
}

void CEpoll::Close()
{
	if (m_iEpollFd >= 0)
	{
		close(m_iEpollFd);
		m_iEpollFd = -1;
	}
    
	if (m_pstEvents)
	{
		delete	[]m_pstEvents;
		m_pstEvents = NULL;
	}
}

// 创建epoll 
bool CEpoll::Create( int iMaxFd )
{
	Close();

	m_iMaxFd = iMaxFd;
	m_pstEvents = new epoll_event[ m_iMaxFd ];
	m_iEpollFd = epoll_create( m_iMaxFd );

    if( m_iEpollFd < 0 )
	{
	    snprintf( m_szErrMsg, sizeof(m_szErrMsg), "fail to create epoll! %s", strerror(errno) );
		return false;
	}

	return true;
}


// 添加socket到epoll中
bool CEpoll::EpollAdd( int iSock, void *ptr, uint32_t uiEvent )
{
    struct epoll_event ev;

    ev.events = uiEvent;

    if( ptr )
    {
        ev.data.ptr = ptr;
    }else
    {
        ev.data.fd = iSock;
    }

    if ( epoll_ctl( m_iEpollFd, EPOLL_CTL_ADD, iSock, &ev ) < 0 )
    {
        snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Epoll add <Fd:%d> error! %s", iSock, strerror(errno) );
        return	false;
    }

    return	true;
}


bool CEpoll::EpollMod( int iSock, void *ptr, uint32_t uiEvent )
{
	struct epoll_event ev;

	ev.events = uiEvent;
    if( ptr )
	{
	    ev.data.ptr = ptr;
    }else
    {
        ev.data.fd = iSock; 
    }

	if ( epoll_ctl( m_iEpollFd, EPOLL_CTL_MOD, iSock, &ev ) < 0)
	{
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Epoll mod <Fd:%d> error! %s", iSock, strerror(errno) );
		return	false;
	}
	
	return	true;
}


// 等待epoll事件
int CEpoll::Wait( int iTimeInMs )
{
	return	epoll_wait( m_iEpollFd, m_pstEvents, m_iMaxFd,iTimeInMs );
}


// 从epoll中删除socket
bool CEpoll::EpollDel( int iSock )
{
    struct epoll_event ev;

    if( epoll_ctl( m_iEpollFd, EPOLL_CTL_DEL, iSock, &ev ) < 0 )
    {
        snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Epoll del <Fd:%d> failed! %s", iSock, strerror(errno) );
        return false;
    }

    return true;
}


bool CEpoll::IsEvReadable( int iPos )
{
	if ( iPos < 0 || iPos >= m_iMaxFd)
	{
		assert(false);
		return	false;
	}

    return ( m_pstEvents[iPos].events & EPOLLIN ) ? true : false;
}


bool CEpoll::IsEvWritable( int iPos )
{
	if ( iPos < 0 || iPos >= m_iMaxFd)
	{
		assert(false);
		return	false;
	}

    return ( m_pstEvents[iPos].events & EPOLLOUT ) ? true : false;
}


// 获得自定义数据指针
void *CEpoll::GetEpollDataPtr( int iPos )
{
    if( iPos < 0 || iPos >= m_iMaxFd )
    {
        assert( false );
        return NULL;
    }

    return  m_pstEvents[iPos].data.ptr;
}


// 获得epool fd, 注意: 只有EpollAdd时, ptr为NULL, 才能调用此接口
int CEpoll::GetEpollDataFd( int iPos )
{
    if( iPos < 0 || iPos >= m_iMaxFd )
    {
        assert( false );
        return -1;
    }

    return m_pstEvents[iPos].data.fd;
}

