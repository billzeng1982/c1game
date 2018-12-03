#pragma once

#include <iostream>
#include <map>
#include <queue>
#include <vector>
#include <set>
#include "skiplist.h"
#include "DynMempool.h"
#include "list_i.h"
#include "MyTdrBuf.h"

template<typename KEY, typename OBJ, typename COMPARE>
class RankSys
{
private:
    static const int DEFAULT_NODESIZE = 1000;
    static const int DELTA_NODESIZE = 100;
    static const int CONST_MAX_NODE_NUM = 5000;
    static const int CONST_DIRTYNODE_MAX = 50;
    static const int CONST_BUFFSIZE = 10240;
    static const int CONST_SIZE_OBJ = sizeof(OBJ);
    
public:
	RankSys() : m_fp(NULL), m_iCapacity(0), m_stTdrBuf(CONST_BUFFSIZE)
    {
        TLIST_INIT(&m_stDirtyListHead);
        m_stTdrBuf.Reset();
    }

	~RankSys();

	//初始化，pName为保存数据的文件名
	bool Init(const char* pszName, int iVersion);

	//根据key获取其对应的数据项的排行
	int GetRank(KEY Key);

	//根据key更新排行榜中某项的数据，并根据更新的数据返回新的排行
	int Update(KEY Key, const OBJ& Obj);

	//获取排行榜中前N项key
	int GetTopKey(int n, std::vector<KEY>& v);

	//获取排行榜中前N项obj
	int GetTopObj(int n, std::vector<OBJ>& v);

	//清除排行榜
	void ClearRank();

    //根据排名获取obj
    OBJ* GetObjAtN(int n) { return m_ObjKeySkipList.GetScoreAtN(n); }

	//根据key删除排行榜中某项的数据
	bool Delete(KEY Key);
    int Size() { return m_stFileHead.m_iNodeNum; }

	void OnExit()
	{
        LOGRUN("Table<%s> OnExit start!", m_szFileName);
		if (m_fp != NULL)
		{
            this->_WriteToFile();
            LOGRUN("Table<%s> OnExit OK!", m_szFileName);
			fclose(m_fp);
            m_fp = NULL;
		}
        LOGRUN("Table<%s> OnExit end!", m_szFileName);
	}

	void PrintMe();

private:
	struct FileHead
	{
		int m_iVersion;     //版本号
		int m_iOffset;      //文件可用地址    这个字段不用 
		int m_iNodeSize;    //节点大小
		int m_iNodeNum;     //节点数量
	} ;

	struct RankSysNode
	{
		OBJ m_Obj;
        int m_iFileOffset;
        TLISTNODE m_stDirtyListNode;
	};

    struct OffsetNode
    {
        OffsetNode() {}
        OffsetNode(int iOffset, RankSysNode *poRankSysNode):m_iOffset(iOffset), m_poRankSysNode(poRankSysNode){}
        int m_iOffset;
        RankSysNode * m_poRankSysNode;
        bool operator > (const OffsetNode &t2) const
        {
             return this->m_iOffset > t2.m_iOffset;
        }
    };

	RankSysNode* _CreatRankSysNode(const OBJ& Obj)
	{
		RankSysNode* pstNode = m_RankSysNodePool.Get();
		if (pstNode)
		{
			pstNode->m_Obj = Obj;
            pstNode->m_iFileOffset = 0;
            TLIST_INIT(&pstNode->m_stDirtyListNode);
		}

		return pstNode;
	}

	FILE * m_fp;

    char m_szFileName[255];

	FileHead m_stFileHead;

    //当前容量
	int m_iCapacity;

    //上次写文件时的节点数量
    int m_iLastWriteNum;

    //脏数据节点数量
    int m_iDirtyNodeNum;

    //脏数据链表头
    TLISTNODE m_stDirtyListHead;

	COMPARE m_stCompareFunc;

	DynMempool<RankSysNode>  m_RankSysNodePool;

	//保存Key -Obj对应关系的map
    std::map<KEY, RankSysNode*> m_KeyNodeMap;
	typename std::map<KEY, RankSysNode*>::iterator m_oMapIter;

	//保存Obj -Key对应关系的SkipList
	SkipList<OBJ, KEY, COMPARE> m_ObjKeySkipList;
    
    MyTdrBuf m_stTdrBuf;

    //被删除的节点在文件中的队列 递减
    std::priority_queue<int, std::vector<int>, std::greater<int> >  m_DelOffsetQueue;
    //RankNode节点偏移队列 递减
    std::set <OffsetNode, std::greater<OffsetNode> > m_OffsetNodeSet;
    typename std::set <OffsetNode, std::greater<OffsetNode> >::iterator m_oOffsetSetIter;

private:
	bool _GetFileHead();
	bool _CreateFile();
	bool _InitDataFromFile();

    void _AddToDirtyList(RankSysNode* pstNode);
	void _WriteToFile();

    void _ClearDelOffsetQueue();

	int _Insert(KEY Key, const OBJ& Obj);
	int _Update(KEY Key,const OBJ& Obj);
    int _GetNewOffset();
};




template<typename KEY, typename OBJ, typename COMPARE>
RankSys<KEY, OBJ, COMPARE>::~RankSys()
{
    LOGRUN("Table<%s> ~RankSys start!", m_szFileName);
    this->OnExit();
    for (m_oMapIter = m_KeyNodeMap.begin(); m_oMapIter != m_KeyNodeMap.end(); m_oMapIter++)
    {
        m_RankSysNodePool.Release(m_oMapIter->second);
    }
    m_KeyNodeMap.clear();
    this->_ClearDelOffsetQueue();
    m_OffsetNodeSet.clear();
    LOGRUN("Table<%s> ~RankSys end!", m_szFileName);
}


template<typename KEY, typename OBJ, typename COMPARE>
bool RankSys<KEY, OBJ, COMPARE>:: _GetFileHead()
{
	fseek(m_fp, 0, SEEK_SET);
	if (fread(&m_stFileHead, sizeof(m_stFileHead), 1, m_fp) != 1)
	{
		LOGERR("RankTable<%s>::get file head failed", m_szFileName);
		return false;
	}

	return true;
}


template<typename KEY, typename OBJ, typename COMPARE>
bool RankSys<KEY, OBJ, COMPARE>::_CreateFile()
{
	m_fp = fopen(m_szFileName, "wb+");
	if (!m_fp)
	{
		LOGERR("RankTable<%s>::create file failed", m_szFileName);
		return false;
	}

	m_stFileHead.m_iNodeNum = 0;
	m_stFileHead.m_iOffset = sizeof(FileHead);

	return true;
}


template<typename KEY, typename OBJ, typename COMPARE>
bool RankSys<KEY, OBJ, COMPARE>::_InitDataFromFile()
{
    KEY stTempKey;
    OBJ stTempObj;
    int iRet = 0;
    int iOffset = 0;
	for (int i = 0; i < m_stFileHead.m_iNodeNum; i++)
	{
		iOffset = sizeof(m_stFileHead) + i * m_stFileHead.m_iNodeSize;
		fseek(m_fp, iOffset, SEEK_SET);
        m_stTdrBuf.Reset();
        iRet = fread(m_stTdrBuf.m_szTdrBuf, m_stFileHead.m_iNodeSize, 1, m_fp);
		if (iRet != 1)
		{
			LOGERR("RankTable<%s>::get file node failed", m_szFileName);
			return false;
		}
        iRet = stTempObj.unpack(m_stTdrBuf.m_szTdrBuf, CONST_BUFFSIZE, &m_stTdrBuf.m_uPackLen, m_stFileHead.m_iVersion);
        if (iRet != 0)
        {
            LOGERR("RankTable<%s>::unpack file node failed, Ret=%d", m_szFileName, iRet);
			return false;
        }
        stTempKey = stTempObj.GetKey();

		RankSysNode* pstNode = this->_CreatRankSysNode(stTempObj);
		if (!pstNode)
		{
			LOGERR("RankTable<%s>:: _CreatRankSysNode failed", m_szFileName);
			return false;
		}
        //  赋值新的位置,采用新的节点大小
        pstNode->m_iFileOffset = sizeof(m_stFileHead) + i * CONST_SIZE_OBJ;

        //加入脏数据链表，当Version变化时，从文件中读出数据后需要再马上重写一遍文件，这里
        //采取不管Version是否变化，都重写一遍的策略
        this->_AddToDirtyList(pstNode);

		m_KeyNodeMap.insert(make_pair(stTempKey, pstNode));
		m_ObjKeySkipList.Insert(stTempObj, stTempKey);
	}

	return true;
}


template<typename KEY, typename OBJ, typename COMPARE>
bool RankSys<KEY, OBJ, COMPARE>::Init(const char* pszName, int iVersion)
{
    if (pszName == NULL || pszName[0] == '\0' )
    {
        LOGERR("RankSys:the RankTableName is NULL!");
        return false;
    }
    strncpy(m_szFileName, pszName, sizeof(m_szFileName));
    //初始化内存池
	if (!m_RankSysNodePool.Init(DEFAULT_NODESIZE, DELTA_NODESIZE, CONST_MAX_NODE_NUM))
	{
		LOGERR("RankTable<%s>::m_RankSysNodePool init failed", m_szFileName);
		return false;
	}

    //初始化文件
    m_fp = fopen(m_szFileName, "rb+");
	if (m_fp != NULL)
	{
        //已存在文件，从文件中读出文件头
		if (!this->_GetFileHead())
		{
			return false;
		}
	}
	else
	{
        //不存在文件，创建文件
		if (!this->_CreateFile())
		{
			return false;
		}
	}
    //LOGWARN("Table<%s> Init NodeNum<%d>, NodeSize<%d>, FileOffset<%d> ", m_szFileName, m_stFileHead.m_iNodeNum, m_stFileHead.m_iNodeSize, m_stFileHead.m_iOffset);
    //从文件中初始化数据
    if (!this->_InitDataFromFile())
    {
        return false;
    }

    
    //赋值新的节点大小和文件位置
    m_stFileHead.m_iNodeSize = CONST_SIZE_OBJ;
    //m_stFileHead.m_iOffset = sizeof(m_stFileHead) + CONST_SIZE_OBJ * m_stFileHead.m_iNodeNum;
    m_stFileHead.m_iVersion = iVersion;
    //如果出现版本变更，需要回写一次文件，此处不论版本是否变更都回写了一次数据库
    this->_WriteToFile();

	return true;
}


template<typename KEY, typename OBJ, typename COMPARE>
inline void RankSys<KEY, OBJ, COMPARE>::_ClearDelOffsetQueue()
{
    while (!m_DelOffsetQueue.empty())
    {
        m_DelOffsetQueue.pop();
    }
}

template<typename KEY, typename OBJ, typename COMPARE>
int RankSys<KEY, OBJ, COMPARE>::_Insert(KEY Key, const OBJ& Obj)
{
    int iRank = 0;
    RankSysNode* pstNode = NULL;
   
    if (m_stFileHead.m_iNodeNum >= CONST_MAX_NODE_NUM)
    {
        //  排行榜数量大于 CONST_MAX_NODE_NUM 就剔除掉最后一个 
        //  然后插入新的进行排序

        OBJ stTempObj;
        KEY stTempKey;

        if (!m_ObjKeySkipList.GetTail(stTempObj, stTempKey))
        {
            return -11;
        }

        if (m_stCompareFunc(stTempObj, Obj) == -1)
        {
            //与排行版最后一名比较,无法上榜
            return -12;
        }

        m_oMapIter = m_KeyNodeMap.find(stTempKey);
        if (m_oMapIter == m_KeyNodeMap.end())
        {
            return -13;
        }
        m_oMapIter->second->m_Obj = Obj;

        //插入新的<Key-Node> 
        m_KeyNodeMap.insert(make_pair(Key, m_oMapIter->second));

        this->_AddToDirtyList(m_oMapIter->second);

        //删除老的<Key-Node>
        m_KeyNodeMap.erase(m_oMapIter);
        //弹出最后一名
        m_ObjKeySkipList.PopTail();
        //插入新的进行排序
        iRank = m_ObjKeySkipList.Insert(Obj, Key);

        
    }
    else
    {
        pstNode = this->_CreatRankSysNode(Obj);
        if (!pstNode)
    	{
    		LOGERR("RankTable<%s>:: _Insert failed, pObj is NULL", m_szFileName);
    		return -14;
    	}
        pstNode->m_iFileOffset = this->_GetNewOffset();
        m_OffsetNodeSet.insert(OffsetNode(pstNode->m_iFileOffset, pstNode));
        m_KeyNodeMap.insert(make_pair(Key, pstNode));
	    iRank = m_ObjKeySkipList.Insert(Obj, Key);
        m_stFileHead.m_iNodeNum++;
        this->_AddToDirtyList(pstNode);
        //LOGWARN("Talbe<%s> Insert OK Offset<%d> NodeNum<%d>", m_szFileName, pstNode->m_iFileOffset, m_stFileHead.m_iNodeNum);
    }

	return iRank;
}


template<typename KEY, typename OBJ, typename COMPARE>
int RankSys<KEY, OBJ, COMPARE>::_Update(KEY  Key, const OBJ& Obj)
{
	int iRank = m_ObjKeySkipList.Update(m_oMapIter->second->m_Obj, Obj, Key);

	m_oMapIter->second->m_Obj = Obj;

	this->_AddToDirtyList(m_oMapIter->second);

	return iRank;
}


template<typename KEY, typename OBJ, typename COMPARE>
int RankSys<KEY, OBJ, COMPARE>::Update(KEY Key, const OBJ& Obj)
{
	int iRank = 0;
	m_oMapIter = m_KeyNodeMap.find(Key);
	if (m_oMapIter != m_KeyNodeMap.end())
	{
		//map中有此key,走update流程
		iRank = this->_Update(Key, Obj);
	}
	else
	{
		//map中无此key,走insert流程
		iRank = this->_Insert(Key, Obj);
	}

	//DirtyList中的元素数量达到临界值，写一次文件
	if (m_iDirtyNodeNum >= CONST_DIRTYNODE_MAX)
	{
		this->_WriteToFile();
	}

	return iRank;
}


template<typename KEY, typename OBJ, typename COMPARE>
void RankSys<KEY, OBJ, COMPARE>::_AddToDirtyList(RankSysNode* pstNode)
{
    TLISTNODE* pstTlistNode = &pstNode->m_stDirtyListNode;
    if (!TLIST_IS_EMPTY(pstTlistNode))
    {
        //已经在DirtyList中
        return;
    }

    //加入DirtyList表头
    TLIST_INSERT_NEXT(&m_stDirtyListHead, pstTlistNode);
    m_iDirtyNodeNum++;
}

template<typename KEY, typename OBJ, typename COMPARE>
void RankSys<KEY, OBJ, COMPARE>::_WriteToFile()
{
    //先刷文件头
    fseek(m_fp, 0, SEEK_SET);
    if (fwrite(&m_stFileHead, sizeof(m_stFileHead), 1, m_fp) != 1)
    {
        LOGERR("RankTable<%s>::WriteToFile failed, write FileHead failed", m_szFileName);
    }

    //刷新脏数据
    TLISTNODE* pstPos = NULL;
    TLISTNODE* pstNext = NULL;
    TLISTNODE* pstHead = &(m_stDirtyListHead);
    RankSysNode* pstNode = NULL;
 
    //把文件最后的节点存储位置设置为已删除的节点位置上,然后加入DirtyList
    while (!m_DelOffsetQueue.empty() && !m_OffsetNodeSet.empty())
    {
        m_oOffsetSetIter = m_OffsetNodeSet.begin();
        if (m_DelOffsetQueue.top() > m_oOffsetSetIter->m_poRankSysNode->m_iFileOffset)
        {
            //删除的节点中最小的位置比当前最大的位置还大,就不需要处理了
            break;
        }
        m_oOffsetSetIter->m_poRankSysNode->m_iFileOffset = m_DelOffsetQueue.top();
        //加入DirtyList
        this->_AddToDirtyList(m_oOffsetSetIter->m_poRankSysNode);
        m_OffsetNodeSet.insert(OffsetNode(m_oOffsetSetIter->m_poRankSysNode->m_iFileOffset, m_oOffsetSetIter->m_poRankSysNode));
        m_OffsetNodeSet.erase(m_oOffsetSetIter);
        m_DelOffsetQueue.pop();
    }
    int iRet = 0;
    TLIST_FOR_EACH_SAFE(pstPos, pstNext, pstHead)
    {
        pstNode = TLIST_ENTRY(pstPos, RankSysNode, m_stDirtyListNode);
        //printf(" Uin=%lu, m_iFileOffset=%d ", pstNode->m_Obj.GetKey(), pstNode->m_iFileOffset);
        fseek(m_fp, pstNode->m_iFileOffset, SEEK_SET);
        m_stTdrBuf.Reset();
        iRet = pstNode->m_Obj.pack((char*)m_stTdrBuf.m_szTdrBuf, CONST_BUFFSIZE, &m_stTdrBuf.m_uPackLen);
        if (iRet != 0)
        {
            LOGERR("RankTable<%s>:: pack Node failed, Ret=%d", m_szFileName, iRet);
        }
        if (fwrite(m_stTdrBuf.m_szTdrBuf, m_stFileHead.m_iNodeSize, 1, m_fp) != 1)
        {
            LOGERR("RankTalbe<%s>:: write node to file failed!! Offset<%u>", m_szFileName, pstNode->m_iFileOffset);
            continue;
        }
        //LOGWARN("Talbe<%s> write node Offset<%d> to file  ", m_szFileName, pstNode->m_iFileOffset);
        TLIST_DEL(pstPos);
    }
    TLIST_INIT(pstHead);
    //LOGWARN("Talbe<%s> write node  m_iDirtyNodeNum<%d> ", m_szFileName, m_iDirtyNodeNum);
    m_iDirtyNodeNum = 0;
    fflush(m_fp);

    //如果已删除节点队列还不为空,就清空,已经无效了.
    this->_ClearDelOffsetQueue();

    return;
}



template<typename KEY, typename OBJ, typename COMPARE>
int RankSys<KEY, OBJ, COMPARE>::GetRank(KEY Key)
{
	m_oMapIter = m_KeyNodeMap.find(Key);
	if (m_oMapIter == m_KeyNodeMap.end())
	{
		return -1;
	}

	return m_ObjKeySkipList.GetRank(m_oMapIter->second->m_Obj, Key);
}

template<typename KEY, typename OBJ, typename COMPARE>
bool RankSys<KEY, OBJ, COMPARE>::Delete(KEY Key)
{
    //排行榜中没有元素
    if (m_stFileHead.m_iNodeNum <= 0)
    {
        return false;
    }

    //未找到对应的key
    m_oMapIter = m_KeyNodeMap.find(Key);
    if (m_oMapIter == m_KeyNodeMap.end())
    {
        LOGERR("RankTable<%s>::Delete failed, not found", m_szFileName);
        return false;
    }

    //删除SkipList中对应的节点
    if (!m_ObjKeySkipList.Delete(m_oMapIter->second->m_Obj, Key))
    {
        LOGERR("RankTable<%s>::Delete  from SkipList failed", m_szFileName);
    }
    //增加已删除的节点位置
    if (m_oMapIter->second->m_iFileOffset >= sizeof(FileHead))
    {
        m_DelOffsetQueue.push(m_oMapIter->second->m_iFileOffset);
    }
    //删除 m_OffsetNodeSet 中的
    m_oOffsetSetIter = m_OffsetNodeSet.find(OffsetNode(m_oMapIter->second->m_iFileOffset, NULL));
    if (m_oOffsetSetIter != m_OffsetNodeSet.end())
    {
        m_OffsetNodeSet.erase(m_oOffsetSetIter);
    }
    TLIST_DEL(&m_oMapIter->second->m_stDirtyListNode);
    //在mempool中删除对应的节点
    m_RankSysNodePool.Release(m_oMapIter->second);
    //在map中删除对应的key
    m_KeyNodeMap.erase(m_oMapIter);

    m_stFileHead.m_iNodeNum--;
    return true;
}

//清除排行榜
template<typename KEY, typename OBJ, typename COMPARE>
void RankSys<KEY, OBJ, COMPARE>::ClearRank()
{
    for (m_oMapIter = m_KeyNodeMap.begin(); m_oMapIter != m_KeyNodeMap.end(); m_oMapIter++)
    {
        this->Delete(m_oMapIter->first);
    }
    this->_ClearDelOffsetQueue();
}

template<typename KEY, typename OBJ, typename COMPARE>
int RankSys<KEY, OBJ, COMPARE>::_GetNewOffset()
{
    int iOffset = 0;
    if (m_DelOffsetQueue.empty())
    {
        iOffset = sizeof(FileHead) + m_stFileHead.m_iNodeSize * m_stFileHead.m_iNodeNum;
    }
    else
    {
        iOffset = m_DelOffsetQueue.top();
        m_DelOffsetQueue.pop();
    }
    return iOffset;
}

template<typename KEY, typename OBJ, typename COMPARE>
int RankSys<KEY, OBJ, COMPARE>::GetTopKey(int n, std::vector<KEY>&v)
{
	return m_ObjKeySkipList.GetObjTopN(n, v);
}


template<typename KEY, typename OBJ, typename COMPARE>
int RankSys<KEY, OBJ, COMPARE>::GetTopObj(int n, std::vector<OBJ>&v)
{
	return m_ObjKeySkipList.GetScoreTopN(n, v);
}


template<typename KEY, typename OBJ, typename COMPARE>
void RankSys<KEY, OBJ, COMPARE>::PrintMe()
{
	m_ObjKeySkipList.PrintMe();
}


