#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_

/*
	state machine 基类
*/

#include "define.h"
#include <assert.h>

template<typename TState, typename TContext>
class IStateMachine
{
public:
	IStateMachine() {}
	virtual ~IStateMachine() {}

	virtual TState* GetState( int iState ) = 0;
	
	virtual void ChangeState( TContext*, int iNewState );

	virtual void Update( TContext*, int iDeltaTime );

	virtual void HandleEvent( TContext*, int iEventID, void* pvEventPara );
};


template<typename TState, typename TContext>
void IStateMachine<TState, TContext>::ChangeState( TContext* poContext, int iNewState )
{
    int iCurState = poContext->GetState();

    if (iCurState == iNewState)
    {
        return;
    }

    TState* poCurState = this->GetState( iCurState );

    if (poCurState)
    {
        poCurState->ChangeState( poContext, iNewState );
    }
	else
    {
        /* 当前状态为NULL */

        // 设置新状态
        poContext->SetState( iNewState );
        TState* poNewState = this->GetState( iNewState );
        if( poNewState )
        {
            poNewState->Enter( poContext );
        }
    }
}

template<typename TState, typename TContext>
void IStateMachine<TState, TContext>::Update( TContext* poContext, int iDeltaTime )
{
	int iCurState = poContext->GetState();

	TState* poCurState = this->GetState( iCurState );

	if (poCurState)
	{
		poCurState->Update(poContext, iDeltaTime);
	}
}

template<typename TState, typename TContext>
void IStateMachine<TState, TContext>::HandleEvent( TContext* poContext, int iEventID, void* pvEventPara )
{
	if( NULL == poContext )
    {
        assert( false );
        return;
    }
    
    TState* poCurState = this->GetState( poContext->GetState() );

    if( poCurState )
    {
        poCurState->HandleEvent( poContext,  iEventID, pvEventPara );
    }
}

#endif

