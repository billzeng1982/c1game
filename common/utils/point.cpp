#include "point.h"
#include <math.h>

// 方向: 从正右方向开始的逆时针方向
SPoint2D g_astEvenNeighborVec[6] = { {1,0}, {0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1} }; // 偶
SPoint2D g_astOddNeighborVec[6] = { {1,0}, {1,1}, {0,1}, {-1,0}, {0,-1}, {1,-1} }; // 奇

// 城内四方格邻居, 从正右方向逆时针方向
SPoint2D g_astQuadNeighborVec[4] = { {1,0}, {0,-1}, {-1,0}, {0,1} }; 

bool HashCmp_Point2D( void* key1, void* key2 )
{
    return *(SPoint2D*)key1 == *(SPoint2D*)key2;
}

int CalcNeighbor( const SPoint2D* rstCenter, SPoint2D astNeighbor[] )
{
    SPoint2D* pastNeighborVec = ( rstCenter->m_shPosY % 2) ? g_astOddNeighborVec : g_astEvenNeighborVec;
    SPoint2D stNeighborPos;
    int iNum = 0;
    
    for( int i = 0; i < 6; i++ )
    {
        stNeighborPos.m_shPosX = rstCenter->m_shPosX + pastNeighborVec[i].m_shPosX;
        stNeighborPos.m_shPosY = rstCenter->m_shPosY + pastNeighborVec[i].m_shPosY;
        
        astNeighbor[iNum++] = stNeighborPos;
    }

    return iNum;
}


int CalcEuclideanDist( const SPoint2D& rstPosA, const SPoint2D& rstPosB )
{
    return  (int)sqrt( pow( ( rstPosA.m_shPosX - rstPosB.m_shPosX ), 2 ) + pow( ( rstPosA.m_shPosY - rstPosB.m_shPosY ), 2 ) );
}

