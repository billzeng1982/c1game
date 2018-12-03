#pragma once

#include "ObjectUpdator.h"
#include "singleton.h"
#include <map>

class ObjectUpdatorMgr : public TSingleton<ObjectUpdatorMgr>
{
	typedef std::map< unsigned int /*obj type*/, ObjectUpdator* > ObjectUpdatorMap_t;

public:
	ObjectUpdatorMgr(){}
	~ObjectUpdatorMgr(){}

	ObjectUpdator* RegisterObjectUpdator( unsigned short wObjType );
	ObjectUpdator* RegisterObjectUpdator( unsigned short wObjType, int iIdleUptCount, int iBusyUptCount, int iUptFreq );

	// ��object��ӽ�update����
	void Schedule( IObject* pObj );
	
	// ����objectʱ����
	void Unschedule( IObject* pObj );

	void Update( bool bIdle );
	ObjectUpdator* GetObjUpdatorPtr(unsigned int dwObjType);
	
private:
	ObjectUpdatorMap_t m_ObjectUpdatorMap;
};

#define SCHEDULE_OBJECT( pObj  ) do { ObjectUpdatorMgr::Instance().Schedule(pObj); } while(0)
#define UNSCHEDULE_OBJECT( pObj ) do { ObjectUpdatorMgr::Instance().Unschedule(pObj); } while(0)

