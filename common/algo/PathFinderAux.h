#ifndef _PATH_FINDER_AUX_H_
#define _PATH_FINDER_AUX_H_

/*
	path finder 辅助类
*/

#include "list.h"
#include "point.h"
#include "HashMap.h"
#include "PriorityQueue.h"
#include "DynMempool.h"

#define PATH_FINDER_NODE_INIT_NUM 	2000
#define PATH_FINDER_NODE_DELTA_NUM 	1000
#define MAX_OPEN_HMAP_SLOTS 		1000
#define MAX_CLOSH_HMAP_SLOTS 		1000

struct SPathFindNode
{
	SPoint2D m_stPos;

	int m_iF; // F = G + H
	int m_iG; // 从初始节点到n节点的实际代价, 注意是个累加值
	int m_iH;

	SPathFindNode* m_pstParent;

	struct list_head  m_stCloseListNode;

	struct hlist_node m_stOpenHashNode;
	struct hlist_node m_stCloseHashNode;

	int m_iOpenPriQPos; // 在Open优先队列中的Pos

	SPathFindNode()
	{
		this->Reset();
	}

	void Reset()
	{
		m_stPos.m_shPosX = 0;
		m_stPos.m_shPosY = 0;

		m_iF = 0;
		m_iG = 0;
		m_iH = 0;

		m_pstParent = NULL;

		INIT_LIST_NODE(&m_stCloseListNode);
		
		INIT_HLIST_NODE( &m_stOpenHashNode );
		INIT_HLIST_NODE( &m_stCloseHashNode );

		m_iOpenPriQPos = -1;
	}
};


struct SPathFindNodeLess
{
	bool operator() ( const SPathFindNode* rhs, const SPathFindNode* lhs ) const
	{ 
		//return ( rhs->m_iF < lhs->m_iF );

		if( rhs->m_iF < lhs->m_iF )
	 	{
	 		return true;
	 	}
		else if( rhs->m_iF == lhs->m_iF )
 		{
 			return ( rhs->m_iH < lhs->m_iH );
 		}
		else
		{
			return false;
		}
	}
};


class CPathFinderAux
{
public:
	CPathFinderAux() : m_oOpenPriQ( 1000 )
	{
		INIT_LIST_HEAD(&m_stCloseList);
	}
	
	~CPathFinderAux() {}

	bool Init();
	void Clear( );

	void PushToOpen( SPathFindNode* pstNode );
	void PushToClose( SPathFindNode* pstNode );
	SPathFindNode* PopFromOpen( );
	SPathFindNode* FindInOpen( SPoint2D& rstPos );
	SPathFindNode* FindInClose( SPoint2D& rstPos );
	void EraseFromCLose( SPathFindNode* pstNode );

	SPathFindNode* NewNode() { return m_oPathFindNodePool.Get( ); }

	bool OpenPriQEmpty() { return m_oOpenPriQ.Empty(); }
	void OpenPriQUpdate( SPathFindNode* pstNode );
		
private:
	DynMempool<SPathFindNode> m_oPathFindNodePool;
	
	// open表相关
	typedef PriorityQueue< SPathFindNode*, SPathFindNodeLess > OpenPriQ_t;
	OpenPriQ_t m_oOpenPriQ;
	CHashMap m_oOpenHMap;

	// close表相关
	struct list_head m_stCloseList; 
	CHashMap m_oCloseHMap;
};


void* GetPathFindNodePos_OpenHNode( struct hlist_node* pstHashNode );
void* GetPathFindNodePos_CloseHNode( struct hlist_node* pstHashNode );

#endif

