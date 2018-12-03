#include <assert.h>
#include <string.h>
#include "HashMap.h"

CHashMap::CHashMap()
{
    m_pTable = NULL;
    m_fHashCompare = NULL;
    m_fHashGetKey = NULL;
    
    bzero( &m_stHashMapCtl, sizeof(m_stHashMapCtl) );
}

CHashMap::~CHashMap()
{
    if(m_pTable)
    {
        delete m_pTable;
        m_pTable = NULL;
    }
}


bool CHashMap::HashCreate( int iSlots, int iKeyLen, int iKeyType )
{
    m_pTable = new struct hlist_head[ iSlots ];
    if( NULL == m_pTable )
    {
        assert( false );
        return false;
    }

    m_stHashMapCtl.m_iSlots = iSlots;
    m_stHashMapCtl.m_iKeyLen = iKeyLen;
    m_stHashMapCtl.m_iKeyType = iKeyType;

    HashInit();

    return true;
}


void CHashMap::SetCallback( HashCompareCb fHashCompare, void* (*fHashGetKey)(struct hlist_node*) )
{
    assert( fHashCompare && fHashGetKey );

    m_fHashCompare = fHashCompare;
    m_fHashGetKey  = fHashGetKey;
}


/* clears the hash */
void CHashMap::HashInit( )
{
	m_stHashMapCtl.m_iShmKey = 0;

	for ( int i = 0 ; i < m_stHashMapCtl.m_iSlots; i++ )
		INIT_HLIST_HEAD( &m_pTable[i] );
}


/* adds hash node to the hashtable. */
int CHashMap::HashAdd( struct hlist_node* pHNode, void* key )
{
    if( NULL == pHNode || NULL == key )
    {
        assert(false);
        return EHASH_MAP_FAIL;
    }

    int iSlot = _HashChoose( key );
    assert( iSlot >= 0 && iSlot < m_stHashMapCtl.m_iSlots );
    struct hlist_head* pHead = &m_pTable[ iSlot ];

    if( _HashFind( pHead, key ) )
    {
        // key duplicate
        return EHASH_MAP_DUP;
    }
    
    // insert to head
    hlist_add_head( pHNode, pHead );
    ++m_stHashMapCtl.m_iElems;

    return EHASH_MAP_SUCC;
}


/* find hash node in the hashtable */
struct hlist_node* CHashMap::HashFind( void* key )
{
    if( NULL == key )
    {
        assert(false);
        return NULL;
    }

    int iSlot = _HashChoose( key );
    assert( iSlot >= 0 && iSlot < m_stHashMapCtl.m_iSlots );

    struct hlist_head* pHead = &m_pTable[iSlot];
    return _HashFind( pHead, key );
}

/* delete hlist_node from hashtable */
int CHashMap::HashDelByKey( void* key )
{
    if( NULL == key )
    {
        assert(false);
        return EHASH_MAP_FAIL;
    }

    struct hlist_node* pHNode = HashFind( key );
    if( pHNode )
    {
        hlist_del( pHNode );
        --m_stHashMapCtl.m_iElems;
    }

    return EHASH_MAP_SUCC;
}


// ±È½Ï±©Á¦ 
void CHashMap::HashDel( struct hlist_node* pHNode )
{
    if( pHNode && !hlist_node_empty(pHNode))
    {
        hlist_del( pHNode );
        --m_stHashMapCtl.m_iElems;
    }
}


struct hlist_node* CHashMap::_HashFind(  struct hlist_head* pHead, void* key )
{
    struct hlist_node* pos = NULL;

    hlist_for_each(pos, pHead )
    {
        if( m_fHashCompare( m_fHashGetKey(pos), key ) )
            return pos;
    }

    return NULL;
}


