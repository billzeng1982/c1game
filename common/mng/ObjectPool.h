#pragma once

/*
	������Ҫʹ�ñ�pool�������ĸ��������IObject
	���󲻻����������������Ƿŵ����б���
	����object pool�ǵ���
	֧�����������Դ��������

	���������ݲ����ٿ��нڵ�
*/

#include "object.h"
#include "list.h"
#include <assert.h>
#include <map>

struct PoolNodeInfo
{
	unsigned short m_wType;
	int m_iMaxObjNum; 	 // �������, Ϊ0������
	int m_iAllocatedNum; // �ѷ����������( free + used )
	int m_iCurUsedNum; 
	unsigned int m_dwMemBytes; // �����Ͷ����ڴ�ռ��bytes

	PoolNodeInfo()
	{
		m_wType = 0;
		m_iMaxObjNum = 0;
		m_iAllocatedNum = 0;
		m_iCurUsedNum = 0;
		m_dwMemBytes = 0;
	}
};

struct PoolNode
{
	PoolNodeInfo 	 m_stPoolNodeInfo;
	struct list_head m_stFreeListHead;
	
	PoolNode()
	{
		INIT_LIST_HEAD( &m_stFreeListHead );
	}
};

template< class T >
class ObjectPool 
{
protected:
	typedef std::map< unsigned int /*ObjID*/, IObject*> UsedObjMap_t;
	typedef std::map< unsigned short /*type*/, PoolNode* > PoolNodeMap_t;

	typedef IObject* (T::*mfpObjCreator_t)();
	typedef std::map<unsigned short /*TypeID*/, mfpObjCreator_t>  ObjCreatorMap_t;

	#define OBJ_CREATOR( ClassType ) \
		IObject* Create##ClassType( ) \
		{ \
			ClassType* pObj = new ClassType( ); \
			m_dwMemBytes += sizeof(*pObj); \
			pObj->SetMemBytes((unsigned short)sizeof(*pObj)); \
			return pObj; \
		}

	#define REGISTER_OBJ_CREATOR( TypeID, PoolType, ClassType ) \
		m_oObjCreatorMap.insert( ObjCreatorMap_t::value_type( TypeID, &PoolType::Create##ClassType ) );

public:
	ObjectPool() : m_dwObjIDSeq(0), m_dwMemBytes(0) {}
	virtual ~ObjectPool();

	IObject* Get( unsigned short wType );
	// ����������أ�������
	void Release( IObject* pObj );
	IObject* Find( unsigned int dwObjID );

	unsigned int GetMemBytes() { return m_dwMemBytes; }

	void SetMaxObjNum( unsigned short wType, int iMaxNum );

	void DumpPoolNodeInfo( unsigned short wType, PoolNodeInfo& rPoolNodeInfo );

private:
	T* Me()
	{
		return dynamic_cast<T*>(this);
	}

	PoolNode* _GetPoolNode( unsigned short wType );

protected:
	unsigned int m_dwObjIDSeq; // object id sequence
	unsigned int m_dwMemBytes; // ��pool���ж�����ڴ�ռ��

	UsedObjMap_t m_oUsedObjMap;
	PoolNodeMap_t m_oPoolNodeMap;
	ObjCreatorMap_t	m_oObjCreatorMap;
};


template <class T>
ObjectPool<T>::~ObjectPool()
{
	PoolNodeMap_t::iterator oPoolNodeIter = m_oPoolNodeMap.begin();
	for( ; oPoolNodeIter != m_oPoolNodeMap.end(); oPoolNodeIter++ )
	{
		PoolNode* pstPoolNode = oPoolNodeIter->second;

		IObject* pCurObj = NULL;
		IObject* pNxtObj = NULL;
	
		list_for_each_entry_safe( pCurObj, pNxtObj, &(pstPoolNode->m_stFreeListHead), m_stListNode )
		{
			list_del( &(pCurObj->m_stListNode) );
			delete pCurObj;
		}

		delete pstPoolNode;
	}
	m_oPoolNodeMap.clear();

	UsedObjMap_t::iterator oUsedIter = m_oUsedObjMap.begin();
	for( ; oUsedIter != m_oUsedObjMap.end(); oUsedIter++ )
	{
		oUsedIter->second->Clear();
		delete oUsedIter->second;
	}
	m_oUsedObjMap.clear();
}


template <class T>
IObject* ObjectPool<T>::Get( unsigned short wType )
{
	if( 0 == wType )
	{
		assert( false );
		return NULL;
	}

	PoolNode* pstPoolNode = this->_GetPoolNode(wType);
	assert( pstPoolNode );

	IObject* pObj = NULL;

	if( !list_empty(&pstPoolNode->m_stFreeListHead) )
	{
		// ��ȡ���� object
		pObj = list_entry( (pstPoolNode->m_stFreeListHead.next), IObject, m_stListNode);
		list_del( &(pObj->m_stListNode) );
	}
	else
	{
		// �޿��� object
		if( pstPoolNode->m_stPoolNodeInfo.m_iMaxObjNum != 0 && 
			pstPoolNode->m_stPoolNodeInfo.m_iAllocatedNum >= pstPoolNode->m_stPoolNodeInfo.m_iMaxObjNum )
		{
			// ������Դ��������
			pObj = NULL;
		}else
		{
			// new ��Դ
			typename ObjCreatorMap_t::iterator oObjCreatorIter = m_oObjCreatorMap.find( wType );
			if( oObjCreatorIter != m_oObjCreatorMap.end() )
			{
				pObj = ( Me()->*(oObjCreatorIter->second) )();  // ��Ҫ����������ת�� Me()
				if( pObj )
				{
					pstPoolNode->m_stPoolNodeInfo.m_dwMemBytes += pObj->GetMemBytes();
					pstPoolNode->m_stPoolNodeInfo.m_iAllocatedNum++;
				}
			}
		}
	}

	if( pObj )
	{
		pObj->SetObjType( wType );
		++m_dwObjIDSeq;
		if( 0 == m_dwObjIDSeq )
		{
			m_dwObjIDSeq = 1; // ����
		}
		pObj->SetObjID( m_dwObjIDSeq );

		// ����used map,����find
		m_oUsedObjMap.insert( UsedObjMap_t::value_type( pObj->GetObjID(), pObj ) );
		
		pstPoolNode->m_stPoolNodeInfo.m_iCurUsedNum++;
	}
	
	return pObj;
}


// ����������أ�������
template <class T>
void ObjectPool<T>::Release( IObject* pObj )
{
	if( !pObj )
	{
		return;
	}

	if( pObj->GetObjID() == 0 )
	{
		assert( false );
		return;
	}

	PoolNodeMap_t::iterator it = m_oPoolNodeMap.find( pObj->GetObjType() );
	if( it == m_oPoolNodeMap.end() )
	{
		// ������pool node!
		assert( false );
		return;
	}

	PoolNode* pstPoolNode = it->second;

	// ��used map��erase
	UsedObjMap_t::iterator oUsedIter = m_oUsedObjMap.find( pObj->GetObjID() );
	m_oUsedObjMap.erase( oUsedIter );

	// ����free list
	if( !list_node_empty( &pObj->m_stListNode ) )
	{
		list_del( &pObj->m_stListNode );
	}
	list_add( &(pObj->m_stListNode), &(pstPoolNode->m_stFreeListHead) );
	
	pObj->Clear();

	pstPoolNode->m_stPoolNodeInfo.m_iCurUsedNum--;
	if( pstPoolNode->m_stPoolNodeInfo.m_iCurUsedNum < 0 )
	{
		assert( false );
		pstPoolNode->m_stPoolNodeInfo.m_iCurUsedNum = 0;
	}

	return;
}


template <class T>
IObject* ObjectPool<T>::Find( unsigned int dwObjID )
{
	UsedObjMap_t::iterator iter = m_oUsedObjMap.find( dwObjID );
	if( iter == m_oUsedObjMap.end() )
	{
		return NULL;
	}
	
	return iter->second;
}

template <class T>
void ObjectPool<T>::SetMaxObjNum( unsigned short wType, int iMaxNum )
{
	PoolNode* pstPoolNode = this->_GetPoolNode(wType);
	if( 0 == pstPoolNode->m_stPoolNodeInfo.m_iMaxObjNum )
	{
		pstPoolNode->m_stPoolNodeInfo.m_iMaxObjNum = iMaxNum;
	}
}

template <class T>
void ObjectPool<T>::DumpPoolNodeInfo( unsigned short wType, PoolNodeInfo& rPoolNodeInfo )
{
	PoolNodeMap_t::iterator it = m_oPoolNodeMap.find( wType );
	if( it == m_oPoolNodeMap.end() )
	{
		bzero( &rPoolNodeInfo, sizeof(rPoolNodeInfo) );
		return;
	}else
	{
		rPoolNodeInfo = it->second->m_stPoolNodeInfo;
	}
}

template <class T>
PoolNode* ObjectPool<T>::_GetPoolNode( unsigned short wType )
{
	PoolNodeMap_t::iterator it = m_oPoolNodeMap.find( wType );
	if( it != m_oPoolNodeMap.end() )
	{
		return it->second;
	}

	// new one
	PoolNode* pstPoolNode = new PoolNode();
	pstPoolNode->m_stPoolNodeInfo.m_wType = wType;
	m_oPoolNodeMap.insert( PoolNodeMap_t::value_type( wType, pstPoolNode ) );
	return pstPoolNode;
}

