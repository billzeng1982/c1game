#include "../coroutine.h"
#include <stdio.h>
#include <vector>
using namespace std;

namespace COTEST
{
    static vector<int> g_vCoIDs;

    static void func( void* arg )
    {
        CoroutineEnv* pCoEnv = (CoroutineEnv*)arg;
        g_vCoIDs.push_back( pCoEnv->GetCurrCoID() );

        printf("[Co] run in func, cur CoID = %d \n", pCoEnv->GetCurrCoID());
        printf("[Co] ready to yield\n");

        pCoEnv->CoYield();

        printf("[Co] coroutine<%d> resumed!\n", pCoEnv->GetCurrCoID());
        printf("[Co] resume event id: %d\n\n", pCoEnv->GetCurrResumeEvent()->m_iEventID);
    }
};

int main()
{
    CoroutineEnv oCoEnv;

    int iRet = 0;

    oCoEnv.Init(16);

    for( int i = 0; i < 3; i++ )
    {
        printf("[main] start coroutine %d\n", i+1);
        iRet = oCoEnv.StartCoroutine( COTEST::func, &oCoEnv );
        if( iRet < 0 )
        {
            assert( false );
        }
    }

    printf("[main] main logic go here ...\n");

    // resume event
    CoResumeEvent_t stCoResumeEvt;
    
    printf("[main] Now resume all coroutines ... \n\n");
    while( !COTEST::g_vCoIDs.empty() )
    {
        int iCoID = COTEST::g_vCoIDs.back();
        stCoResumeEvt.m_iEventID = iCoID;

        printf("[main] resume %d\n", iCoID);
        oCoEnv.CoResume( iCoID, &stCoResumeEvt );
        printf("[main] control back to main\n");
        
        COTEST::g_vCoIDs.pop_back();
    }
    
    printf("[main] program exit!!\n");
    
    return 0;
}


