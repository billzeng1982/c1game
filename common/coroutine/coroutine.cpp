#include "coroutine.h"
#include "../log/LogMacros.h"

// 所有新协程第一次被调度执行时的入口函数, 新协程在该入口函数中被执行
static void CoroutineEntry( uint32_t low32, uint32_t hi32 )
{
    unsigned long ulptr = (unsigned long)low32 | ((unsigned long)hi32 << 32);

    Coroutine_t *pCo = (Coroutine_t *)ulptr;
    assert(pCo->m_iID>0);

    if( pCo->m_pfn )
    {
        pCo->m_pfn( pCo->m_pArg );  // m_pfn函数里面可能发生 yield
    }

    // 协程执行完
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

    int iRet = m_oCoPool.CreatePool(iMaxCoNum+1); // 主协程占用1个
    if( iRet < 0 )
    {
        return false;
    }

    // 初始化协程环境中的部分成员变量
    this->m_iCallStackLen = 0;
    Coroutine_t* pMainCo = this->_CoCreate( NULL, NULL );  // 将当前线程中的上下文包装成主协程
    assert(pMainCo);
    pMainCo->m_bIsMain = true;
    m_pCallStack[m_iCallStackLen++] = pMainCo;

    return true;
}

// 创建协程，返回协程id号, <0出错
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

// 执行协程
void CoroutineEnv::_CoResume( Coroutine_t* pCo  )
{
    assert( pCo );
    assert( ( m_iCallStackLen - 1 ) >= 0 );

    LOGRUN("Resume Co(%d)", pCo->m_iID);

    Coroutine_t* pCurrCo = m_pCallStack[ m_iCallStackLen - 1 ]; // 获取当前正在执行的协程
    assert(pCurrCo->m_iID>0);
    assert(pCo->m_iID>0);

    switch( pCo->m_iState )
    {
        case CO_RUNNABLE:
        {
            // 协程第一次被调度
            getcontext( &(pCo->m_ctx) );

            pCo->m_ctx.uc_link = NULL;//&(pCurrCo->m_ctx);

            // 第一次运行,通过入口函数CoroutineEntry()为其构造上下文
            unsigned long ulptr=(unsigned long)pCo;
            makecontext( &(pCo->m_ctx), (void (*)(void))(CoroutineEntry), 2, (uint32_t)ulptr, (uint32_t)(ulptr>>32) );
        }
        /*注意这里不要加 break!! */

        case CO_SUSPEND:
        {
            assert( m_iCallStackLen < MAX_CALL_STACK_SIZE );
            pCo->m_iState = CO_RUNNING;
            m_pCallStack[ m_iCallStackLen++ ] = pCo;

            LOGRUN("Resume Co<%d>, m_iCallStackLen=<%d>", pCo->m_iID, m_iCallStackLen);
            assert(pCurrCo->m_iID>0);
            assert(pCo->m_iID>0);
	       swapcontext( &(pCurrCo->m_ctx), &(pCo->m_ctx) ); // 保存当前上下文到pCurrCo->m_ctx, 并切换到新的上下文pCo->m_ctx
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
        curr->m_iState = CO_SUSPEND; // 切换出栈的协程状态设置为SUSPEND
    }

    LOGRUN("Yield current, change to Co(%d), m_iCallStackLen=%d", last->m_iID, m_iCallStackLen);

    // 切换到上次被切换出去的协程last. 注意如果是已经release的Coroutine, 此处不影响
    swapcontext(&(curr->m_ctx),&(last->m_ctx));
    assert(last->m_iID>0);
}

// 创建协程，返回协程ID，出错返回 <0
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

// 创建并执行协程
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

// 判断是否所有协程都执行完
bool CoroutineEnv::CoFinished( )
{
    return m_oCoPool.GetFreeNum() == (m_oCoPool.GetMaxNum()-1); // 因主协程占用一个
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
        m_iCoSeq = 2; // 1被主协程占用
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


