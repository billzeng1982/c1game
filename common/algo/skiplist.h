/*
	主要参考redis的skiplist实现 (t_zset.c)，并实现改造和模版化
	构造有序集合核心数据结构(比如用于排行榜)
    使用此SkipList时，需要提供仿函数COMPARE，用于比较score和key的大小
	tangxing 2016/1/30

    更新: 2017/2/6 by tangxing
*/


#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <vector>
using namespace std;

template <typename SCORE, typename KEY, typename COMPARE>
class SkipList
{
public:
    //向skiplist中插入元素并返回插入后的rank，若插入的key已存在，返回-1
    int Insert(const SCORE& score, const KEY& key);

    //更新skiplist中的元素并返回更新后的rank，若更新的key不存在，返回-1
    int Update(const SCORE& oldScore, const SCORE& newScore, const KEY& key);

    //删除skiplist中的元素，若删除的key不存在，返回false
    bool Delete(const SCORE& score, const KEY& key);

    //得到skiplist中key的排名，若key不存在，返回-1
    int GetRank(const SCORE& score, const KEY& key);

    //获取skiplist中的最后一个元素，将值保存在score和key中
    bool GetTail(SCORE& score, KEY& key);

    //弹出skiplist中最后一个元素
    void PopTail();

    //获取skiplist中前N个KEY
    int GetObjTopN( int n, vector<KEY>& v);

    //获取skiplist中前N个SCORE
    int GetScoreTopN( int n, vector<SCORE>& v);

    //获取skiplist的长度
    int Length() { return m_length; }

	// 获取排名第N的SCORE
	SCORE* GetScoreAtN( int n );

    void PrintMe();

    SkipList(int level = SKIPLIST_DEFAULT_LEVEL)
    {
        this->_Creat(level);
    }

    ~SkipList()
    {
        this->_Free();
    }

private:
    struct SkipListNode
    {
        SCORE score;
        KEY key;
        SkipListNode* backward;
        struct SkipListLevel
        {
            SkipListNode* forward;
            int span;
            SkipListLevel() : forward(NULL), span(0)
            { }
        };
        SkipListLevel* level;

        SkipListNode() : backward(NULL), level(NULL)
        { }

        ~SkipListNode()
        {
            if (level)
            {
                delete[] level;
                level = NULL;
            }
        }
    };

    // 14层可以较好的支持2^14 个元素以内的skiplist
    const static int SKIPLIST_DEFAULT_LEVEL = 14;

    //skiplist最大为32层，层数过高会降低效率
    const static int SKIPLIST_MAX_LEVEL = 32;

    // skiplist P = 1/4
	const static float SKIPLIST_P = 0.25;

private:
    //创建一个层数为level的SkipList节点
    SkipListNode* _CreatNode(const SCORE& score, const KEY& key, int level)
    {
        SkipListNode* node = new SkipListNode();
        if (node==NULL)
        {
            return NULL;
        }

        node->score = score;
        node->key = key;
        node->level = new typename SkipListNode::SkipListLevel[level];

        return node;
    }

    void _FreeNode(SkipListNode* node)
    {
        delete node;
    }

    void _Creat( int MaxLevel);
    void _Free();
    int _RandomLevel();
    void DeleteNode(SkipListNode* x, SkipListNode** update);

private:
    struct SkipListNode *m_header, *m_tail;
    int m_maxLevel;
    int m_level;
    int m_length; // 当前节点数量
    COMPARE m_comparefunc;
};

template <typename SCORE, typename KEY, typename COMPARE>
void SkipList<SCORE, KEY, COMPARE>::_Creat( int MaxLevel)
{
    if (MaxLevel > SKIPLIST_MAX_LEVEL)
    {
	    MaxLevel = SKIPLIST_MAX_LEVEL;
    }

    m_maxLevel = MaxLevel;
    m_level = 1;
    m_length = 0;

    //初始化表头
    SCORE tmpScore;
    KEY tmpKey;
    m_header = _CreatNode(tmpScore, tmpKey, MaxLevel);

    //设置表尾
    m_tail = NULL;

    //设置随机种子
    srandom( time(NULL) );
}


template <typename SCORE, typename KEY, typename COMPARE>
void SkipList<SCORE, KEY, COMPARE>::_Free()
{
    SkipListNode* x, *next;
    x = m_header;
    while(x)
    {
        next = x->level[0].forward;
        _FreeNode(x);
        x = next;
    }
}

template <typename SCORE, typename KEY, typename COMPARE>
int SkipList<SCORE, KEY, COMPARE>:: _RandomLevel()
{
    int level = 1;
    while ((random()&0xFFFF) < (SKIPLIST_P * 0xFFFF))
        level += 1;

    return (level < m_maxLevel) ? level : m_maxLevel;
}

template <typename SCORE, typename KEY, typename COMPARE>
int SkipList<SCORE, KEY, COMPARE>::Insert(const SCORE& score, const KEY& key)
{
    SkipListNode* x = m_header;
    //用来记录遍历过程中，每一层上将要与新节点连接的节点
    SkipListNode* update[m_maxLevel];
    int rank[m_maxLevel];

    for (int i = m_level -1; i >= 0; i--)
    {
        rank[i] = (i == (m_level -1)) ? 0 : rank[i + 1];
        while (x->level[i].forward &&
                (1 == m_comparefunc(score, x->level[i].forward->score)
                  ||(0 == m_comparefunc(score, x->level[i].forward->score) && 1 == m_comparefunc(key, x->level[i].forward->key))))
        {
            // 如果 i 不是m_level -1 层, 那么 i 层的起始 rank 值为 i+1 层的 rank 值
            // 各个层的 rank 值一层层累积, 最终 rank[0] 的值加一就是新节点的前置节点的排位
            // rank[0] 会在后面成为计算 span 值和 rank 值的基础
            rank[i] += x->level[i].span;
            x = x->level[i].forward;
        }
        // 记录将要和新节点相连接的节点
        update[i] = x;
    }

    // 获取一个随机值作为新节点的层数
    int level = _RandomLevel();

    // 如果新节点的层数比表中其他节点的层数都要大,那么初始化表头节点中未使用的层，
    //并将它们记录到 update 数组中, 将来也指向新节点
    if (level > m_level)
    {
        // 初始化未使用层
        for (int i = m_level; i < level; i++)
        {
            rank[i] = 0;
            update[i] = m_header;
            update[i]->level[i].span = m_length;
        }
        // 更新表中节点最大层数
        m_level = level;
    }

    // 创建新节点
    x = _CreatNode(score, key, level);

    // 将前面记录的指针指向新节点，并做相应的设置
    for (int i = 0; i < level; i++)
    {
        // 设置新节点的 forward 指针
        x->level[i].forward = update[i]->level[i].forward;

        // 将沿途记录的各个节点的 forward 指针指向新节点
        update[i]->level[i].forward = x;

        //计算新节点跨越的节点数量
        x->level[i].span = update[i]->level[i].span -(rank[0] - rank[i]);

        // 更新新节点插入之后，沿途节点的 span 值
        update[i]->level[i].span = rank[0] - rank[i] + 1;
    }

    // 未接触的节点的 span 值也需要增一，这些节点直接从表头越过新节点指向新节点后面的节点
    for (int i = level; i < m_level; i++)
    {
        update[i]->level[i].span++;
    }

    //设置新节点的前一个节点
    x->backward = update[0] == m_header ? NULL : update[0];

    if (x->level[0].forward)
    {
        x->level[0].forward->backward = x;
    }
    else
    {
        m_tail = x;
    }

    m_length++;

    return rank[0] + 1;
}



template <typename SCORE, typename KEY, typename COMPARE>
void SkipList<SCORE, KEY, COMPARE>::DeleteNode(SkipListNode* x, SkipListNode** update)
{
    // 更新所有和被删除节点 x 有关的节点
    for (int i = 0; i < m_level; i++)
    {
        if (update[i]->level[i].forward == x)
        {
            update[i]->level[i].forward = x->level[i].forward;
            update[i]->level[i].span += x->level[i].span -1;
        }
        else
        {
            update[i]->level[i].span--;
        }
    }

    //更新被删除节点x的后一个节点的后退指针
    if (x->level[0].forward)
    {
        x->level[0].forward->backward = x->backward;
    }
    else
    {
        m_tail = x->backward;
    }

    //若被删除节点是跳跃表中唯一的最高节点，更新跳跃表当前层数
    while((m_level > 1) && (m_header->level[m_level-1].forward == NULL))
    {
        m_level--;
    }

    m_length--;
}


template <typename SCORE, typename KEY, typename COMPARE>
bool SkipList<SCORE, KEY, COMPARE>::Delete(const SCORE& score, const KEY& key)
{
    SkipListNode* x = m_header;
    SkipListNode* update[m_level];

    for (int i = m_level -1; i >= 0; i--)
    {
        while (x->level[i].forward &&
                (1 == m_comparefunc(score, x->level[i].forward->score)
                    ||(0 == m_comparefunc(score, x->level[i].forward->score) && 1 == m_comparefunc(key, x->level[i].forward->key))))
        {
            x = x->level[i].forward;
        }
        // 记录将要和新节点相连接的节点
        update[i] = x;
    }

    x = x->level[0].forward;
    if (x && 0 == m_comparefunc(x->score, score) && 0 == m_comparefunc(x->key, key))
    {
        this->DeleteNode(x, update);
        this->_FreeNode(x);
        return true;
    }
    else
    {
        return false;
    }
}

template <typename SCORE, typename KEY, typename COMPARE>
int SkipList<SCORE, KEY, COMPARE>::Update(const SCORE& oldScore, const SCORE& newScore,const KEY& key)
{
    if (!this->Delete(oldScore, key))
    {
        return -1;
    }

    return this->Insert(newScore, key);
}

template <typename SCORE, typename KEY, typename COMPARE>
int SkipList<SCORE, KEY, COMPARE>::GetRank(const SCORE& score, const KEY& key)
{
    SkipListNode* x = m_header;
    int rank = 0;

    for (int i = m_level -1; i >= 0; i--)
    {
        while (x->level[i].forward &&
            (1 == m_comparefunc(score, x->level[i].forward->score)
                    ||(0 == m_comparefunc(score, x->level[i].forward->score) && 1 == m_comparefunc(key, x->level[i].forward->key))))
        {
            rank += x->level[i].span;
            x = x->level[i].forward;
        }
    }

    x = x->level[0].forward;

    if (x && 0 == m_comparefunc(score, x->score) && 0 == m_comparefunc(key, x->key))
    {
        return rank + 1;
    }

    return -1;
}


template <typename SCORE, typename KEY, typename COMPARE>
bool SkipList<SCORE, KEY, COMPARE>::GetTail(SCORE& score, KEY& key)
{
    if (m_tail)
    {
        score = m_tail->score;
        key = m_tail->key;
        return true;
    }
    else
    {
        return false;
    }
}


template <typename SCORE, typename KEY, typename COMPARE>
void SkipList<SCORE, KEY, COMPARE>::PopTail()
{
	if( m_tail == NULL )
	{
		return;
	}

	this->Delete(m_tail->score, m_tail->key);
}


template <typename SCORE, typename KEY, typename COMPARE>
void SkipList<SCORE, KEY, COMPARE>::PrintMe()
{
/*
	printf("lenth: %u\n", m_length);
	printf("level: %d, max level: %d\n", m_level, m_maxLevel);
	printf("Head:");
	for( int j = 0; j < m_level; j++ )
	{
        if (m_header->level != NULL)
        {
		    printf("level[%d], span:%d\n", j, m_header->level[j].span);
        }
	}
	printf("---------------------\n");

	int i = 1;
	SkipListNode *x = m_header->level[0].forward;
	while( x != NULL )
	{
		printf("node %d: \n", i);
// 		printf("level: %d \n", x->levelLength );
// 		for(int j=0; j < (int)x->levelLength; j++ )
// 		{
//             if (x->level != NULL)
//             {
//                 printf("\tlevel[%d], span: %d\n", j ,x->level[j].span);
//             }
// 		}
        x->score.ShowMe();
		printf("---------------------\n");
		x = x->level[0].forward;
		i++;
	}
*/
}

template <typename SCORE, typename KEY, typename COMPARE>
int SkipList<SCORE, KEY, COMPARE>::GetObjTopN( int n, vector<KEY>& v)
{
	int i = 0;
	SkipListNode *x = m_header->level[0].forward;

	while(x && i < n)
	{
		v.push_back(x->key);
		x = x->level[0].forward;
		i++;
	}
    return i;
}

template <typename SCORE, typename KEY, typename COMPARE>
int SkipList<SCORE, KEY, COMPARE>::GetScoreTopN( int n, vector<SCORE>& v)
{
    int i = 0;
	SkipListNode *x = m_header->level[0].forward;

	while(x && i < n)
	{
		v.push_back(x->score);
		x = x->level[0].forward;
		i++;
	}

    return i;
}

// 带优化: 利用 span，减少遍历
template <typename SCORE, typename KEY, typename COMPARE>
SCORE* SkipList<SCORE, KEY, COMPARE>::GetScoreAtN( int n )
{
	if( n > m_length )
	{
		return NULL;
	}

	int i = 1;
	SkipListNode *x = m_header->level[0].forward;

	while(x && i < n)
	{
		x = x->level[0].forward;
		i++;
	}

    if (x)
    {
        return &x->score;
    }
    else
    {
        return NULL;
    }
}

