#include "transaction.h"
#include <string.h>
#include "LogMacros.h"
#include "CompositeAction.h"
#include "TransactionHelper.h"

Transaction::Transaction()
{
	this->Reset();
}

void Transaction::Reset()
{
	m_iActionCount = 0;
	m_dwTransactionID = 0;
	m_bFinished = false;
	m_dwTimeout = 0;
    bzero( m_aActionArray, sizeof(m_aActionArray) );
    m_iCurStep = 0;
}

TActionIndex Transaction::AddAction(IAction* poAction)
{
    if( !poAction )
    {
        assert(false);
        return -1;
    }

    if( m_iActionCount >= MAX_ACTION_NUM )
    {
        LOGERR("Can not add more action!");
        return -1;
    }

    TActionIndex iIndex = m_iActionCount;
    poAction->SetIndex( iIndex );
    m_aActionArray[m_iActionCount++] = poAction;

    if (this->GetTransactionID() != 0)
    {
        this->SetActionToken(iIndex);
    }

    printf("transaction id=(%u) name=(%s), add action id=(%d) name=(%s) token=(%lu)\n", this->GetTransactionID(),
            this->GetTransactionName(), iIndex+1, poAction->GetActionName(), poAction->GetToken());

    return iIndex;
}


void Transaction::SetActionToken(TActionIndex iIndex)
{
    if( m_aActionArray[iIndex]->ActionType() == ACTION_TYPE_COMPOSITE )
    {
        CompositeAction* poCompoAction = dynamic_cast<CompositeAction*>(m_aActionArray[iIndex]);
        assert(poCompoAction);
        int iCompoActionID = poCompoAction->GetIndex() + 1;
        assert(poCompoAction);
        int iActionCount = poCompoAction->GetActionCount();
        for( int i = 0; i < iActionCount; i++ )
        {
            IAction* poAction = poCompoAction->GetAction(i);
            poAction->SetToken( TransactionHelper::MakeActionToken( m_dwTransactionID, iCompoActionID, poAction->GetIndex()+1 ) );
            printf("CompositeAction id=(%u) set action id=(%d) token=(%lu)\n", iCompoActionID, poAction->GetIndex()+1, poAction->GetToken());
        }
    }else
    {
        m_aActionArray[iIndex]->SetToken( TransactionHelper::MakeActionToken( m_dwTransactionID, 0, m_aActionArray[iIndex]->GetIndex()+1 ) );
        IAction* poAction = m_aActionArray[iIndex];
        printf("TransAction id=(%u) set action id=(%d) token=(%lu)\n", this->GetTransactionID(), poAction->GetIndex()+1, poAction->GetToken());
    }
}


void Transaction::SetActionsToken()
{
    for( int i = 0; i < m_iActionCount; i++ )
    {
        this->SetActionToken(i);
    }
}


IAction* Transaction::GetAction(TActionIndex iIndex)
{
    if( iIndex < 0 )
    {
        assert( false );
        return NULL;
    }

    if( iIndex >= m_iActionCount )
    {
        m_bFinished = true;
        return NULL;
    }

    return m_aActionArray[iIndex];
}

void Transaction::NextStep()
{
    m_iCurStep++;
    if( m_iCurStep >= m_iActionCount )
    {
        m_bFinished = true;
    }
}


