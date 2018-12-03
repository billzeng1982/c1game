#include "UdpRelay.h"
#include "ss_proto.h"
//#include "oi_misc.h"
#include "FakeRandom.h"
#include "common_proto.h"
#include <stdlib.h>

// usage: ./UdpRelayCltTest ip port procid
using namespace std;
using namespace PKGMETA;
static int CompareExploreOreUid(const void* pA, const void* pB)
{
	if ( ((DT_MINE_EXPLORE_ORE_INFO*)pA)->m_ullOreUid > ((DT_MINE_EXPLORE_ORE_INFO*)pB)->m_ullOreUid )
	{
		return 1;
	}
	else if ( ((DT_MINE_EXPLORE_ORE_INFO*)pA)->m_ullOreUid < ((DT_MINE_EXPLORE_ORE_INFO*)pB)->m_ullOreUid) 
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int MyBSearch(const void *key, const void *base, size_t nmemb, size_t size, int *piEqual, int(*compar) (const void *, const void *))
{
	size_t l, u, idx;
	const void *p, *p2;
	int comparison, comparison2;

	*piEqual = 0;
	if (!nmemb) return 0;
	l = 0;
	u = nmemb;

	while (l < u)
	{
		idx = (l + u) / 2;
		p = (void *)(((const char *)base) + (idx * size));
		comparison = (*compar) (key, p);

		if (comparison == 0)
		{
			*piEqual = 1;
			return idx;
		}
		else if (comparison < 0)
		{
			if (idx == 0) return idx;

			p2 = (void *)(((const char *)base) + ((idx - 1) * size));
			comparison2 = (*compar) (key, p2);

			if (comparison2 > 0) return idx;

			u = idx;
		}
		else /*if (comparison > 0)*/
		{
			l = idx + 1;
		}
	}

	return u;
}

int MyBInsert(const void *key, const void *base, size_t *pnmemb, size_t size, int iUnique, int(*compar) (const void *, const void *))
{
	int i, iInsert, iEqu;

	iInsert = MyBSearch(key, base, *pnmemb, size, &iEqu, compar);
	printf("Insert index<%d> cur num<%d>", iInsert, *pnmemb);
	if (iEqu && iUnique) return 0;
	if (iInsert < *pnmemb)
		for (i = *pnmemb; i > iInsert; i--) memcpy((char *)base + i * size, (char *)base + (i - 1)*size, size);
	memcpy((char *)base + iInsert*size, key, size);
	(*pnmemb)++;
	return 1;
}

int main( int argc, char** argv )
{
	DT_MINE_PLAYER_INFO m_stPlayerInfo = { 0 };
	int bExploreOreCount = 0;
	DT_MINE_EXPLORE_ORE_INFO tmp = { 0 };
	//uint64_t ullArray[] = { 1580213209137158 ,1580213209146374 ,1580213209148422 ,1580111273950214 ,1580847256833030 };
	uint64_t ullArray[] = { 1580213209146374, 1580213209137158 ,1580213209148422 ,1580111273950214 ,1580847256833030 };
	//uint64_t ullArray[] = { 2,3,4,1,5 };
	size_t nmemb = 0;
	for (int i = 0; i < 5; i++)
	{
		tmp.m_ullOreUid = ullArray[i];
		if (!MyBInsert(&tmp, m_stPlayerInfo.m_astExploreOreIdList, &nmemb, sizeof(DT_MINE_EXPLORE_ORE_INFO), 1, CompareExploreOreUid))
		{
			printf("Uin<%lu> UpdateExploreOreList  err!", m_stPlayerInfo.m_ullUin);
			return 0;
		}
		m_stPlayerInfo.m_bExploreOreCount = nmemb;
		printf("List:<%lu> | <%lu> | <%lu> | <%lu> | <%lu> \n", m_stPlayerInfo.m_astExploreOreIdList[0].m_ullOreUid, m_stPlayerInfo.m_astExploreOreIdList[1].m_ullOreUid,
			m_stPlayerInfo.m_astExploreOreIdList[2].m_ullOreUid, m_stPlayerInfo.m_astExploreOreIdList[3].m_ullOreUid, m_stPlayerInfo.m_astExploreOreIdList[4].m_ullOreUid);
	}
	bool bMin = true;
	for (int i = 0; i + 1 < m_stPlayerInfo.m_bExploreOreCount; i++)
	{
		if (m_stPlayerInfo.m_astExploreOreIdList[i].m_ullOreUid >= m_stPlayerInfo.m_astExploreOreIdList[i+1].m_ullOreUid)
		{
			bMin = false;
			break;
		}
	}
	printf("Min <%hhu>", bMin);
    return 0;
}

