#ifndef _MEMPOOL_H_
#define _MEMPOOL_H_

/*
	˵��:
	1. �̶��ڵ��������ڴ��, ����/���չ̶��ڴ��С��node
	2. HANDLE��ʵ�������ݽڵ���������� pos+1, ͨ����HANDLE����ȡ�����ݽڵ��ָ��
	3. �����ڴ��ϵ�mempool, ���ݲ�Ҫ����ָ��
	4. ��������ע�������Ҫ���� operator =
	5. ע��,�����Ƕ�used list���е���!
	6. ��0�����ɵ����߸���
	7. �����ڴ�,attach usedʱ,���ⲿ�����߾����Ƿ�construct all

	8. Iterator���������, ��֧�ֶ��������, billzeng, 2016/04/19
 */

#include <assert.h>
#include <string.h>
#include <new>
#include "define.h"
#include "shmop.h"
#include "list_i.h"
#include "container_of.h"
#include "DynArray.h"

struct SMemBlock
{
	TLISTNODE stNode; // ����free list��used list��
	char	  cUsed;
};

struct SMemPoolHead
{
	/*
		run time non volatile member
	*/
	key_t	iShmKey;	// ����0���ڶ��Ϸ���
	uint32_t uiFlag;		
		
	int 	iMax;		//	mempool������
	int		iBuffSize;	// total buff size	

	intptr_t iBlockOff;
	int		 iBlockSize;

	intptr_t iDataOff;
	int		 iDataSize;		// data����size(bytes)
	int 	 iDataUnit; 	// ÿһ�����ݵ�Ԫ�Ĵ�С

	/*
		run time volatile member
	*/
	int 	  iUsedNum;
	TLISTNODE stFreeListHead;
	TLISTNODE stUsedListHead;	
};

/*
	useful macro definitions
*/

#define MEMPOOL_HEAD_SIZE sizeof(SMemPoolHead)

#define MEMPOOL_BLOCK_SIZE(iMax) ( (iMax) * sizeof(SMemBlock))

#define MEMPOOL_DATA_SIZE(iMax,iUnit) ( (iMax) * (iUnit) )

// mempool�ܴ�С
#define MEMPOOL_SIZE(iMax, iUnit ) ( MEMPOOL_HEAD_SIZE + MEMPOOL_BLOCK_SIZE(iMax) + MEMPOOL_DATA_SIZE(iMax, iUnit) )

#define MEMPOOL_GET_BLOCK( pstPoolHead, iPos ) ( (SMemBlock*)( (size_t)(pstPoolHead) + (pstPoolHead)->iBlockOff + sizeof(SMemBlock) * ((int)(iPos) % (pstPoolHead)->iMax) ) )

#define MEMPOOL_GET_DATA( pstPoolHead, iPos )  ( (void *)( (size_t)(pstPoolHead) + (pstPoolHead)->iDataOff + (pstPoolHead)->iDataUnit * ((int)(iPos) % (pstPoolHead)->iMax) ) )	

#define MEMPOOL_BLOCK_POS( pstPoolHead, pstBlock ) ( ( (size_t)(pstBlock) - ( (pstPoolHead)->iBlockOff + (size_t)(pstPoolHead) ) ) / sizeof(SMemBlock) )

#define MEMPOOL_DATA_POS( pstPoolHead, pvData ) ( ( (size_t)(pvData) - ( (pstPoolHead)->iDataOff + (size_t)(pstPoolHead) ) ) / (pstPoolHead)->iDataUnit )

#define MEMPOOL_BLOCK_2_DATA( pstPoolHead, pstBlock ) MEMPOOL_GET_DATA( pstPoolHead, MEMPOOL_BLOCK_POS(pstPoolHead, pstBlock) )

//#define MEMPOOL_DATA_2_BLOCK( pstPoolHead, pvData ) MEMPOOL_GET_BLOCK( pstPoolHead, MEMPOOL_DATA_POS(pstPoolHead, pvData) )


template< typename TData >
class CMemPool
{
public:

	// ����usedԪ��
	class UsedIterator
	{
	public:
		UsedIterator( )
		{
			m_poMemPool = NULL;
			m_pstUsedIter = NULL;
		}

		void SetMempool( CMemPool* poMemPool )
		{
			m_poMemPool = poMemPool;
		}

		void Begin()
		{
			assert(m_poMemPool);
			m_pstUsedIter = TLIST_NEXT( &(m_poMemPool->m_pstPoolHead->stUsedListHead) );
		}
		
		void Next()
		{
			assert(m_poMemPool);
			m_pstUsedIter = TLIST_NEXT( m_pstUsedIter );
		}
		
		bool IsEnd()
		{
			return ( (NULL == m_pstUsedIter) || ( m_pstUsedIter == &(m_poMemPool->m_pstPoolHead->stUsedListHead) ) );
		}
		
	 	TData* CurrItem()
 		{
 			if( this->IsEnd() )
			{
				return NULL;
			}

			SMemBlock* pstBlock = TLIST_ENTRY( m_pstUsedIter, SMemBlock, stNode );
			assert( pstBlock->cUsed );

			return (TData*)MEMPOOL_BLOCK_2_DATA(m_poMemPool->m_pstPoolHead, pstBlock);
 		}

		friend class CMemPool;

	private:
		void _Prev()
		{
			m_pstUsedIter = TLIST_PREV( m_pstUsedIter );
		}
		
	private:
		CMemPool* 	m_poMemPool;
		TLISTNODE*	m_pstUsedIter; // used list interator
	};

public:
	CMemPool() : m_oSlicedIterArr(10)
	{
		m_pszBuff = NULL;
		m_pstPoolHead = NULL;
		bzero( m_szErrMsg, sizeof(m_szErrMsg) );
		//m_pstUsedIter = NULL;
	}

	~CMemPool( )
	{
		this->DestroyPool();
		m_oSlicedIterArr.Clear();
	}

	void DestroyPool();
	
	/*
		����ֵ: 
		0	: �ɹ�
		<0	: ����, GetErrMsg()
	*/
	int	CreatePool( int iMax );

	/* ��shm�ϴ����ڴ��
		����:
		1 - new create shm
		0 - attatch existed shm
		<0 - error
	*/
	int CreatePool_shm( key_t iShmKey, int iMax );
		
	/*
		����:
		pszBuff: ָ������cache���׵�ַ, ����Ϊ��
		iBuffSize: ����cacheռ���ڴ��С
		iMax: ������ݽڵ�����
		
		
		����ֵ:
		0	: �ɹ�
		<0	: ����, GetErrMsg()
	*/
	int AttachNew( char* pszBuff, int iBuffSize, int iMax );

	/*
		��ȡ���е�data��Դ������ȡ�ɹ�,��Ӧ��data��Դ�Զ�����free list,����used list

		����ֵ: 
		������ָ�뼰handleֵ
		NULL : ����
	*/
	TData* 	NewData( int* piHnd = NULL );

	/*
		����used data��Դ

		����ֵ:
		0 	: �ɹ�
		<0	: ����
	*/
	int		DeleteData( HANDLE iHnd );
	int 	DeleteData( TData* pData );
	

	/*
		����:
		ͨ��Handle����������used����ָ��

		����ֵ:
		����Handle����������ָ��
		NULL: ����, GetErrMsg()
	*/
	TData* GetDataByHnd( HANDLE iHnd );

	/*
		����:
		ͨ�������±��������������ָ��,��һ����used������
		ע���GetDataByHnd()������

		����ֵ:
		����Handle����������ָ��
		NULL: ����, GetErrMsg()
	*/
	TData* GetDataByPos( int iPos );

	/*
		����:
		�����ڴ��
	*/
	void CleanPool();

	int GetUsedNum()
	{
		assert(m_pstPoolHead);
		return m_pstPoolHead->iUsedNum;
	}

	int GetMaxNum()
	{
		assert(m_pstPoolHead);
		return m_pstPoolHead->iMax;
	}

	int GetFreeNum()
	{
		assert(m_pstPoolHead);
		return ( m_pstPoolHead->iMax - m_pstPoolHead->iUsedNum );
	}

	char* GetErrMsg() 
	{
		return m_szErrMsg;
	}

	char GetUsedFlag( HANDLE iHnd );

	/*
		used list �������API
	*/

#if 0
	void IterBegin( )
	{
		m_pstUsedIter = TLIST_NEXT( &(m_pstPoolHead->stUsedListHead) );
	}

	bool IsIterEnd( )
	{
		return ( (NULL == m_pstUsedIter) || ( m_pstUsedIter == &(m_pstPoolHead->stUsedListHead) ) );
	}

	void IterNext( )
	{
		m_pstUsedIter = TLIST_NEXT( m_pstUsedIter );
	}
	TData* IterCurr( );
#endif

	// ע���Ƭ�����ĵ�����
	void RegisterSlicedIter( UsedIterator* poIter )
	{
		if( !poIter )
		{
			assert(false);
			return;
		}

		poIter->m_poMemPool = this;
		m_oSlicedIterArr.PushBack( poIter );
	}
	
	
	// ȫ������һ��replacement new,������һ�ι��캯��
	void ConstructAll();

	friend class UsedIterator;

private:
	/*
		private methods
	*/
	void _InitFreeList();
	
private:
	char		 *m_pszBuff;
	SMemPoolHead *m_pstPoolHead;

	char 		 m_szErrMsg[256];

	//TLISTNODE	 *m_pstUsedIter; // used list interator

	CDynArray<UsedIterator*> m_oSlicedIterArr; // ��Ƭ����iterator�б�
};


template< typename TData >
int CMemPool<TData>::CreatePool( int iMax )
{
	if( iMax <= 0 )
	{
		assert( false );
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Error input parameters! iMax[%d]", iMax );
		return -1;
	}

	/* �����ڴ��С*/
	int iBuffSize = (int)MEMPOOL_SIZE(iMax, (int)sizeof(TData));
	char* pszBuff = NULL;

	pszBuff = new char[iBuffSize];
	if( NULL == pszBuff )
	{
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "new mem failed! BuffSize[%d]", iBuffSize );
		return -3;
	}

	return AttachNew( pszBuff, iBuffSize, iMax );
}


/* ��shm�ϴ����ڴ��
	����:
	1 - new create shm
	0 - attatch existed shm
	<0 - error
*/
template< typename TData >
int CMemPool<TData>::CreatePool_shm( key_t iShmKey, int iMax )
{
	if( iMax <= 0 || 0 == iShmKey)
	{
		assert( false );
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Error input parameters! iMax[%d]", iMax );
		return -1;
	}

	/* �����ڴ��С*/
	int iBuffSize = (int)MEMPOOL_SIZE(iMax, (int)sizeof(TData));
	char* pszBuff = NULL;
	int iNew = 0;

	iNew = GetShm( (void**)&pszBuff, iShmKey, iBuffSize, (0666 | IPC_CREAT) );
	if( iNew < 0 )
	{
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Get shm failed! ShmKey[%d], BuffSize[%d]", iShmKey, iBuffSize );
		return -2;
	}

	if( 0 == iNew )
	{
		// attach existed shm
		m_pszBuff = pszBuff;
		m_pstPoolHead = (SMemPoolHead*)pszBuff;
		if(	m_pstPoolHead->iBuffSize != iBuffSize ||
			m_pstPoolHead->iShmKey 	 != iShmKey   ||
			m_pstPoolHead->iMax      != iMax )
		{
			assert(false);
			return -2;
		}
	}else
	{
		// new shm
		if( this->AttachNew( pszBuff, iBuffSize, iMax ) < 0 )
		{
			return -3;
		}
		
		m_pstPoolHead->iShmKey = iShmKey;
	}

	return iNew;
}


template< typename TData >
int CMemPool<TData>::AttachNew( char* pszBuff, int iBuffSize, int iMax )
{
	if( NULL == pszBuff || iBuffSize <= 0 || iMax <=0 )
	{
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Error input parameters!" );
		return -1;
	}

	m_pszBuff = pszBuff;
	m_pstPoolHead = (SMemPoolHead*)pszBuff;
	
	// ���ֳ�ʼ������
	bzero(m_pszBuff, iBuffSize);
	
	m_pstPoolHead->iMax			= iMax;
	m_pstPoolHead->iBuffSize	= iBuffSize;
	
	m_pstPoolHead->iBlockOff	= MEMPOOL_HEAD_SIZE;
	m_pstPoolHead->iBlockSize 	= MEMPOOL_BLOCK_SIZE(m_pstPoolHead->iMax);

	m_pstPoolHead->iDataOff 	= m_pstPoolHead->iBlockOff + m_pstPoolHead->iBlockSize;
	m_pstPoolHead->iDataUnit	= sizeof(TData);
	m_pstPoolHead->iDataSize	= MEMPOOL_DATA_SIZE( m_pstPoolHead->iMax, m_pstPoolHead->iDataUnit );

	if( iBuffSize != (int)MEMPOOL_SIZE( iMax, (int)sizeof(TData) ) )
	{
		assert(false);
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Buffsize error!" );
		return -2;
	}

	this->CleanPool();
	this->ConstructAll();

	return 0; 
}


template< typename TData >
TData* 	CMemPool<TData>::NewData( int* piHnd )
{
	if( NULL == m_pstPoolHead )
	{
		assert(false);
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Not attached!" );
		return NULL;
	}

	if( m_pstPoolHead->iUsedNum >= m_pstPoolHead->iMax )
	{
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Mempool full!" );
		return NULL;
	}

	TLISTNODE* pstFreeNode = TLIST_NEXT( &(m_pstPoolHead->stFreeListHead) );
	TLIST_DEL( pstFreeNode );
	//TLIST_INSERT_NEXT( &(m_pstPoolHead->stUsedListHead), pstFreeNode ); // stack, not use
	TLIST_INSERT_PREV( &(m_pstPoolHead->stUsedListHead), pstFreeNode );	 // queue	

	++m_pstPoolHead->iUsedNum;
	
	SMemBlock* psBlock = TLIST_ENTRY( pstFreeNode, SMemBlock, stNode );
	psBlock->cUsed = 1;

	int iPos = (HANDLE)MEMPOOL_BLOCK_POS( m_pstPoolHead, psBlock );
	assert( iPos >= 0 && iPos < m_pstPoolHead->iMax );
	HANDLE iHnd = (HANDLE)(iPos + 1); // ע��Handle�� pos+1 !

	if( piHnd )
	{
		*piHnd = iHnd;
	}

	TData *pData = (TData*)MEMPOOL_GET_DATA( m_pstPoolHead, iPos );

	return pData;
}


template< typename TData >
int	CMemPool<TData>::DeleteData( HANDLE iHnd )
{
	if( NULL == m_pstPoolHead )
	{
		assert(false);
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Not attached!" );
		return -1;
	}

	if( iHnd <= 0 || iHnd > m_pstPoolHead->iMax )
	{
		assert( false );
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Handle [%d] error!", iHnd );
		return  -2; 
	}

	int iPos = iHnd - 1;
	SMemBlock* pstBlock = MEMPOOL_GET_BLOCK(m_pstPoolHead, iPos);
	if( !pstBlock->cUsed )
	{
		assert( false );
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Data not used! Handle [%d]", iHnd );
		return -3;
	}

	pstBlock->cUsed = 0;

	//if( m_pstUsedIter == &(pstBlock->stNode) )
	//{
	//	m_pstUsedIter = TLIST_NEXT( &(pstBlock->stNode) );
	//}

	// ����sliced iterator, ע��, iterָ����һ��Ԫ��, ��ֹ����ѭ������Next����һ��Ԫ��!!
	for( int i=0; i < m_oSlicedIterArr.Length(); i++ )
	{
		if( m_oSlicedIterArr[i]->m_pstUsedIter == &(pstBlock->stNode) )
		{
			m_oSlicedIterArr[i]->_Prev();
		}
	}
	
	TLIST_MOVE( &(m_pstPoolHead->stFreeListHead), &(pstBlock->stNode) );
	--m_pstPoolHead->iUsedNum;
	if( m_pstPoolHead->iUsedNum < 0 )
	{
		assert( false );
		m_pstPoolHead->iUsedNum = 0;
	}

	return 0;
}


template< typename TData >
int CMemPool<TData>::DeleteData( TData* pData )
{
	HANDLE iHnd = (int)MEMPOOL_DATA_POS( m_pstPoolHead, pData ) + 1;
	return DeleteData( iHnd );
}


template< typename TData >
TData* CMemPool<TData>::GetDataByHnd( HANDLE iHnd )
{
	assert( m_pstPoolHead );

	if( iHnd <= 0 || iHnd > m_pstPoolHead->iMax )
	{
		assert( false );
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Handle [%d] error!", iHnd );
		return  NULL; 
	}

	int iPos = iHnd - 1;

	SMemBlock* pstBlock = MEMPOOL_GET_BLOCK(m_pstPoolHead, iPos);
	if( !pstBlock->cUsed )
	{
		//assert( false );
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Data not used! Handle [%d]", iHnd );
		return NULL;
	}

	return (TData*)MEMPOOL_GET_DATA( m_pstPoolHead, iPos );
}


template< typename TData >
TData* CMemPool<TData>::GetDataByPos( int iPos )
{
	assert(m_pstPoolHead);
	
	if( iPos < 0 || iPos >= m_pstPoolHead->iMax )
	{
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Pos [%d] error!", iPos );
		return  NULL; 
	}

	return (TData*)MEMPOOL_GET_DATA( m_pstPoolHead, iPos );
}


template< typename TData >
void CMemPool<TData>::CleanPool()
{
	assert( m_pstPoolHead );

	m_pstPoolHead->iUsedNum 	= 0;
	TLIST_INIT( &(m_pstPoolHead->stUsedListHead) );
	_InitFreeList();
}


template< typename TData >
char CMemPool<TData>::GetUsedFlag( HANDLE iHnd )
{
	assert(m_pstPoolHead);
	if( iHnd <= 0 || iHnd > m_pstPoolHead->iMax )
	{
		assert( false );
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "Handle [%d] error!", iHnd );
		return  -1; 
	}

	int iPos = iHnd - 1;
	SMemBlock* pstBlock = MEMPOOL_GET_BLOCK(m_pstPoolHead, iPos);
	return pstBlock->cUsed;
}


#if 0
template< typename TData >
TData* CMemPool<TData>::IterCurr( )
{
	if( IsIterEnd() )
	{
		return NULL;
	}

	SMemBlock* pstBlock = TLIST_ENTRY( m_pstUsedIter, SMemBlock, stNode );
	assert( pstBlock->cUsed );

	return (TData*)MEMPOOL_BLOCK_2_DATA(m_pstPoolHead, pstBlock);
}
#endif

template< typename TData >
void CMemPool<TData>::_InitFreeList()
{
	assert(m_pstPoolHead);

	TLIST_INIT( &(m_pstPoolHead->stFreeListHead) );

	SMemBlock* pstBlock;
	for( int i = 0; i < m_pstPoolHead->iMax; i++ )
	{	
		pstBlock = MEMPOOL_GET_BLOCK( m_pstPoolHead, i );
		pstBlock->cUsed = 0;
		TLIST_INSERT_NEXT( &(m_pstPoolHead->stFreeListHead), &(pstBlock->stNode));
	}
}


template< typename TData >
void CMemPool<TData>::DestroyPool()
{
	if( NULL == m_pstPoolHead )
	{
		return;
	}
	
	if( m_pstPoolHead->iShmKey )
	{
		DetachShm( (void*)m_pszBuff );
	}else
	{
		delete[] m_pszBuff;
	}

	m_pszBuff = NULL;
	m_pstPoolHead = NULL;
}

// ȫ������һ��replacement new,������һ�ι��캯��
template< typename TData >
void CMemPool<TData>::ConstructAll()
{
	void* pvAddr = NULL;
	for( int i=0; i < m_pstPoolHead->iMax; i++ )	
	{
		pvAddr = MEMPOOL_GET_DATA( m_pstPoolHead, i );
		::new(pvAddr) TData;
	}
}

#endif

