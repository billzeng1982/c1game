#ifndef _STATIC_HASH_H_
#define _STATIC_HASH_H_

/*
 *	��̬hash����, ʹ��list_i.h���struct tlist_node������
 *	struct tlist_node����Ƕ��ĳ��Entry���, ʹ���߸���Entry�Ľ���
 */

#include "define.h"
#include "list_i.h"
#include "shmop.h"

enum EHASH_MAP_RET_TYPE
{
	EHASH_MAP_SUCC = 0,
	EHASH_MAP_FAIL = -1,
	EHASH_MAP_DUP  = -2,  // key duplicate
};

enum EHASH_MAP_KEY_TYPE
{
	HASH_MAP_KEY_NUMBER = 0,
	HASH_MAP_KEY_STRING = 1,
};

struct SHashMapCtl 
{
	key_t 	m_iShmKey;		// iShmKey=0��ʾ�ڶ��ϴ���
	int  	m_iSlots; 		// number of slots in hash table
	int  	m_iElems; 		// number of elements in hash table
	int  	m_iKeyLen;		// key Length in bytes
	int 	m_iKeyType;		// key��������
	int 	m_iNoRelease;	// 0: ����detach, 1: ������detatch
};


typedef bool (*HashCompareCb)(void*, void*);  // true - equal, false - not equal

#define HASH_MAP_I_SIZE( slots ) ( (slots)*sizeof(TLISTNODE) + sizeof(struct SHashMapCtl) ) 

/*
	+----------------------------+
    |      	  SHashMapCtl        |
    +----------------------------+
    |                            |
    |		  HashTable          |
    |                            |
    +----------------------------+
 */
class CHashMap_i
{
public:
	CHashMap_i()
	{
		m_pstHashMapCtl=NULL;
		m_pTable=NULL;
		m_fHashCompare=NULL;
		m_fHashGetKey=NULL;
	}
	
	~CHashMap_i() 
	{
		Detach();
	}

	// ����hash����, iShmKey=0��ʾ�ڶ��ϴ���
	bool HashCreate( key_t iShmKey, int iSlots, int iKeyLen, int iKeyType = HASH_MAP_KEY_NUMBER, int iNoRelease = 0 );

	bool HashInit( char* pszBuff, int iBuffSize, int iNew, key_t iShmKey, int iSlots, int iKeyLen, int iKeyType = HASH_MAP_KEY_NUMBER, int iNoRelease = 0);

	// ���ú���ָ��
	void SetCallback( HashCompareCb fHashCompare, void* (*fHashGetKey)(TLISTNODE*) );
	
	int  HashAdd( TLISTNODE *pHNode,  void* key );

	TLISTNODE* HashFind( void* key );

	int HashDelByKey( void* key );
	void HashDel( TLISTNODE* pHNode );

	void Detach();

	void Clear();

private:
	TLISTNODE* _HashFind(  TLISTNODE* pHead, void* key );
	// ѡ����һ��slot
	inline int _HashChoose( void* pHashKey );
	
private:
	char* 			m_pszBuff;
	SHashMapCtl* 	m_pstHashMapCtl;
	TLISTNODE*		m_pTable;   // tlist_node ����
		
	HashCompareCb	m_fHashCompare; // compare����ָ��
	void* (*m_fHashGetKey)(TLISTNODE*);  // ���key�ĺ���ָ��
};

#endif

