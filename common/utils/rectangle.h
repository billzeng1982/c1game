#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_

#include "define.h"
#include "point.h"

/*
	���뱣֤width��height > 0
*/

struct SRect
{
	int16_t m_nX;
	int16_t m_nY;
	int16_t m_nW; // width > 0
	int16_t m_nH; // height > 0

	SRect()
	{
		m_nX = 0;
		m_nY = 0;
		m_nW = 0;
		m_nH = 0;
	}
	
	SRect(int16_t nX, int16_t nY, int16_t nW, int16_t nH)
	{
		m_nX = nX;
		m_nY = nY;
		m_nW = ( nW < 0 ? -nW : nW ) ;
		m_nH = ( nH < 0 ? -nH : nH );
	}

	// �ж��������Ƿ��ཻ
	bool IsIntersect( const SRect& rRect )
	{
		return ( m_nX + m_nW >= rRect.m_nX ) &&
           	   ( rRect.m_nX + rRect.m_nW >= m_nX ) &&
               ( m_nY + m_nH >= rRect.m_nY ) &&
               ( rRect.m_nY + rRect.m_nH >= m_nY ); 
	}

	// �ж�һ�����Ƿ��ھ�����
	bool IsPointInRect( const SPoint2D& rstPos )
	{
		return this->IsPointInRect( rstPos.m_shPosX, rstPos.m_shPosY ); 
	}
	
	bool IsPointInRect( int16_t nX, int16_t nY )
	{
		return ( nX >= m_nX ) &&
		   	   ( nX <= m_nX + m_nW ) &&
		       ( nY >= m_nY ) &&
		       ( nY <= m_nY + m_nH );
	}
};

#endif

