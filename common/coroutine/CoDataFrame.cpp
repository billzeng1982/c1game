#include "CoDataFrame.h"
#include "../sys/GameTime.h"

CoDataFrame::CoDataFrame()
{
    m_pTransTimeoutQ = NULL;
    m_poCoroutineEnv = NULL;
}

uint64_t CoDataFrame::MakeActionToken( uint32_t dwTransID, int iActionIdx )
{
    uint64_t ultoken = 0;
    ultoken = ( ((uint64_t)dwTransID) << 32 ) | iActionIdx;
    return ultoken;
}


void CoDataFrame::ParseActionToken(const uint64_t ulToken, uint32_t& dwTransID, int& iActionIdx )
{
    dwTransID       = (uint32_t)(ulToken >> 32);
    iActionIdx      = (int)( ulToken & 0xFFFFFFFF );
}


bool CoDataFrame::BaseInit( int iMaxTrans )
{
    if( !m_oTransMgr.Init(iMaxTrans) )\
    {
        return false;
    }

    m_pTransTimeoutQ = new IndexedPriorityQ< CoGetDataTrans*, uint32_t >( \
        iMaxTrans, new Comparer<CoGetDataTrans*>(), new GetItemKey<CoGetDataTrans*, uint32_t> );
    if( !m_pTransTimeoutQ )
    {
        LOGERR( "create transaction timeout queue failed!" );
        return false;
    }

    return true;
}

void CoDataFrame::Update()
{
    int iCount = 0;
    
    //��������Ƿ�ʱ
    while( !m_pTransTimeoutQ->Empty() && iCount < UPT_TIMEOUT_Q_PER_LOOP )
    {
        CoGetDataTrans* poTrans= m_pTransTimeoutQ->Top();
        if( TvBefore( poTrans->GetTimeout(), CGameTime::Instance().GetCurrTime() ) )
        {
            LOGERR("CoGetDataTrans <%u> timeout!", poTrans->GetTransID());
        
            // timeout
            ++iCount;
            poTrans->SetFiniFlag( ERRNO_TIMEOUT );
            int iCoID = poTrans->GetCoroutineID();

            m_pTransTimeoutQ->Pop();
            
            CoResumeEvent_t stResumeEvent;
            stResumeEvent.m_iEventID = ERRNO_TIMEOUT;
            stResumeEvent.m_pEventArg = NULL;
            m_poCoroutineEnv->CoResume( iCoID, &stResumeEvent ); // ��ʱ�¼����л�Э��
        }
        else
        {
            break;
        }
    }
}

void CoDataFrame::_ReleaseGetDataTrans( CoGetDataTrans* poTrans )
{
    assert( poTrans );

    m_pTransTimeoutQ->Erase( poTrans->GetTransID() );

    int iActionCount = poTrans->GetActionCount();
    for( int i = 0; i < iActionCount; i++ )
    {
        CoGetDataAction* poAction = poTrans->GetAction(i);
        this->_ReleaseGetDataAction( poAction );
    }

    m_oTransMgr.Release( poTrans );
}

/*  	�첽get data action��ɣ����ⲿ�߼�(��Э��)����, ���ݻ�ȡ�ɹ����л�����Э��
	pvResultΪ�첽ȡ�õ�����, ����Ϊ��
*/
void CoDataFrame::AsyncGetDataDone( uint64_t ulToken, int iErrNo, void* pvResult )
{
    //m_poCoroutineEnv->DebugCheck();

    uint32_t dwTransID = 0;
    int iActionIdx = 0;
    ParseActionToken( ulToken, dwTransID, iActionIdx );

    //m_poCoroutineEnv->DebugCheck();

    CoGetDataTrans* poTrans = m_oTransMgr.Find( dwTransID );
    if( !poTrans )
    {
        LOGERR("Can not find CoGetDataTrans, trans id <%u>", dwTransID);
        return;
    }

    CoGetDataAction* poAction = poTrans->GetAction(iActionIdx);
    assert( poAction );
    //m_poCoroutineEnv->DebugCheck();

    poAction->OnAsyncRspData( iErrNo, pvResult );

    //m_poCoroutineEnv->DebugCheck();

    CoResumeEvent_t stResumeEvent;
    int iCoID = poTrans->GetCoroutineID();
    int iRet = poTrans->GetFiniFlag();
    if( 0 == iRet )
    {
        // δ���
        return;
    }else if( iRet < 0 )
    {
        // error occurs
        LOGERR("CoGetDataTrans <%u> failed. errorno <%d>", dwTransID, iRet);
        
        stResumeEvent.m_iEventID = iRet; // errorno
        stResumeEvent.m_pEventArg = NULL;
        m_poCoroutineEnv->CoResume( iCoID, &stResumeEvent );
    }else
    {
        // success
        assert( iRet > 0 );

        stResumeEvent.m_iEventID = ERRNO_SUCCESS;
        stResumeEvent.m_pEventArg = NULL;
        m_poCoroutineEnv->CoResume( iCoID, &stResumeEvent ); // ���ݻ�ȡ�ɹ��л�����Э��
    }
}

int CoDataFrame::GetData(void* key, OUT void*& rpData)
{
    rpData = CoDataFrame::GetData(key);
    if (rpData == NULL)
    {
        CoResumeEvent_t* pstResumeEvent = m_poCoroutineEnv->GetCurrResumeEvent();
        if (pstResumeEvent != NULL)
        {
            return pstResumeEvent->m_iEventID;
        }
        return ERRNO_GET_DATA_FAILED;
    }
    return ERRNO_SUCCESS;
}

// ��Э���߼�����
void* CoDataFrame::GetData( void* key)
{
    void* pvData = this->_GetDataInMem( key );
    if( pvData )
    {
        // �ڴ��д��ڣ�ֱ�ӷ���
        return pvData;
    }

    CoGetDataTrans* poTrans = m_oTransMgr.New();
    if( !poTrans )
    {
        return NULL;
    }

    CoGetDataAction* poAction = this->_CreateGetDataAction( key );
    if( !poAction )
    {
        LOGERR("Create CoGetDataAction failed!");
        m_oTransMgr.Release(poTrans);

        return NULL;
    }
    poTrans->AddAction( poAction );

    struct timeval stTimeout = *(CGameTime::Instance().GetCurrTime());
    stTimeout = *(TvAddMs( &stTimeout, DEFAULT_TIMEOUT_TIME_MS ));
    poTrans->SetTimeout( stTimeout );
    poTrans->SetCoroutineID( m_poCoroutineEnv->GetCurrCoID() );
    
    this->_AsyncGetData( poTrans );

    // ------------------------ resume -----------------------
    LOGRUN("Get one data resume ...");
    // �ͷ� CoGetDataTrans*
    this->_ReleaseGetDataTrans( poTrans );
        
    CoResumeEvent_t* pstResumeEvent = m_poCoroutineEnv->GetCurrResumeEvent();
    if( pstResumeEvent->m_iEventID != ERRNO_SUCCESS )
    {
        LOGERR("Get data failed, errno <%d>", pstResumeEvent->m_iEventID);
        //failed
        return NULL;
    }

    return this->_GetDataInMem(key);
}

/* һ����Ҫ��ȡ������ݵ����
    ���ܲ����������ڴ棬���ֲ���
*/
int CoDataFrame::GetData( vector<void*>& vKey, vector<void*>& vReturnedData )
{
    vector<void*> vNotInMemKey;

    vReturnedData.clear();
    
    void* pvData = NULL;
    for( int i = 0; i < (int)vKey.size(); i++ )
    {
        pvData = this->_GetDataInMem( vKey[i] );
        if( pvData )
        {
            vReturnedData.push_back(pvData);
        }else
        {
            vNotInMemKey.push_back( vKey[i] );
        }
    }

    if( vNotInMemKey.size() == 0 )
    {
        return CoDataFrame::ERRNO_SUCCESS;
    }

    vReturnedData.clear();

    CoGetDataTrans* poTrans = m_oTransMgr.New();
    if( !poTrans )
    {
        return ERRNO_GET_DATA_FAILED;
    }

    for( int i = 0; i < (int)vNotInMemKey.size(); i++ )
    {
        CoGetDataAction* poAction = this->_CreateGetDataAction( vNotInMemKey[i] );
        if( !poAction )
        {
            LOGERR("Create CoGetDataAction failed!");
            this->_ReleaseGetDataTrans(poTrans);

            return ERRNO_GET_DATA_FAILED;
        }
        poTrans->AddAction( poAction );
    }

    struct timeval stTimeout = *(CGameTime::Instance().GetCurrTime());
    stTimeout = *(TvAddMs( &stTimeout, DEFAULT_TIMEOUT_TIME_MS ));
    poTrans->SetTimeout( stTimeout );
    poTrans->SetCoroutineID( m_poCoroutineEnv->GetCurrCoID() );
    
    this->_AsyncGetData( poTrans );

    // ------------------------ resume -----------------------
    // �ͷ� CoGetDataTrans*
    this->_ReleaseGetDataTrans( poTrans );
        
    CoResumeEvent_t* pstResumeEvent = m_poCoroutineEnv->GetCurrResumeEvent();
    if( pstResumeEvent->m_iEventID != ERRNO_SUCCESS )
    {
        //failed
        return pstResumeEvent->m_iEventID;
    }
    
    for( int i = 0; i < (int)vKey.size(); i++ )
    {
        pvData = this->_GetDataInMem( vKey[i] );
        if( pvData )
        {
            vReturnedData.push_back(pvData);
        }else
        {
            vReturnedData.clear();
            return ERRNO_GET_DATA_FAILED;
        }
    }

    return ERRNO_SUCCESS;
}

// �����첽��ȡ���ݣ����л�����Э��
bool CoDataFrame::_AsyncGetData( CoGetDataTrans* poTrans )
{
    if( !poTrans->Execute( this ) )
    {
        // failed
        this->_ReleaseGetDataTrans( poTrans );
        return false;
    }

    m_pTransTimeoutQ->Push( poTrans ); // ���볬ʱ����
    LOGRUN("Yield, switch to main coroutine");
    m_poCoroutineEnv->CoYield();

    return true;
}

