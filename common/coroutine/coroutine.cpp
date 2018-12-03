#include "coroutine.h"
#include "../log/LogMacros.h"

// ������Э�̵�һ�α�����ִ��ʱ����ں���, ��Э���ڸ���ں����б�ִ��
static void CoroutineEntry( uint32_t low32, uint32_t hi32 )
{
    unsigned long ulptr = (unsigned long)low32 | ((unsigned long)hi32 << 32);

    Coroutine_t *pCo = (Coroutine_t *)ulptr;
    assert(pCo->m_iID>0);

    if( pCo->m_pfn )
    {
        pCo->m_pfn( pCo->m_pArg );  // m_pfn����������ܷ��� yield
    }

    // Э��ִ����
    pCo->m_iState = CO_FREE;

    CoroutineEnv* pCoEnv=pCo->m_pEnv;

    LOGRUN("Co(%d) finished", pCo->m_iID);

    pCoEnv->CoYield();
}


CoroutineEnv::CoroutineEnv()
{
    m_iCallStackLen = 0;
    m_iCoSeq = 0;
    bzero( m_pCallStack, sizeof(m_pCallStack) );
}

bool CoroutineEnv::Init( int iMaxCoNum )
{
    assert(MAX_CALL_STACK_SIZE>0);

    if( 0 == iMaxCoNum )
    {
        iMaxCoNum = DEFAULT_COROUTINE_NUM;
    }

    int iRet = m_oCoPool.CreatePool(iMaxCoNum+1); // ��Э��ռ��1��
    if( iRet < 0 )
    {
        return false;
    }

    // ��ʼ��Э�̻����еĲ��ֳ�Ա����
    this->m_iCallStackLen = 0;
    Coroutine_t* pMainCo = this->_CoCreate( NULL, NULL );  // ����ǰ�߳��е������İ�װ����Э��
    assert(pMainCo);
    pMainCo->m_bIsMain = true;
    m_pCallStack[m_iCallStackLen++] = pMainCo;

    return true;
}

// ����Э�̣�����Э��id��, <0����
Coroutine_t* CoroutineEnv::_CoCreate( pfn_coroutine_t pf, void* arg )
{
    Coroutine_t* pCo = m_oCoPool.NewData( );
    if( !pCo )
    {
        LOGERR("Cannot allocate Coroutine_t object!");
        return NULL;
    }

    pCo->Clear();

    pCo->m_pEnv = this;
    pCo->m_iState = CO_RUNNABLE;
    pCo->m_pfn = pf;
    pCo->m_pArg = arg;
    pCo->m_iID = this->_GetNextCoID();

    pCo->m_ctx.uc_stack.ss_sp = pCo->m_szRunStack;
    pCo->m_ctx.uc_stack.ss_size = DEFAULT_STACK_SZIE;
    pCo->m_ctx.uc_stack.ss_flags = 0;

    // add indexing
    m_oId2CoMap.insert( Id2CoMap_t::value_type( pCo->m_iID, pCo ) );

    return pCo;
}

// ִ��Э��
void CoroutineEnv::_CoResume( Coroutine_t* pCo  )
{
    assert( pCo );
    assert( ( m_iCallStackLen - 1 ) >= 0 );

    LOGRUN("Resume Co(%d)", pCo->m_iID);

    Coroutine_t* pCurrCo = m_pCallStack[ m_iCallStackLen - 1 ]; // ��ȡ��ǰ����ִ�е�Э��
    assert(pCurrCo->m_iID>0);
    assert(pCo->m_iID>0);

    switch( pCo->m_iState )
    {
        case CO_RUNNABLE:
        {
            // Э�̵�һ�α�����
            getcontext( &(pCo->m_ctx) );

            pCo->m_ctx.uc_link = NULL;//&(pCurrCo->m_ctx);

            // ��һ������,ͨ����ں���CoroutineEntry()Ϊ�乹��������
            unsigned long ulptr=(unsigned long)pCo;
            makecontext( &(pCo->m_ctx), (void (*)(void))(CoroutineEntry), 2, (uint32_t)ulptr, (uint32_t)(ulptr>>32) );
        }
        /*ע�����ﲻҪ�� break!! */

        case CO_SUSPEND:
        {
            assert( m_iCallStackLen < MAX_CALL_STACK_SIZE );
            pCo->m_iState = CO_RUNNING;
            m_pCallStack[ m_iCallStackLen++ ] = pCo;

            LOGRUN("Resume Co<%d>, m_iCallStackLen=<%d>", pCo->m_iID, m_iCallStackLen);
            assert(pCurrCo->m_iID>0);
            assert(pCo->m_iID>0);
	       swapcontext( &(pCurrCo->m_ctx), &(pCo->m_ctx) ); // ���浱ǰ�����ĵ�pCurrCo->m_ctx, ���л����µ�������pCo->m_ctx
	       assert(pCurrCo->m_iID>0);
            assert(pCo->m_iID>0);
        }
        break;

        default:
            break;
    }
}

void CoroutineEnv::CoYield()
{
    if(m_iCallStackLen < 2)
    {
        LOGERR("m_iCallStackLen in error state!!! <%d>, main thread, do not yield!", m_iCallStackLen);
        m_iCallStackLen = 1;
        return;
    }

    Coroutine_t *last = m_pCallStack[ m_iCallStackLen - 2 ];
    Coroutine_t *curr = m_pCallStack[ m_iCallStackLen - 1 ];
    
    m_iCallStackLen--;
    
    assert(last->m_iID>0);
    assert(curr->m_iID>0);

    if( CO_FREE == curr->m_iState )
    {
        this->_CoRelease( curr );
    }else
    {
        assert( CO_RUNNING == curr->m_iState );
        curr->m_iState = CO_SUSPEND; // �л���ջ��Э��״̬����ΪSUSPEND
    }

    LOGRUN("Yield current, change to Co(%d), m_iCallStackLen=%d", last->m_iID, m_iCallStackLen);

    // �л����ϴα��л���ȥ��Э��last. ע��������Ѿ�release��Coroutine, �˴���Ӱ��
    swapcontext(&(curr->m_ctx),&(last->m_ctx));
    assert(last->m_iID>0);
}

// ����Э�̣�����Э��ID�������� <0
int CoroutineEnv::CoCreate( pfn_coroutine_t pf, void* arg )
{
    Coroutine_t* pCo = this->_CoCreate(pf, arg);
    if( pCo )
    {
        assert( pCo->m_iID > 0 );
        return pCo->m_iID;
    }else
    {
        return -1;
    }
}

// ������ִ��Э��
int CoroutineEnv::StartCoroutine( pfn_coroutine_t pf, void* arg )
{
    Coroutine_t* pCo = this->_CoCreate(pf, arg);
    if( pCo )
    {
        assert( pCo->m_iID > 0 );
        this->_CoResume( pCo );
        return 0;
    }else
    {
        LOGERR("start coroutine failed!!");
        return -1;
    }
}

void CoroutineEnv::CoResume( int iCoID, CoResumeEvent_t* pResumeEvent  )
{
    Coroutine_t* pCo = this->_CoFind(iCoID);
    if( !pCo )
    {
        LOGERR("iCoID<%d> is invalid", iCoID);
        return;
    }

    assert( CO_FREE != pCo->m_iState );

    pCo->m_pResumeEvent = pResumeEvent;
    this->_CoResume( pCo );
}

// �ж��Ƿ�����Э�̶�ִ����
bool CoroutineEnv::CoFinished( )
{
    return m_oCoPool.GetFreeNum() == (m_oCoPool.GetMaxNum()-1); // ����Э��ռ��һ��
}

void CoroutineEnv::_CoRelease( Coroutine_t* pCo )
{
    if( !pCo )
    {
        return;
    }

    LOGRUN("Release Co(%d)", pCo->m_iID);

    this->_CoDelIndexing( pCo->m_iID );
    m_oCoPool.DeleteData( pCo );
}

int  CoroutineEnv::_GetNextCoID()
{
    m_iCoSeq++;
    if(0x7FFFFFFF == m_iCoSeq )
    {
        m_iCoSeq = 2; // 1����Э��ռ��
    }
    return m_iCoSeq;
}

Coroutine_t* CoroutineEnv::_CoFind( int iCoID )
{
    Id2CoMap_t::iterator it = m_oId2CoMap.find( iCoID );
    if( it != m_oId2CoMap.end() )
    {
        return it->second;
    }
    return NULL;
}

void CoroutineEnv::_CoDelIndexing( int iCoID )
{
    assert( iCoID > 0 );

    Id2CoMap_t::iterator it = m_oId2CoMap.find( iCoID );
    if( it != m_oId2CoMap.end() )
    {
        m_oId2CoMap.erase( it );
    }
    return;
}


