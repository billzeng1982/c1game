#ifndef _THREAD_MUTEXT_H_
#define _THREAD_MUTEXT_H_

#include <pthread.h>
#include <assert.h>

class CThreadMutex
{
public:
	CThreadMutex() {}
	~CThreadMutex() { pthread_mutex_destroy(&m_stMutex); }
	
	bool Init() { return  ( pthread_mutex_init(&m_stMutex, NULL) == 0 ); }
	
	int Lock()      { return pthread_mutex_lock(&m_stMutex); }
    int Unlock()    { return pthread_mutex_unlock(&m_stMutex); }
    int	TryLock()   { return pthread_mutex_trylock(&m_stMutex); }
	
private:
	pthread_mutex_t m_stMutex;
};


class CThreadMutexGuard
{
public:
    explicit CThreadMutexGuard(CThreadMutex& roMutex) : m_roMutex(roMutex)
    {
        m_roMutex.Lock();
    }

    ~CThreadMutexGuard()
    {
        m_roMutex.Unlock();
    }
	
private:
    CThreadMutex& m_roMutex;
};

#endif
