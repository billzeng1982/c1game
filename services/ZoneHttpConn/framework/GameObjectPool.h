#pragma once

#ifndef GAME_OBJECT_POOL_H_
#define GAME_OBJECT_POOL_H_

#include "utils/singleton.h"
#include "mng/ObjectPool.h"
#include "module/HttpReqTimer.h"

enum GAMEOBJECT_TYPE
{
    GAMEOBJ_HTTP_REQ_TIMER = 1,
};

class GameObjectPool : public ObjectPool<GameObjectPool>, public TSingleton<GameObjectPool>
{
public:
    GameObjectPool()
    {
        REGISTER_OBJ_CREATOR(GAMEOBJ_HTTP_REQ_TIMER, GameObjectPool, HttpReqTimer);
    }

private:
    OBJ_CREATOR(HttpReqTimer);
};

#define RELEASE_GAMEOBJECT(p) do{ if(p) {GameObjectPool::Instance().Release(p); p=NULL;}}while(0)
#define GET_GAMEOBJECT(ClassType, EnumType) dynamic_cast<ClassType*>(GameObjectPool::Instance().Get(EnumType) )
#define FIND_GAMEOBJECT(ClassType, ObjID)	dynamic_cast<ClassType*>(GameObjectPool::Instance().Find(ObjID) )

#endif
