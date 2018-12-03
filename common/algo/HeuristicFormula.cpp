#include <stdlib.h>
#include <math.h>
#include "HeuristicFormula.h"


int CHeuristicCustom::CalcHeuristic( const SPoint2D& rstCurPos, const SPoint2D& rstEndPos )
{
    SPoint2D stDxy;
    stDxy.m_shPosX = (int16_t)abs( rstEndPos.m_shPosX - rstCurPos.m_shPosX );
    stDxy.m_shPosY = (int16_t)abs( rstEndPos.m_shPosY - rstCurPos.m_shPosY );
    int16_t shOrthogonal = abs( stDxy.m_shPosX - stDxy.m_shPosY );

    int iDiagonal = abs( ( (stDxy.m_shPosX+stDxy.m_shPosY) - shOrthogonal ) / 2 );

    return m_iHEstimate * ( iDiagonal + shOrthogonal + stDxy.m_shPosX + stDxy.m_shPosY );
}


int CHeuristicEuclidean::CalcHeuristic( const SPoint2D& rstCurPos, const SPoint2D& rstEndPos )
{
    return  m_iHEstimate * (int)sqrt( pow( ( rstCurPos.m_shPosX - rstEndPos.m_shPosX ), 2 ) + pow( ( rstCurPos.m_shPosY - rstEndPos.m_shPosY ), 2 ) );
}

IHeuristic* CHeuristicFactory::GetHeuristic( uint16_t wType )
{
    switch( wType )
    {
        case HEURISTIC_CUSTOM :     return &m_oHCustom;
        case HEURISTIC_EUCLIDEAN:   return &m_oHEuclidean;
        default: return &m_oHCustom;
    }
}


