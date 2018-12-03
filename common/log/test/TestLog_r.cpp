#include "../LogMacros.h"
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include "oi_misc.h"
#include "GameTime.h"

namespace TestLog_r
{

    struct ThreadArg
    {
        int m_iThreadID;
        int m_iLogNum;
    };

    void* LogTest( void* pvArg )
    {
        ThreadArg* pThreadArg = (ThreadArg*)pvArg;
       
        for( int i = 0; i < pThreadArg->m_iLogNum; i++ )
        {
            CGameTime::Instance().UpdateTime();
        
            if( i % 50 == 0 )
            {
                MsSleep(1);
            }
            LOGRUN_r( "Thread <%d>: Log seq %d", pThreadArg->m_iThreadID, i+1 );
        }

        pthread_exit((void *)0);
    }
};

int main( int argc, char** argv )
{
    assert( argc == 3 );

    int iThreadNum = atoi(argv[1]);
    int iLogNum = atoi(argv[2]);
    assert( iThreadNum > 0 );

    char szLogPath[512];   
    snprintf( szLogPath, sizeof(szLogPath), "./" );
    CSingleton<CBufferedLog>::Instance().Init( szLogPath, "TestLog_r", 0, 10, LOG_FILE_SIZE );

    TestLog_r::ThreadArg* paThreadArg = new TestLog_r::ThreadArg[ iThreadNum ];
    pthread_t* aiThreadID = new pthread_t[iThreadNum];

    CGameTime::Instance().UpdateTime();
    
    // create threads
    for( int i = 0; i < iThreadNum; i++ )
    {
        paThreadArg[i].m_iThreadID = i+1;
        paThreadArg[i].m_iLogNum = iLogNum;

        pthread_create( &(aiThreadID[i]), NULL, TestLog_r::LogTest, &paThreadArg[i] );
    }
    
    // join threads
    for( int i = 0; i < iThreadNum; i++ )
    {
        pthread_join( aiThreadID[i], NULL );
    }

    return 0;
}


