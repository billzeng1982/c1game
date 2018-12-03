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

	// ����һ�������
	uint32_t Random();

	// ����һ��[0, uiRange - 1]֮��������
	uint32_t Random( uint32_t uiRange );

	// ����һ��[uiMinRange, uiMaxRange - 1]֮��������
	uint32_t Random( uint32_t uiMinRange, uint32_t uiMaxRange );

	// ��һ�����ʳ齱�������Ƿ����
	bool Lottery( float fRate, uint32_t uiAccuracy = 1000 );

    //��[0, uiRange - 1]֮������uiCount�����ظ��������
    void Random(uint32_t uiRange, uint32_t uiCount, uint32_t* pResult);

private:
	uint32_t m_uiSeed;
};

#endif

