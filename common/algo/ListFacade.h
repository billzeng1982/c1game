#ifndef _LIST_FACADE_H_
#define _LIST_FACADE_H_

#include "list.h"

#include <assert.h>

class CListFacade
{
public:
	struct list_head m_stListHead;
		
public:
	CListFacade() 
	{
		INIT_LIST_HEAD( &m_stListHead );
		m_iCurNum = 0;
	}

	~CListFacade(){}

	void Reset()
	{
		INIT_LIST_HEAD( &m_stListHead );
		m_iCurNum = 0;
	}

	bool AddTail( struct list_head* pstNode )
	{
		if( !pstNode )
	    {
	        return false;
	    }
    	list_add_tail( pstNode, &m_stListHead );
    	m_iCurNum++;
    	return true;
	}

	bool AddHead( struct list_head* pstNode )
	{
		if( !pstNode )
	    {
	        return false;
	    }
    	list_add( pstNode, &m_stListHead );
    	m_iCurNum++;
    	return true;
	}
	
	void Del( struct list_head* pstNode )
	{
	    if( !pstNode )
	    {
	        return;
	    }
		if( list_node_empty( pstNode ) )
		{
			return;
		}
		
	    list_del( pstNode );
	    --m_iCurNum;
	    if( m_iCurNum < 0 )
	    {
	        assert( false );
	        m_iCurNum = 0;
	    }
	}

	int GetCurNum( ) { return m_iCurNum; }

protected:
	int m_iCurNum;
};

#endif

