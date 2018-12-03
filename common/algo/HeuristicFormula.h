#ifndef _HEURISTIC_FORMULA_H_
#define _HEURISTIC_FORMULA_H_

#include "point.h"

enum EHEURISTIC_TYPE
{
	HEURISTIC_MANHATTAN = 1,
	HEURISTIC_MAXDXDY,
	HEURISTIC_DIAGONAL_SHORT_CUT,
	HEURISTIC_EUCLIDEAN,
	HEURISTIC_EUCLIDEAN_NOSQR,
	HEURISTIC_CUSTOM,
};

class IHeuristic
{
public:
	IHeuristic() : m_iHEstimate( 1 )
	{ }

	virtual ~IHeuristic( ) {}
	
	virtual int CalcHeuristic( const SPoint2D& rstCurPos, const SPoint2D& rstEndPos ) = 0;

	void SetHEstimate( int iHEstimate ) { m_iHEstimate = iHEstimate; }

protected:
	int m_iHEstimate;
};


class CHeuristicCustom : public IHeuristic
{
public:
	CHeuristicCustom(){}
	virtual ~CHeuristicCustom(){}
	
	virtual int CalcHeuristic( const SPoint2D& rstCurPos, const SPoint2D& rstEndPos );
};


class CHeuristicEuclidean : public IHeuristic
{
public:
	CHeuristicEuclidean() {}
	virtual ~CHeuristicEuclidean() {}

	virtual int CalcHeuristic( const SPoint2D& rstCurPos, const SPoint2D& rstEndPos );
};

class CHeuristicFactory
{
public:
	CHeuristicFactory(){}
	~CHeuristicFactory() {}
	
	IHeuristic* GetHeuristic( uint16_t wType );
	
private:
	CHeuristicCustom	m_oHCustom;
	CHeuristicEuclidean m_oHEuclidean;
};


#endif

