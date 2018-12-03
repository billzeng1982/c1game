#include "../CoDataFrame.h"
#include "strutil.h"
#include <map>
using namespace std;

namespace COTEST
{
    static uint64_t g_ulToken = 0;

    struct Student
    {
        int  id;
        char name[40];
        int  age;

        Student() { id=0; name[0]='\0'; age = 0; }
    };

    class AsyncGetStudent : public CoGetDataAction
    {
    public:
        AsyncGetStudent() { m_id = 0; }
        virtual ~AsyncGetStudent(){}

        virtual bool Execute( )
        {
            g_ulToken = this->GetToken();
            LOGRUN("Async get student data, token: <%lu>", g_ulToken);
            return true;
        }

        int m_id;
    };

    class StudentDataFrame : public CoDataFrame
    {
        typedef map<int, Student*> Id2StudentMap_t;
    public:
        struct Argument
        {
            StudentDataFrame* pFrame;
            int id;
        };
        
    public:
        StudentDataFrame() { m_oId2StudentMap.clear(); }
        virtual ~StudentDataFrame(){}

        virtual bool IsInMem( void* pResult )
        {
            Student* student = (Student*)pResult;
            Id2StudentMap_t::iterator it = m_oId2StudentMap.find( student->id );
            return it != m_oId2StudentMap.end();
        }

        virtual bool SaveInMem( void* pResult )
        {
            Student* student = (Student*)pResult;
            Student* pNew = m_oStudentPool.NewData( );
            if( !pNew ) return false;  // 正式情况下应该执行 LRU，换出老数据
            pNew->id = student->id;
            pNew->age = student->age;
            StrCpy( pNew->name, student->name, 40 );
            m_oId2StudentMap.insert( Id2StudentMap_t::value_type( student->id, pNew ) );
            return true;
        }

        bool Init( int iMaxCacheNum, int iMaxTransNum )
        {
            if( !this->BaseInit(iMaxTransNum) )
            {
                LOGERR("BaseInit failed!");
                return false;
            }
            if( m_oStudentPool.CreatePool(iMaxCacheNum) < 0)
                return false;
            return true;
        }

        // 从内存缓存中获取数据
	   virtual void* _GetDataInMem( void* key )
        {
            int* pi = (int*)key;
            Id2StudentMap_t::iterator it = m_oId2StudentMap.find( *pi );
            if( it != m_oId2StudentMap.end() )
                return it->second;
            return NULL;
        }

        virtual CoGetDataAction* _CreateGetDataAction( void* key )
        {
            AsyncGetStudent* po = new AsyncGetStudent();
            po->m_id = *(int*)key;
            return po;
        }

        virtual void _ReleaseGetDataAction( CoGetDataAction* poAction )
        {
            delete poAction;
        }

    private:
          Id2StudentMap_t m_oId2StudentMap;
          CMemPool<Student> m_oStudentPool;
    };

    static void CoFunc( void* arg )
    {
        StudentDataFrame::Argument* pArg = (StudentDataFrame::Argument*)arg;
        StudentDataFrame* pFrame = pArg->pFrame;
        int id = pArg->id;

        LOGRUN("Get student data ... start");

        Student* student = (Student*)pFrame->GetData( &id );

        LOGRUN("Get student data ... end");

        // logic to handle data
        if( !student )
        {
            LOGERR("Get data failed!");
            return;
        }

        LOGRUN("student id<%d>, name<%s>, age<%d>", student->id, student->name, student->age);
    }
};

int main()
{
    CoroutineEnv oCoEnv;
    int iRet = 0;
    oCoEnv.Init(16);

    COTEST::StudentDataFrame oStudentDataFrame;
    if( !oStudentDataFrame.Init( 20, 16 ) )
    {
        LOGERR("init failed!");
        return -1;
    }
    oStudentDataFrame.SetCoroutineEnv( &oCoEnv );

    CGameTime::Instance().UpdateTime();

    COTEST::StudentDataFrame::Argument stCoArg;
    stCoArg.pFrame = &oStudentDataFrame;
    stCoArg.id = 1234;
    
    iRet = oCoEnv.StartCoroutine( COTEST::CoFunc, &stCoArg );
    if( iRet < 0 )
    {
        assert( false );
    }

    LOGRUN("[main] logic");

    int iCount = 0;
    while( iCount < 1000 ) // 把数字改大可测试超时
    {
        CGameTime::Instance().UpdateTime();
        iCount++;
        MsSleep(1);
        oStudentDataFrame.Update();
    }

    LOGRUN("[main] Got student data ...");
    COTEST::Student student;
    student.id = 1234;
    StrCpy( student.name, "billzeng", 40 );
    student.age = 22;
    oStudentDataFrame.AsyncGetDataDone( COTEST::g_ulToken, 0, &student );

    LOGRUN("[main] example end ...");
    
    return 0;
}

