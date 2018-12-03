#include <stdio.h>
#include "GameEvent.h"

class A
{
public:
    void test(GameEvent* poEvent)
    {
        printf("A test event, id=%d\n", poEvent->m_iEventID);
    }

    void bind()
    {
        f = RayE::Bind(&A::test, this);
        GameEventDispatcher::Instance().Register(1, 1, f);
    }

    Function1<void, GameEvent*> f;
};


class B
{
public:
    void test(GameEvent* poEvent)
    {
        printf("B test event, id=%d\n", poEvent->m_iEventID);
    }

    void bind()
    {
        f = RayE::Bind(&B::test, this);
        GameEventDispatcher::Instance().Register(1, 2, f);
    }

    Function1<void, GameEvent*> f;
};


int main()
{
    A a;
    B b;
    a.bind();
    b.bind();
    GameEvent oEvent(1);
    GameEventDispatcher::Instance().Dispatch(&oEvent);
}
