#include <assert.h>

#include "HashMap_i.h"
#include "hash_func.h"

// 创建hash索引, pszBuff由外部传入，可能是堆上分配的内存，也可能是共享内存
bool CHashMap_i::HashCreate( key_t iShmKey, int iSlots, int iKeyLen, int iKeyType, int iNoRelease )
{
    if( iSlots < 0    ||
        iKeyLen <= 0  )
    {
        assert( false );
        return false;
    }

    /* 计算并分配内存 */
    int iBuffSize = sizeof(SHashMapCtl) + iSlots * sizeof(TLISTNODE);

    char* pszBuff = NULL;
    int iNew;
    if( iShmKey > 0 )
    {
        iNew = GetShm( (void**)&pszBuff, iShmKey, iBuffSize, (0666 | IPC_CREAT) );
        if( iNew < 0 )
        {
            assert(false);
            return false;
        }
    }else
    {
        pszBuff = new char[iBuffSize];
        if( NULL == pszBuff )
        {
            assert(false);
            return false;
        }
        iNew = 1;
    }

    return HashInit( pszBuff, iBuffSize, iNew, iShmKey, iSlots, iKeyLen, iKeyType, 0 );
}


bool CHashMap_i::HashInit( char* pszBuff, int iBuffSize, int iNew, key_t iShmKey, int iSlots, int iKeyLen, int iKeyType, int iNoRelease )
{   
    if( NULL == pszBuff ||
        iBuffSize <= 0  ||
		( iNew != 0 && iNew != 1 ) ||
		iSlots <=0 	||
		iKeyLen <=0	 )  
    {
        assert( false );
        return false;
    }

    m_pszBuff = pszBuff;
    m_pstHashMapCtl = (SHashMapCtl*)m_pszBuff;
    m_pTable   = (TLISTNODE*)( m_pszBuff + sizeof(SHashMapCtl) );

    if( iShmKey > 0 && 0 == iNew )
    {
        // pszBuff为已存在shm
        if( m_pstHashMapCtl->m_iKeyLen != iKeyLen ||
            m_pstHashMapCtl->m_iShmKey != iShmKey ||
            m_pstHashMapCtl->m_iSlots  != iSlots )
        {
            assert( false );
            return false;
        }else
        {
            return true;
        }
    }

    // 初始化操作
    m_pstHashMapCtl->m_iElems   = 0;
    m_pstHashMapCtl->m_iSlots   = iSlots;
    m_pstHashMapCtl->m_iNoRelease = iNoRelease;
    m_pstHashMapCtl->m_iShmKey  = iShmKey;
    m_pstHashMapCtl->m_iKeyLen  = iKeyLen;
    m_pstHashMapCtl->m_iKeyType = iKeyType;
    
    for( int i = 0; i < iSlots; i++ )
    {
        TLIST_INIT( &m_pTable[i] );
    }
    
    return true;
}


// 设置函数指针
void CHashMap_i::SetCallback(HashCompareCb fHashCompare, void* (*fHashGetKey)(TLISTNODE*) )
{
    assert( fHashCompare && fHashGetKey );

    m_fHashCompare = fHashCompare;
    m_fHashGetKey  = fHashGetKey;
}


int  CHashMap_i::HashAdd( TLISTNODE *pHNode,  void* key )
{
    if( NULL == pHNode || NULL == key )
    {
        assert( false );
        return EHASH_MAP_FAIL;
    }

    int iSlot = _HashChoose( key );
    assert( iSlot >=0 && iSlot < m_pstHashMapCtl->m_iSlots );

    TLISTNODE* pHead = &m_pTable[iSlot];

    if( _HashFind( pHead, key ) )
    {
        // key duplicate
        return EHASH_MAP_DUP;
    }

    // insert_to_head
    TLIST_INSERT_NEXT( pHead, pHNode );
    m_pstHashMapCtl->m_iElems++;
    
    return EHASH_MAP_SUCC;
}


TLISTNODE* CHashMap_i::HashFind( void* key )
{
    if( NULL == key )
    {
        assert(false);
        return NULL;
    }

    int iSlot = _HashChoose( key );
    assert( iSlot >=0 && iSlot < m_pstHashMapCtl->m_iSlots );

    TLISTNODE* pHead = &m_pTable[iSlot];
    return _HashFind( pHead, key );
}


int CHashMap_i::HashDelByKey( void* key )
{
    if( NULL == key )
    {
        assert( false );
        return EHASH_MAP_FAIL;
    }

    int iSlot = _HashChoose( key );
    assert( iSlot >=0 && iSlot < m_pstHashMapCtl->m_iSlots );

    TLISTNODE* pHead = &m_pTable[iSlot];
    TLISTNODE* pHNode = _HashFind( pHead, key );

    if( pHNode )
    {
        TLIST_DEL( pHNode );
        --m_pstHashMapCtl->m_iElems;
        
    }

    return EHASH_MAP_SUCC;
}


// 比较暴力 
void CHashMap_i::HashDel( TLISTNODE* pHNode )
{
    if( pHNode )
    {
        TLIST_DEL( pHNode );
        --m_pstHashMapCtl->m_iElems;
    }
}


void CHashMap_i::Detach()
{
    if( NULL == m_pszBuff )
    {
        assert(false);
        return;
    }

    if( !m_pstHashMapCtl->m_iNoRelease )
    {
        if( m_pstHashMapCtl->m_iShmKey )
        {
            shmdt( (void*)m_pszBuff );
        }else
        {
            shmdt( (void*)m_pszBuff );
        }

        m_pszBuff = NULL;
		m_pstHashMapCtl = NULL;
        m_pTable = NULL;
    }
}

// 选择哪一个slot
int CHashMap_i::_HashChoose( void* key )
{
    assert( key );
    return (int)zend_inline_hash_func( key, m_pstHashMapCtl->m_iKeyLen, m_pstHashMapCtl->m_iSlots );
}


TLISTNODE* CHashMap_i::_HashFind(  TLISTNODE* pHead, void* key )
{
    assert( pHead && key );
    TLISTNODE* pstPos = NULL;
    TLISTNODE* pstNext = NULL;

    TLIST_FOR_EACH_SAFE( pstPos, pstNext, pHead )
    {
        if( m_fHashCompare( m_fHashGetKey(pstPos), key ) )
            return pstPos;
    }

    return NULL;
}

void CHashMap_i::Clear()
{   
    m_pstHashMapCtl->m_iElems   = 0;
       
    for( int i = 0; i < m_pstHashMapCtl->m_iSlots; i++ )
    {
        TLIST_INIT( &m_pTable[i] );
    }
}

