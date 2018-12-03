#ifndef _BASE_OBJECT_H_
#define _BASE_OBJECT_H_

#include "list.h"
#include <assert.h>

class IObject
{
public:
	struct list_head m_stListNode;  // 挂接在free list上
	struct list_head m_stUptNode;   // 挂在Updator上面

public:
	IObject() : m_dwID(0), m_wType(0) 
	{
		INIT_LIST_NODE(&m_stListNode);
		INIT_LIST_NODE(&m_stUptNode);
		m_wMemBytes = 0;
	}
	IObject( unsigned short wType )	: m_dwID(0), m_wType(wType) 
	{
		INIT_LIST_NODE(&m_stListNode);
		INIT_LIST_NODE(&m_stUptNode);
		m_wMemBytes = 0;
	}
	
	virtual ~IObject() {}

	// 清理,注意不是销毁(析构)
	virtual void Clear();

	// ms
	virtual void Update(int iDeltaTime) {}

	void SetObjID( unsigned int dwID ) { m_dwID = dwID; }
	unsigned int GetObjID() { return m_dwID; }

	void SetObjType( unsigned short wType ) { m_wType = wType; }
	unsigned short GetObjType() { return m_wType; } 

	unsigned short GetMemBytes() { return m_wMemBytes; }
	void SetMemBytes( unsigned short wMemBytes ) { m_wMemBytes = wMemBytes; }

protected:
	unsigned int m_dwID;
	unsigned short m_wType;

private:
	unsigned short m_wMemBytes; // 对象占用内存数，统计
};


#endif

