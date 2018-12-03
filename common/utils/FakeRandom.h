#ifndef _FAKE_RANDOM_H_
#define	_FAKE_RANDOM_H_

#include "singleton.h"
#include "define.h"

class CFakeRandom : public TSingleton<CFakeRandom>
{
public:
	CFakeRandom() : m_uiSeed( 0 )
	{ }

	~CFakeRandom() {}

	void SetSeed( uint32_t uiSeed ) { m_uiSeed = uiSeed; }

	// 生成一个随机数
	uint32_t Random();

	// 生成一个[0, uiRange - 1]之间的随机数
	uint32_t Random( uint32_t uiRange );

	// 生成一个[uiMinRange, uiMaxRange - 1]之间的随机数
	uint32_t Random( uint32_t uiMinRange, uint32_t uiMaxRange );

	// 以一定几率抽奖，返回是否抽中
	bool Lottery( float fRate, uint32_t uiAccuracy = 1000 );

    //在[0, uiRange - 1]之间生成uiCount个不重复的随机数
    void Random(uint32_t uiRange, uint32_t uiCount, uint32_t* pResult);

private:
	uint32_t m_uiSeed;
};

#endif

