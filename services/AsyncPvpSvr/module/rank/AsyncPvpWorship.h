#include "singleton.h"
#include "ss_proto.h"
#include "../player/AsyncPvpPlayer.h"

using namespace std;
using namespace PKGMETA;


class AsyncPvpWorship : public TSingleton<AsyncPvpWorship>
{
private:
	static const int WORSHIPPED_CANDIDATE_MAX_NUM = 100;
	static const int DAILY_WORSHIPPED_SETTLE_MAIL_ID = 10008;

public:
	bool Init();

	void Update();

	void SendWorshippedReward();

private:
	SSPKG m_stSsPkg;

	//每日结算发奖励的时间
	int m_iSettleTime;
	bool m_bSettleFlag;
};


