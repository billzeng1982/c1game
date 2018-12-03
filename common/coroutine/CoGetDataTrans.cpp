#include "CoDataFrame.h"
#include "CoGetDataTrans.h"

bool CoGetDataAction::_IsInMem( void* pResult )
{ 
    return m_poDataFrame->IsInMem(pResult); 
}

bool CoGetDataAction::_SaveInMem( void* pResult )
{ 
    return m_poDataFrame->SaveInMem(pResult); 
}

//处理异步返回结果的数据, 由框架调用
void CoGetDataAction::OnAsyncRspData( int iErrNo, void* pResult )
{
    do
    {
        if( iErrNo != CoDataFrame::ERRNO_SUCCESS )
        {
            m_iFiniFlag = iErrNo;
            break;
        }

        // 注意，如果数据已经在内存，不能再去存，覆盖数据，造成不一致性
        if( !this->_IsInMem(pResult) )
        {
            if( !this->_SaveInMem( pResult ) )
            {
                // save in mem error
                m_iFiniFlag = CoDataFrame::ERRNO_SAVE_IN_MEM_FAILED;
                break;
            }
        }

        m_iFiniFlag = 1;
    }while(0);
 
    return;
}


// -----------------------------------------------------------------------------------------------------

/*void CoGetDataTrans::AddAction( vector<CoGetDataAction*>& rvActions )
{
    for( int i = 0; i < (int)rvActions.size(); i++ )
    {
        assert( rvActions[i] );
        m_vActions.push_back( rvActions[i] );
    }
}*/

bool CoGetDataTrans::Execute( CoDataFrame* poDataFrame )
{
    assert( m_iCoroutineID > 0 );

    if( m_vActions.size() == 0 )
    {
        assert( false );
        return false;
    }

    bool bRet = true;

    for( int i = 0; i < (int)m_vActions.size(); i++ )
    {
        CoGetDataAction* poAction = m_vActions[i];
        uint64_t ulToken = CoDataFrame::MakeActionToken( m_dwTransID, i );
        poAction->SetToken( ulToken );
        poAction->SetDataFrame( poDataFrame );

        bRet = poAction->Execute( ); 
        if( !bRet )
        {
            LOGERR( "CoGetDataAction %d failed!", i );
            break;
        }
    }

    return bRet;
}

int CoGetDataTrans::GetFiniFlag()
{
    for( int i = 0; i < (int)m_vActions.size(); i++ )
    {
        CoGetDataAction* poAction = m_vActions[i];

        int iFiniFlag = poAction->GetFiniFlag();
        if( 0 == iFiniFlag  )
        {
            return 0;
        }
        if( iFiniFlag < 0 )
        {
            // errer occurs
            m_iFiniFlag = iFiniFlag;
            return m_iFiniFlag;
        }
    }

    m_iFiniFlag = 1; // finished
    
    return m_iFiniFlag;
}

CoGetDataAction* CoGetDataTrans::GetAction(int index)
{
    if( index < 0 || index >= (int)m_vActions.size())
    {
        assert( false );
        return NULL;
    }

    return m_vActions[index];
}


