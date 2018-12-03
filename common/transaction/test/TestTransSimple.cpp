#include "TransactionFrame.h"
#include "oi_misc.h"
#include "GameTime.h"
#include <stdio.h>
#include <stdlib.h>

namespace TestTrans
{
    void ReleaseSimpleAction(IAction* poAction)
    {
        printf("ReleaseSimpleAction\n");
        poAction->Reset();
        delete poAction;
    }

    void ReleaseTransactionSelf(Transaction* poTrans)
    {
        printf("ReleaseTransactionSelf\n");
        poTrans->Reset();
        delete poTrans;
    }

    class GetDataAction : public IAction
    {
    public:
        GetDataAction() { }
        virtual ~GetDataAction() { }

        virtual void Reset()
        {
            printf("GetDataAction Reset\n");
            IAction::Reset();
        }

        virtual void OnFinished()
        {
            printf("GetDataAction finished\n");
        }

        virtual int Execute( Transaction* pObjTrans )
        {
            printf("GetDataActoin execute!\n");
            printf("async wait...\n");

            return 0;
        }

        virtual void OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen)
        {
            printf("GetDataActoin OnAsynResMsg!\n");
            assert( iActionIdx == m_iIndex );
            const char* pData = (const char*)pResult;
            printf("pData=%s\n", pData);

            // NOTE!!
            this->SetFiniFlag(1);  // 表示动作执行完成
        }
    };

    class GetDataTransaction : public Transaction
    {
    public:
        GetDataTransaction() {}
        virtual ~GetDataTransaction() {}

        virtual void OnFinished( int iErrCode )
        {
            printf("GetDataTransaction OnFinished! iErrCode <%d>\n", iErrCode);
        }

        virtual void OnActionFinished(TActionIndex iActionIdx)
        {
            printf("GetDataAction finished, ActionIdx<%d>\n", iActionIdx);
        }
    };
};


int main()
{
    CGameTime::Instance().UpdateTime();

    TransactionFrame oFrame;

    if( !oFrame.Init( 10, 10, TestTrans::ReleaseSimpleAction, TestTrans::ReleaseTransactionSelf ) )
    {
        return -1;
    }

    TestTrans::GetDataTransaction* poGetDataTrans = new TestTrans::GetDataTransaction();
    TestTrans::GetDataAction* poGetDataAction = new TestTrans::GetDataAction();
    poGetDataAction->SetTimeout(2000);

    int iRet = poGetDataTrans->AddAction( poGetDataAction );
    printf( "iRet = %d\n", iRet );

    printf("Now schedule transaction!\n");
    oFrame.ScheduleTransaction( poGetDataTrans, 2000 );

    uint64_t ullToken = poGetDataAction->GetToken();
    printf("ullToken=<%lx>\n", ullToken);

    sleep( 1 );

    char data[32];
    snprintf(data, sizeof(data), "%s", "Hello, data got!!");
    oFrame.AsyncActionDone( ullToken, data, strlen(data) );

    // test timeout
    int iTotalSleepMs = 0;
    while( iTotalSleepMs < 3000 )
    {
        CGameTime::Instance().UpdateTime();
        oFrame.Update();

        MsSleep(10);
        iTotalSleepMs += 10;
    }

    // check obsolete logic
    //oFrame.AsyncActionDone( ullToken, data, strlen(data) );

    return 0;
}

