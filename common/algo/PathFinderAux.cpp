#include "PathFinderAux.h"


void* GetPathFindNodePos_OpenHNode( struct hlist_node* pstHashNode )
{
    if( NULL == pstHashNode )
    {
        assert( false );
        return NULL;
    }

    SPathFindNode* pstPathFindNode = hlist_entry( pstHashNode, SPathFindNode, m_stOpenHashNode );
  
    return (void*)&(pstPathFindNode->m_stPos);
}


void* GetPathFindNodePos_CloseHNode( struct hlist_node* pstHashNode )
{
    if( NULL == pstHashNode )
    {
        assert( false );
        return NULL;
    }

    SPathFindNode* pstPathFindNode = hlist_entry( pstHashNode, SPathFindNode, m_stCloseHashNode );
  
    return (void*)&(pstPathFindNode->m_stPos);
}


bool CPathFinderAux::Init( )
{
    if( !m_oPathFindNodePool.Init( PATH_FINDER_NODE_INIT_NUM, PATH_FINDER_NODE_DELTA_NUM) )
    {
        return false;
    }


    if( !m_oOpenHMap.HashCreate( MAX_OPEN_HMAP_SLOTS, sizeof(SPoint2D) ) )
    {
        return false;
    }

    m_oOpenHMap.SetCallback( HashCmp_Point2D,  GetPathFindNodePos_OpenHNode );

    if( !m_oCloseHMap.HashCreate( MAX_CLOSH_HMAP_SLOTS, sizeof(SPoint2D) ) )
    {
        return false;
    }

    m_oCloseHMap.SetCallback( HashCmp_Point2D, GetPathFindNodePos_CloseHNode );

    INIT_LIST_HEAD(&m_stCloseList);

    return true;
}


void CPathFinderAux::Clear( )
{
    SPathFindNode* pstPathFindNode = NULL;
    
    // 清空open表
    while( !m_oOpenPriQ.Empty() )
    {
        pstPathFindNode = m_oOpenPriQ.Top( );
        m_oOpenPriQ.Pop();

        m_oOpenHMap.HashDelByKey( &(pstPathFindNode->m_stPos) );

        m_oPathFindNodePool.Release( pstPathFindNode );
    }
    assert( 0 == m_oOpenHMap.GetHashElems() );

    // 清空close表
    SPathFindNode* pstPathFindNodeNext = NULL;
    pstPathFindNode = NULL;
    struct list_head* pstHead = &m_stCloseList;
    list_for_each_entry_safe( pstPathFindNode, pstPathFindNodeNext, pstHead, m_stCloseListNode )
    {
        list_del( &(pstPathFindNode->m_stCloseListNode) );
        m_oCloseHMap.HashDelByKey( &(pstPathFindNode->m_stPos) );

        m_oPathFindNodePool.Release( pstPathFindNode );
    }
    assert( 0 == m_oCloseHMap.GetHashElems() );
}


void CPathFinderAux::PushToOpen( SPathFindNode* pstNode )
{
    int iRet =m_oOpenHMap.HashAdd( &(pstNode->m_stOpenHashNode), &(pstNode->m_stPos ) );
    if( EHASH_MAP_SUCC == iRet )
    {
        m_oOpenPriQ.Push( pstNode );
    }
}


void CPathFinderAux::PushToClose( SPathFindNode* pstNode )
{
    int iRet = m_oCloseHMap.HashAdd( &(pstNode->m_stCloseHashNode), &(pstNode->m_stPos ) );
    if( EHASH_MAP_SUCC == iRet )
    {
        list_add_tail( &(pstNode->m_stCloseListNode ), &m_stCloseList );
    }
}


SPathFindNode* CPathFinderAux::PopFromOpen( )
{
    if( m_oOpenPriQ.Empty() )
    {
        return NULL;
    }
    SPathFindNode* pstNode = m_oOpenPriQ.Top();
    m_oOpenPriQ.Pop();
    m_oOpenHMap.HashDel( &(pstNode->m_stOpenHashNode) );

    return pstNode;
}


SPathFindNode* CPathFinderAux::FindInOpen( SPoint2D& rstPos )
{
    struct hlist_node* pstHNode = m_oOpenHMap.HashFind( (&rstPos) );
    if( !pstHNode)
    {
        return NULL;
    }

    return hlist_entry(pstHNode, SPathFindNode, m_stOpenHashNode);
}


SPathFindNode* CPathFinderAux::FindInClose( SPoint2D& rstPos )
{
    struct hlist_node* pstHNode = m_oCloseHMap.HashFind( &rstPos );
    if( !pstHNode )
    {
        return NULL;
    }

    return hlist_entry( pstHNode, SPathFindNode, m_stCloseHashNode );
}


void CPathFinderAux::EraseFromCLose( SPathFindNode* pstNode )
{
    list_del( &(pstNode->m_stCloseListNode) );
    m_oCloseHMap.HashDel( &(pstNode->m_stCloseHashNode) );
}


void CPathFinderAux::OpenPriQUpdate( SPathFindNode* pstNode )
{
    if( !pstNode )
    {
        return;
    }

    int iIdx = m_oOpenPriQ.IndexOf( pstNode );
    if( iIdx >= 0 )
    {
        m_oOpenPriQ.Update( iIdx );
    }
}

