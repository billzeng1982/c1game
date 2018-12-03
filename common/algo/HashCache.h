#ifndef _HASH_CACHE_H_
#define _HASH_CACHE_H_

/*
	��lru��hash�����Ļ���

	1. ʹ��һ���������ڴ棬���˽��������Ϳ��ٲ����⣬����Ҳ�������ݽڵ�ķ��䡢���ա�swap in/out.
	2. ֧��ʱ������,�ɲ���LRU����(EHASH_CACHE_SWAP_ON)����������̭. LRU����ͨ��EHASH_CACHE_FLAGָ��
	3. ��������ע�������Ҫ���� operator =
	4. �ڹ����ڴ���������ò�Ҫ����ָ��
	5. time list�ɵ���used list������
*/

#include <assert.h>
#include <string.h>
#include <typeinfo>

#include "define.h"
#include "shmop.h"
#include "hash_func.h"
#include "list_i.h"
#include "container_of.h"

enum EHASH_CACHE_FLAG
{
	EHASH_CACHE_FLAG_NONE		= 0,
	EHASH_CACHE_SWAP_ON 		= 1 << 1, // ��cache��ʱ, ʹ�û���ʱ�����������swap in/out����
	EHASH_CACHE_LRU_WRITE_SENS	= 1 << 2, // ʱ������write����,������д�����,����Ϊ�������
	EHASH_CACHE_LRU_READ_SENS 	= 1 << 3, // ʱ������read����,��������,����Ϊ�������
};

enum EHASH_CACHE_ERRNO
{
	EHASH_CACHE_SUCC 			= 0,
	EHASH_CACHE_ERR				= -1, // ����
	EHASH_CACHE_ERR_NOT_ATT 	= -2, // û��attach
	EHASH_CACHE_ERR_NOT_FIND	= -3, // δ�ҵ�
	EHASH_CACHE_ERR_PARA		= -4, // ��������
	EHASH_CACHE_ERR_MEM			= -5, // �ڴ�������
	EHASH_CACHE_ERR_FULL		= -6, // ������
};


// Ĭ�ϵ�Key compare,����Key��==�������бȽ�, ��������������ȷ���true�����򷵻�false
template<typename TKey>
struct SDftEqualKey
{

    bool operator()( void* key1, void* key2 ) const
    {
		return ( *(TKey*)key1 == *(TKey*)key2 );
    }
};


// �ַ��� hash key compare
struct SEqualStr
{
	bool operator()( void* key1, void* key2) const
	{
		return ( 0 == strcmp( (char*)key1, (char*)key2 ) );
	}
};

template<typename TKey>
struct SDftHashFcn
{
	uint32_t operator()( void* key ) const
	{
		return zend_inline_hash_func( key, sizeof(TKey));
	}
};

struct SHashFcnStr
{
	uint32_t operator()( void* key ) const
	{
		return zend_inline_hash_func( key, strlen( (char*)key ) );
	}
};


/*
	hash cache ������ݽṹ����
*/

struct HCBucket
{
	int		  iCount;
	TLISTNODE stHashListHead;
};


struct HCDataHead
{
	TLISTNODE stHashListNode;
	TLISTNODE stTimeListNode;  // ����ʱ������, ע��ֻ��used��Data�Ź��ڸ�������. ���˻�Ϊһ��used list

	char	  cHashed;	// ��ʾ�����Ƿ������hash����

	char szKey[0];	// ����key��ַ
};


struct HCHead
{
	/*	
		run time non volatile member
	*/
	key_t	 iShmKey;		// ����0���ڶ��Ϸ���
	uint32_t uiFlag;		

	int		 iBuffSize;		// total buff size	
	int		 iKeySize;		// key�Ĵ�С
	
	int		 iBucket;		// bucket number used.
	int		 iMax;			// maximum items can store. 

	// the offset of bucket
	intptr_t iBucketOff;
	int 	 iBucketSize;	// bucket�������size(bytes)

	// the offset of data head.
	intptr_t iDataHeadOff;
	int		 iDataHeadSize;	// data head�������size(bytes)

	// the offset of data
	intptr_t iDataOff;
	int		 iDataSize;		// data����size(bytes)
	int		 iDataUnit;		// ÿһ�����ݵ�Ԫ�Ĵ�С

	/*
		run time volatile member
	*/
	int		 iElems;			// current cached data number.
	TLISTNODE stFreeListHead; 	// ��������ͷ
	TLISTNODE stTimeListHead; 	// ʱ������ͷ
};


/*
	useful macro definitions
 */
#define HC_GET_BUCKET(pstHCHead, i )		( (HCBucket*) ( ((size_t)(pstHCHead)) + (pstHCHead)->iBucketOff + (i) * sizeof(HCBucket) ) )

#define HC_GET_DATA_HEAD( pstHCHead, i )	( (HCDataHead*) ( ((size_t)(pstHCHead)) + (pstHCHead)->iDataHeadOff + (i) * (sizeof(HCDataHead) + (pstHCHead)->iKeySize) ) )

#define HC_GET_DATA( pstHCHead,i )	( (void *)( (size_t)(pstHCHead) + (pstHCHead)->iDataOff + (i) * (pstHCHead)->iDataUnit) )	

#define HC_DATA_HEAD_POS( pstHCHead, pstHCDataHead )  ( ( (size_t)(pstHCDataHead) - ( (pstHCHead)->iDataHeadOff + (size_t)(pstHCHead) ) ) / (sizeof(HCDataHead) + (pstHCHead)->iKeySize) )

#define HC_DATA_POS( pstHCHead, pvHCData ) ( ( (size_t)(pvHCData) - ( (pstHCHead)->iDataOff + (size_t)(pstHCHead) ) ) / (pstHCHead)->iDataUnit )

#define HC_DATAHEAD_2_DATA( pstHCHead, pstHCDataHead ) HC_GET_DATA( pstHCHead, HC_DATA_HEAD_POS( pstHCHead, pstHCDataHead ) )

//#define HC_DATA_2_DATAHEAD( pstHCHead, pvHCData ) HC_GET_DATA_HEAD( pstHCHead, HC_DATA_POS( pstHCHead, pvHCData ) )

/*
	HashCache��
	HashCache��copy KeySize���ֽڵ�HCDataHead.szKey�ĵ�ַ�ϣ�ͨ��������ȷ���㷨�ϵļ���ԣ�һ����
*/
template < typename TKey, typename TData, typename THashFcn=SDftHashFcn<TKey>, typename TEqualKey=SDftEqualKey<TKey> >
class CHashCache
{
public:
	CHashCache()
	{
		m_pszBuff = NULL;
		m_pstHCHead = NULL;
		bzero( m_szErrMsg, sizeof(m_szErrMsg) );
		m_pstTimeListIter = NULL;
	}
	
	~CHashCache()
	{
		this->Detach();
	}

	char* GetErrMsg() { return m_szErrMsg; }

	/*
		����:
		�ڶ��ϴ���cache

		����:
		iBucket: hashͰ�ĸ���
		iMax:	 �������cache����
		uiFlag:  �� enum EHASH_CACHE_FLAG
		iCacheType:  App�����Cache����ֵ

		����ֵ: 
		0 	: �ɹ�
		<0	: ����, GetErrMsg()
	*/
	int	CreateCache( int iBucket, int iMax, uint32_t uiFlag = 0 );

	/* ��shm�ϴ���hash cache
		����:
		1 - new create shm
		0 - attatch existed shm
		<0 - error
	*/
	int CreateCache_shm( key_t iShmKey, int iBucket, int iMax,  uint32_t uiFlag = 0 );

	/*
		����:
		pszBuff: ָ������cache���׵�ַ, ����Ϊ��
		iBuffSize: ����cacheռ���ڴ��С
		iNew: �����shm�Ϸ����, iNew = 0��ʾattach����shm, iNew=1��ʾ�½�shm 
		
		����ֵ:
		0 	: �ɹ�
		<0	: ����, GetErrMsg()
	*/
	int Attach( char* pszBuff, int iBuffSize, int iBucket, int iMax, uint32_t uiFlag = 0 );

	/*
		��ջ���
		����ֵ:
		0	:	�ɹ�
		<0	:	����
	*/	
	int	CleanCache( );

	/*
		����:
		������cache�кʹ���ؼ���ƥ������ݿ�copyһ��, ����

		����ֵ:
		0	:	�ɹ�
		<0	: ����, GetErrMsg()
	*/
	int GetDataCopy( void* pvHashKey, INOUT TData& tData );

	/*
		����:
		������cache�кʹ���ؼ���ƥ������ݿ�ָ�뷵��

		����:
		iErrno:	���ڷ��ش����
		<0	: ����, GetErrMsg()
		
		����ֵ:
		����ƥ������ָ��
	*/
	TData* FindData( void* pvHashKey );
	
	/*
		����:
		��Key->Value��ϵ��¼��cache��,����Ѵ��ڣ���update����

		����ֵ:
		����������hash cache�е�ָ��
	*/
	TData* AddData( void* pvHashKey, const TData& tData );

	/*
		����:
		������cache�кʹ���ؼ���ƥ����������

		����ֵ:
		0	: �ɹ�(����������cache���޴����ݵ����)
		<0	: ����, GetErrMsg()
	*/	
	int	DelDataByKey( void* pvHashKey );

	/*
		����ֵ:
		��ǰ���ݽڵ���
	*/
	int GetDataNum()
	{
		assert(m_pstHCHead);
		return m_pstHCHead->iElems;
	}

	/*
		����ֵ:
		������ݽڵ���
	*/
	int GetMaxDataNum()
	{
		assert(m_pstHCHead);
		return m_pstHCHead->iMax;
	}

	void Detach();

///////////////////////////////////

	/*
		����cache�п��е����ݽڵ�,�ɹ�����������Զ�����used list
	*/
	TData* NewData( );

	/*
		�� new �������������hash����
		<0: ʧ��
	*/
	int AddHash( void* pvHashKey, TData* pData );

	// ����NewData���������ݽڵ�
	int DelData( TData* pData );

	/*
		time list ����
	*/
	void IterBegin( )
	{
		m_pstTimeListIter = TLIST_NEXT( &( m_pstHCHead->stTimeListHead ) );
	}

	bool IsIterEnd( )
	{
		return ( (NULL == m_pstTimeListIter) || ( m_pstTimeListIter == &(m_pstHCHead->stTimeListHead) ) );
	}

	void IterNext( )
	{
		m_pstTimeListIter = TLIST_NEXT( m_pstTimeListIter );
	}

	TData* IterCurr( );

private:
	/*
		private methods
	*/

	void _ResetBuckets()
	{
		if( NULL == m_pstHCHead )
		{
			assert( false );
			return;
		}

		HCBucket* pstBucket = NULL;
		for( int i = 0; i < m_pstHCHead->iBucket; i++ )
		{
			pstBucket = HC_GET_BUCKET( m_pstHCHead, i );
			pstBucket->iCount = 0;
			TLIST_INIT( &(pstBucket->stHashListHead) );
		}
	}

	void _ResetDataHeads( )
	{
		if( NULL == m_pstHCHead )
		{
			assert( false );
			return;
		}

		TLIST_INIT( &(m_pstHCHead->stFreeListHead) );

		HCDataHead* pstDataHead = NULL;
		for( int i = 0; i < m_pstHCHead->iMax; i++ )
		{
			pstDataHead = HC_GET_DATA_HEAD( m_pstHCHead, i );
			
			// ����free list
			TLIST_INIT( &(pstDataHead->stTimeListNode ) );
			TLIST_INSERT_NEXT( &(m_pstHCHead->stFreeListHead), &(pstDataHead->stTimeListNode) );

			// init hash list node
			TLIST_INIT( &(pstDataHead->stHashListNode) );

			pstDataHead->cHashed = 0;
			bzero( pstDataHead->szKey, m_pstHCHead->iKeySize );
		}
	}


	int _HashChoose( void* pvHashKey )
	{
		assert( pvHashKey );
		SHashFcnStr tHasnFcn;
		return (int)( tHasnFcn(pvHashKey) % m_pstHCHead->iBucket );
	}

	HCDataHead* _GetNewNode( );

	HCDataHead* _FindHashNode( void* pvHashKey, HCBucket* pstBucket = NULL );

	void _DelDataNode( HCDataHead* pstDataHead, HCBucket* pstBucket = NULL );

	void _SwapOutHashNode();

private:
	char	*m_pszBuff; 	// ָ������cache���׵�ַ
	HCHead	*m_pstHCHead;

	char	m_szErrMsg[256];

	TLISTNODE* m_pstTimeListIter;
};


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
int	CHashCache<TKey, TData,  THashFcn, TEqualKey >::
	CreateCache( int iBucket, int iMax, uint32_t uiFlag )
{
	if( iBucket <=0  || iMax <=0 )
	{
		assert(false);
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Error input parameters!" );
		return EHASH_CACHE_ERR_PARA;
	}

	int iKeySize = sizeof(TKey);
	
	/* ���㲢�����ڴ� */
	int iBuffSize = sizeof( HCHead ) +							// hash cache head
					sizeof(HCBucket) * iBucket  + 				// bucket
					( sizeof(HCDataHead) + iKeySize ) * iMax +	// data head
					sizeof(TData) * iMax;						// data
	char* pszBuff = NULL;
	
	pszBuff = new char[ iBuffSize ];
	if( NULL == pszBuff )
	{
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "new mem failed! BuffSize[%d]", iBuffSize );
		return EHASH_CACHE_ERR_MEM;
	}
	
	return Attach( pszBuff, iBuffSize, iBucket, iMax, uiFlag );
}


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
int	CHashCache<TKey, TData,  THashFcn, TEqualKey >::
	CreateCache_shm( key_t iShmKey, int iBucket, int iMax,  uint32_t uiFlag )
{
	if( iBucket <=0  || iMax <=0 )
	{
		assert(false);
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Error input parameters!" );
		return EHASH_CACHE_ERR_PARA;
	}

	int iKeySize = sizeof(TKey);
	
	/* ���㲢�����ڴ� */
	int iBuffSize = sizeof( HCHead ) +							// hash cache head
					sizeof(HCBucket) * iBucket  + 				// bucket
					( sizeof(HCDataHead) + iKeySize ) * iMax +	// data head
					sizeof(TData) * iMax;						// data
	char* pszBuff = NULL;
	
	int iNew = GetShm( (void**)&pszBuff, iShmKey, iBuffSize, (0666 | IPC_CREAT) );
	if( iNew < 0 )
	{
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Get shm failed! ShmKey[%d], BuffSize[%d]", iShmKey, iBuffSize );
		return EHASH_CACHE_ERR_MEM;
	}

	if( 0 == iNew )	
	{
		// existed shm
		m_pszBuff = pszBuff;
		m_pstHCHead = (HCHead*)pszBuff;
		
		if( m_pstHCHead->iBuffSize	!= iBuffSize  || 
			m_pstHCHead->iShmKey    != iShmKey    ||
			m_pstHCHead->iMax		!= iMax )
		{
			assert(false);
			return EHASH_CACHE_ERR_PARA;
		}else
		{
			return iNew;	
		}
	}else
	{
		// new shm
		if( this->Attach( pszBuff, iBuffSize, iBucket, iMax, uiFlag ) < 0 )
		{
			return -3;
		}
		
		m_pstHCHead->iShmKey = iShmKey;
		return iNew;
	}
}


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
int	CHashCache<TKey, TData,  THashFcn, TEqualKey >::
	Attach( char* pszBuff, int iBuffSize, int iBucket, int iMax, uint32_t uiFlag )
{
	if( NULL == pszBuff || iBuffSize <= 0  || iBucket <=0 	|| iMax	<=0	)
	{
		assert( false );
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Error input parameters!" );
		return EHASH_CACHE_ERR_PARA;
	}

	m_pszBuff = pszBuff;
	m_pstHCHead = (HCHead*)pszBuff;

	// ���ֳ�ʼ������
	//bzero( m_pszBuff, iBuffSize );
	m_pstHCHead->uiFlag 		=  uiFlag;
	m_pstHCHead->iBuffSize 		= iBuffSize;
	m_pstHCHead->iKeySize 		= sizeof(TKey);
	m_pstHCHead->iBucket 		= iBucket;
	m_pstHCHead->iMax 			= iMax;

	m_pstHCHead->iBucketOff		= sizeof(HCHead);
	m_pstHCHead->iBucketSize	= sizeof(HCBucket)*iBucket;

	m_pstHCHead->iDataHeadOff	= m_pstHCHead->iBucketOff + m_pstHCHead->iBucketSize;
	m_pstHCHead->iDataHeadSize	= ( sizeof(HCDataHead) + m_pstHCHead->iKeySize )* iMax;

	m_pstHCHead->iDataOff 		= m_pstHCHead->iDataHeadOff + m_pstHCHead->iDataHeadSize;
	m_pstHCHead->iDataUnit 		= sizeof( TData );
	m_pstHCHead->iDataSize		= m_pstHCHead->iDataUnit * iMax;

	if( m_pstHCHead->iBuffSize != (int)( sizeof(HCHead) + m_pstHCHead->iBucketSize + m_pstHCHead->iDataHeadSize + m_pstHCHead->iDataSize ) )
	{
		assert(false);
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Buffsize error!" );
		this->Detach();
		return EHASH_CACHE_ERR_PARA;
	}

	return this->CleanCache( );
}


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
int	CHashCache<TKey, TData,  THashFcn, TEqualKey >::CleanCache(  )
{
	if( NULL == m_pstHCHead )
	{
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Not attached!" );
		assert(false);
		return EHASH_CACHE_ERR_NOT_ATT;
	}

	_ResetBuckets();
	_ResetDataHeads();

	m_pstHCHead->iElems = 0;
	TLIST_INIT( &(m_pstHCHead->stTimeListHead) );

	return EHASH_CACHE_SUCC;
}


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
int CHashCache<TKey, TData,  THashFcn, TEqualKey >::
	GetDataCopy( void* pvHashKey, INOUT TData& tData )
{
	if( NULL == pvHashKey )
	{
		assert(false);
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "pvHashKey is NULL!" );
		return EHASH_CACHE_ERR_PARA;
	}

	TData* pDataFound = this->FindData( pvHashKey );
	if( NULL == pDataFound )
	{
		return EHASH_CACHE_ERR_NOT_FIND;
	}

	tData = *pDataFound; // �������͵�data��Ҫ���� operator =

	return EHASH_CACHE_SUCC;
}


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
TData* CHashCache<TKey, TData,  THashFcn, TEqualKey >::
	FindData( void* pvHashKey )
{		
	if( NULL == m_pstHCHead )
	{
		assert(false);
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Not attached!" );
		return NULL;
	}

	if( NULL == pvHashKey )
	{
		assert(false);
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "pvHashKey is NULL!" );
		return NULL;
	}

	TData* pDataFound = NULL;
		
	HCDataHead* pstDataHead = _FindHashNode( pvHashKey );
	if( NULL == pstDataHead )
	{
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Not found!" );
	}else
	{
		pDataFound = (TData*)HC_DATAHEAD_2_DATA( m_pstHCHead, pstDataHead );
		if( m_pstHCHead->uiFlag & EHASH_CACHE_LRU_READ_SENS )
		{
			TLIST_MOVE_TAIL( &(m_pstHCHead->stTimeListHead), &(pstDataHead->stTimeListNode) );
		}
	}

	return pDataFound;
}


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
TData* CHashCache<TKey, TData,  THashFcn, TEqualKey >::
	AddData( void* pvHashKey, const TData& tData )
{
	if( NULL == m_pstHCHead )
	{
		assert(false);
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Not attached!" );		
		return NULL;
	}

	if( NULL == pvHashKey )
	{
		assert(false);
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "pvHashKey is NULL!" );
		return NULL;
	}

	TData* pNewData = NULL;

	int iSlot = _HashChoose( pvHashKey );
	assert( iSlot >= 0 && iSlot < m_pstHCHead->iBucket );
	HCBucket* pstBucket = HC_GET_BUCKET( m_pstHCHead, iSlot );

	HCDataHead* pstDataHead = _FindHashNode( pvHashKey, pstBucket );
	if( pstDataHead )
	{
		// �Ѿ����ڣ���update
		TData* pDataFound = (TData*)HC_DATAHEAD_2_DATA( m_pstHCHead, pstDataHead );
		*pDataFound = tData;

		if( m_pstHCHead->uiFlag & EHASH_CACHE_LRU_WRITE_SENS )
		{
			// move ��ʱ������β��
			TLIST_MOVE_TAIL( &(m_pstHCHead->stTimeListHead), &(pstDataHead->stTimeListNode) );
		}
	}else
	{
		HCDataHead* pstNewDataHead = this->_GetNewNode( );
		if( NULL == pstNewDataHead )
		{
			return NULL;
		}
		
		/* add new data */				
		pNewData = (TData*)HC_DATAHEAD_2_DATA( m_pstHCHead, pstNewDataHead );
		*pNewData = tData;  // ����Ƿ�pod class���ͣ���Ҫʵ�� operator =

		/* add hash */
		memcpy( pstNewDataHead->szKey, pvHashKey, m_pstHCHead->iKeySize );
		TLIST_INSERT_NEXT( &(pstBucket->stHashListHead), &(pstNewDataHead->stHashListNode) );
		pstNewDataHead->cHashed = 1;
		++pstBucket->iCount;	
	}

	return pNewData;
}


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
int	CHashCache<TKey, TData,  THashFcn, TEqualKey >::
	DelDataByKey( void* pvHashKey )
{
	if( NULL == m_pstHCHead )
	{
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Not attached!" );
		assert(false);
		return EHASH_CACHE_ERR_NOT_ATT;
	}

	if( NULL == pvHashKey )
	{
		assert(false);
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "pvHashKey is NULL!" );
		return EHASH_CACHE_ERR_PARA;
	}

	int iSlot = _HashChoose( pvHashKey );
	assert( iSlot >= 0 && iSlot < m_pstHCHead->iBucket );
	HCBucket* pstBucket = HC_GET_BUCKET( m_pstHCHead, iSlot );
	HCDataHead* pstDataHead = _FindHashNode( pvHashKey, pstBucket );
	if( pstDataHead )
	{
		this->_DelDataNode( pstDataHead, pstBucket );
	}
	
	return EHASH_CACHE_SUCC;
}


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
HCDataHead* CHashCache<TKey, TData,  THashFcn, TEqualKey >::_GetNewNode()
{
	assert( m_pstHCHead );

	if( m_pstHCHead->iElems >= m_pstHCHead->iMax )
	{
		if( ( m_pstHCHead->uiFlag & EHASH_CACHE_SWAP_ON ) )
		{
			this->_SwapOutHashNode();
			if( m_pstHCHead->iElems >= m_pstHCHead->iMax )
			{
				assert( false );
				snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Cache full!" );
				return NULL;
			}
		}else
		{
			snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Cache full!" );
			return NULL;
		}
	}

	TLISTNODE* pstFreeNode = TLIST_NEXT( &(m_pstHCHead->stFreeListHead) );
	if( pstFreeNode == &(m_pstHCHead->stFreeListHead) )
	{
		assert( false );
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Cache full!" );
		return NULL;
	}

	// del from free list
	TLIST_DEL(pstFreeNode);

	HCDataHead* pstFreeDataHead = NULL;
	pstFreeDataHead = container_of( pstFreeNode, HCDataHead, stTimeListNode );

	++m_pstHCHead->iElems;
	assert( m_pstHCHead->iElems <= m_pstHCHead->iMax );

	// �嵽ʱ������β��
	TLIST_INSERT_PREV( &(m_pstHCHead->stTimeListHead), &(pstFreeDataHead->stTimeListNode) );
	
	return pstFreeDataHead;
}


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
HCDataHead*  CHashCache<TKey, TData,  THashFcn, TEqualKey >::
	_FindHashNode( void* pvHashKey, HCBucket* pstBucket )
{
	assert( m_pstHCHead && pvHashKey );

	if( NULL == pstBucket )
	{
		int iSlot = _HashChoose( pvHashKey );
		assert( iSlot >= 0 && iSlot < m_pstHCHead->iBucket );
		pstBucket = HC_GET_BUCKET( m_pstHCHead, iSlot );
	}

	TLISTNODE* pstHashListHead= &(pstBucket->stHashListHead);

	TEqualKey   tEqualKeyFunc;

	TLISTNODE* pstPos = NULL;
    TLISTNODE* pstNext = NULL;
	HCDataHead* pstDataHead = NULL;
	HCDataHead* pstDataHeadFound = NULL;

    TLIST_FOR_EACH_SAFE( pstPos, pstNext, pstHashListHead )
	{
		pstDataHead = container_of( pstPos, HCDataHead, stHashListNode );
		if( tEqualKeyFunc( pvHashKey, pstDataHead->szKey ) )
		{		
			pstDataHeadFound = pstDataHead;
			break;
		}
	}

	return pstDataHeadFound;
}


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
void CHashCache<TKey, TData,  THashFcn, TEqualKey >::
	_DelDataNode( HCDataHead* pstDataHead, HCBucket* pstBucket  )
{
	if( NULL == pstDataHead)
	{
		assert( false );
		return;
	}

	--m_pstHCHead->iElems;
	assert( m_pstHCHead->iElems >= 0 );

	if( ( pstDataHead->cHashed ) )
	{
		if( NULL == pstBucket )
		{
			int iSlot = _HashChoose( pstDataHead->szKey );
			assert( iSlot >= 0 && iSlot < m_pstHCHead->iBucket );
		 	pstBucket = HC_GET_BUCKET( m_pstHCHead, iSlot );
		}
	
		// ��hash����������
		--pstBucket->iCount;
		TLIST_DEL( &(pstDataHead->stHashListNode) );
		assert( pstBucket->iCount >= 0 );
		pstDataHead->cHashed = 0;
		// reset key
		bzero( pstDataHead->szKey, m_pstHCHead->iKeySize );
	}

	if( m_pstTimeListIter == &(pstDataHead->stTimeListNode) )
	{
		m_pstTimeListIter = TLIST_NEXT( &(pstDataHead->stTimeListNode) );
	}
	// ��ʱ������������
	TLIST_DEL( &(pstDataHead->stTimeListNode) );
	// add to free list
	TLIST_INSERT_NEXT( &(m_pstHCHead->stFreeListHead), &(pstDataHead->stTimeListNode) );

	// data reset��Ӧ�ó������
}


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
void CHashCache<TKey, TData,  THashFcn, TEqualKey >::
	_SwapOutHashNode( )
{
	if( TLIST_IS_EMPTY( &(m_pstHCHead->stTimeListHead) ) ||
		m_pstHCHead->iElems < m_pstHCHead->iMax )
	{
		// ����Ҫswap
		assert( false );
		return;
	}
	
	TLISTNODE* pstSwapNode = TLIST_NEXT( &(m_pstHCHead->stTimeListHead) );
	HCDataHead* pstDataHead = container_of( pstSwapNode, HCDataHead, stTimeListNode );

	this->_DelDataNode( pstDataHead );
}


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
TData* CHashCache<TKey, TData,  THashFcn, TEqualKey >::NewData(  )
{
	assert( m_pstHCHead );

	HCDataHead* pstNewDataHead = this->_GetNewNode();
	if( !pstNewDataHead )
	{
		return NULL;
	}
	
	return (TData*) HC_DATAHEAD_2_DATA( m_pstHCHead, pstNewDataHead );
}


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
int CHashCache<TKey, TData,  THashFcn, TEqualKey >::AddHash( void* pvHashKey, TData* pData )
{
	if( NULL == pvHashKey || NULL == pData )	
	{
		assert( false );
		return EHASH_CACHE_ERR;
	}

	assert( m_pstHCHead );
		
	int iPos = HC_DATA_POS( m_pstHCHead, pData );
	if( iPos < 0 || iPos > ( m_pstHCHead->iMax-1 ) )
	{
		assert( false );
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Data pos [%d] invalid!", iPos );
		return EHASH_CACHE_ERR;
	}

	HCDataHead* pstDataHead = HC_GET_DATA_HEAD( m_pstHCHead, iPos );

	if( pstDataHead->cHashed )
	{
		assert( false );
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Data Already hashed!" );
		return EHASH_CACHE_ERR;
	}

	int iSlot = _HashChoose( pvHashKey );
	assert( iSlot >= 0 && iSlot < m_pstHCHead->iBucket );
	HCBucket* pstBucket = HC_GET_BUCKET( m_pstHCHead, iSlot );

	HCDataHead* pstDataHeadTmp = _FindHashNode( pvHashKey, pstBucket );
	if( pstDataHeadTmp )
	{
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Key duplicate!" );
		return EHASH_CACHE_ERR;
	}

	/* add hash */
	memcpy( pstDataHead->szKey, pvHashKey, m_pstHCHead->iKeySize );
	pstDataHead->cHashed = 1;		
	TLIST_INSERT_NEXT( &(pstBucket->stHashListHead), &(pstDataHead->stHashListNode) );
	++pstBucket->iCount;

	return EHASH_CACHE_SUCC;
}


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
int CHashCache<TKey, TData,  THashFcn, TEqualKey >::DelData( TData* pData )
{
	if( NULL == pData )
	{
		return EHASH_CACHE_SUCC;
	}

	assert( m_pstHCHead );

	int iPos = HC_DATA_POS( m_pstHCHead, pData );
	if ( iPos < 0 || iPos > ( m_pstHCHead->iMax-1 ) )
	{
		assert( false );
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Data pos [%d] invalid!", iPos );
		return EHASH_CACHE_ERR;
	}
	HCDataHead* pstDataHead = HC_GET_DATA_HEAD( m_pstHCHead, iPos );

	this->_DelDataNode( pstDataHead );
	
	return EHASH_CACHE_SUCC;
}


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
void CHashCache<TKey, TData,  THashFcn, TEqualKey >::Detach()
{
	if( NULL == m_pstHCHead )
	{
		assert( false );
		return;
	}

	if( m_pstHCHead->iShmKey )
	{
		shmdt( (void*)m_pszBuff );
	}else
	{
		delete[] m_pszBuff;
	}

	m_pszBuff = NULL;
	m_pstHCHead = NULL;
}


template < typename TKey, typename TData,  typename THashFcn, typename TEqualKey >
TData* CHashCache<TKey, TData,  THashFcn, TEqualKey >::IterCurr()
{
	if( IsIterEnd() )
	{
		return NULL;
	}

	HCDataHead* pstDataHead = container_of( m_pstTimeListIter, HCDataHead, stTimeListNode );

	return (TData*) HC_DATAHEAD_2_DATA( m_pstHCHead, pstDataHead );
}

#endif

