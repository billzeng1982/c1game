#ifndef _POINT_H_
#define _POINT_H_

#include <ext/hash_set>
#include <ext/pool_allocator.h>
#include "define.h"
#include "hash_func.h"

using namespace __gnu_cxx;
using namespace std;

// 注意，不能自定义构造，析构和重载=，否则无法放到union里
struct SPoint2D
{
	int16_t m_shPosX;
	int16_t m_shPosY;

	bool operator == ( const SPoint2D& rhs ) const
	{
		return ( m_shPosX == rhs.m_shPosX ) && ( m_shPosY == rhs.m_shPosY );
	}

	bool operator != ( const SPoint2D& rhs ) const
	{
		return ( m_shPosX != rhs.m_shPosX ) || ( m_shPosY != rhs.m_shPosY );
	}

	SPoint2D operator + ( const SPoint2D &rhs ) const
	{
		SPoint2D stSum;
		stSum.m_shPosX = m_shPosX + rhs.m_shPosX;
		stSum.m_shPosY = m_shPosY + rhs.m_shPosY;
		return stSum;
	}

	SPoint2D operator - ( const SPoint2D &rhs ) const
	{
		SPoint2D stSub;
		stSub.m_shPosX = m_shPosX - rhs.m_shPosX;
		stSub.m_shPosY = m_shPosY - rhs.m_shPosY;
		return stSub;
	}	
};


struct SPoint2F
{
	float m_fPosX;
	float m_fPosY;

	bool operator == ( const SPoint2F& rhs ) const
	{
		return ( m_fPosX == rhs.m_fPosX ) && ( m_fPosY == rhs.m_fPosY );
	}
};


bool HashCmp_Point2D( void* key1, void* key2 );

struct SPoint2D_Ptr_Equal
{
	bool operator()( const SPoint2D* pstPos1, const SPoint2D* pstPos2 ) const
	{
		return ( pstPos1->m_shPosX == pstPos2->m_shPosX ) &&
			   ( pstPos1->m_shPosY == pstPos2->m_shPosY );
	}
};

struct SPoint2D_Equal
{
	bool operator() ( const SPoint2D& rstPos1,  const SPoint2D& rstPos2 ) const
	{	
		return (rstPos1.m_shPosX == rstPos2.m_shPosX) && (rstPos1.m_shPosY == rstPos2.m_shPosY);
	}
};

struct SPoint2D_Less
{
	bool operator() ( const SPoint2D& rstPos1,  const SPoint2D& rstPos2 ) const
	{	
		if( rstPos1.m_shPosX < rstPos2.m_shPosX )
		{
			return true;
		}
		else if(rstPos1.m_shPosX > rstPos2.m_shPosX)
		{
			return false;
		}else
		{
			return (rstPos1.m_shPosY < rstPos2.m_shPosY);
		}
	}
};

struct SPoint2D_Ptr_Hash
{
	size_t operator()( const SPoint2D* pstPos ) const
	{
		return size_t( zend_inline_hash_func( pstPos, sizeof(SPoint2D) ) );
	}
};

struct SPoint2D_Hash
{
	size_t operator()( const SPoint2D& rstPos ) const
	{
		return size_t( zend_inline_hash_func( &rstPos, sizeof(SPoint2D) ) );
	}
};

int CalcNeighbor( const SPoint2D * rstCenter, SPoint2D astNeighbor[] );

// 计算欧几里德距离
int CalcEuclideanDist( const SPoint2D& rstPosA, const SPoint2D& rstPosB );

typedef hash_set< SPoint2D, SPoint2D_Hash, SPoint2D_Equal, __pool_alloc<SPoint2D> > Point2DHashSet_t;

extern SPoint2D g_astEvenNeighborVec[];
extern SPoint2D g_astOddNeighborVec[];
extern SPoint2D g_astQuadNeighborVec[];
#endif

