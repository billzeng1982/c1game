/**
 * ����hash�����Ϳ��ٲ��ң���������hash���ݽڵ�
 * ��Ƕ���ݵ��ͷ����ϲ��߼�����
 */
#ifndef _HTABLE_H_2011_02_02
#define _HTABLE_H_2011_02_02

#include <assert.h>
#include <string.h>
#include "list.h"
#include "define.h"
#include "HashMap_i.h"
#include "hash_func.h"

class CHashMap 
{
public:
	CHashMap();
	virtual ~CHashMap();
	
	UINT GetHashSize() { return m_stHashMapCtl.m_iSlots; }
	UINT GetHashElems() { return m_stHashMapCtl.m_iElems; }

	/* allocates and clears the hash */
	bool HashCreate( int iSlots, int iKeyLen, int iKeyType = HASH_MAP_KEY_NUMBER );

	/* set callbacks */
	void SetCallback( HashCompareCb fHashCompare, void* (*fHashGetKey)(struct hlist_node*) );

	/* clears the hash */
	void HashInit();

	/* adds hash node to the hashtable. */
	int HashAdd( struct hlist_node* pHNode, void* key );

	/* find hash node int the hashtable */
	struct hlist_node* HashFind( void* key );

	/* delete hlist_node from hashtable */
	int HashDelByKey( void* key );

	// �Ƚϱ��� 
	void HashDel( struct hlist_node* pHNode );
	
private:
	struct hlist_node* _HashFind( struct hlist_head* pHead, void* key );
	inline int _HashChoose( void* pHashKey );
	
private:
	SHashMapCtl m_stHashMapCtl;
	struct hlist_head* m_pTable; // hlist_head ����

	HashCompareCb	m_fHashCompare; // compare����ָ��
	void* (*m_fHashGetKey)(struct hlist_node*);  // ���key�ĺ���ָ��
};


inline int CHashMap::_HashChoose( void* pHashKey )
{
    assert( pHashKey );
	if( HASH_MAP_KEY_STRING == m_stHashMapCtl.m_iKeyType )
    {
    	return (int)zend_inline_hash_func( pHashKey, strlen((char*)pHashKey), m_stHashMapCtl.m_iSlots );	
	}else
	{
		return (int)zend_inline_hash_func( pHashKey, m_stHashMapCtl.m_iKeyLen, m_stHashMapCtl.m_iSlots );
	}
}

#endif

