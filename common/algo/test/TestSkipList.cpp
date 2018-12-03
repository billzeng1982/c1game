#include <stdio.h>
#include "skiplist.h"
   /* struct Student
    {
        int uin;
        int score;
        int age;
        char sex;
        int zipcode;
    };*/

    /*
        学生得分高的排前面，得分一样的，年龄小的排前面，注意skiplist是升序排列的
    */
    struct StudentSort
    {
        int uin;
        int score;
        int age;

        bool operator==(const StudentSort& a) const
        {
            return this->score==a.score && this->age==a.age;
        }

        bool operator < (const StudentSort& a ) const
        {
            if( this->score > a.score )
            {
                return true;
            }else if( this->score==a.score && this->age < a.age )
            {
                // 得分相同，年龄小的牛逼一些
                return true;
            }

            return false;
        }
        void ShowMe()
        {
            printf("uin=%d\tscore=%d\tage=%d\n", this->uin, this->score, this->age);
        }
    };

    struct ScoreCompare
    {
        int operator() ( const StudentSort*& pstLeft, const StudentSort*& pstRight ) const
        {
            if ( *pstLeft < *pstRight )
            {
                return 1;
            }
            else if ( *pstLeft == *pstRight )
            {
                return 0;
            }

            return -1;
        }

        int operator() ( const int& iLeft, const int& iRight ) const
        {
            return iRight - iLeft;
        }
    };


int main( int argc, char** argv )
{
#if 0
    if( argc != 2 )
    {
        printf("%s NodeNum \n", argv[0]);
        return 0;
    }

    int num = 1000;
    assert(num>0);
    srandom( time(NULL) );

    SkipList< StudentSort*, int, ScoreCompare> skiplist(12);
    StudentSort* students = new StudentSort[num];

    // construct skip list
    for( int i = 0; i < num; i++ )
    {
        students[i].uin = i+1;
        students[i].score = random() % 150;
        students[i].age = 15 + random() % 100;
        StudentSort sortKey;
        sortKey.age = students[i].age;
        sortKey.score = students[i].score;
        int rank = skiplist.Insert( &students[i], i+1 );
    }

    //skiplist.PrintMe();

   /* std::vector<CODETEST::Student*> v;
    skiplist.GetTopN(10, v);

    printf("After add, Top 10:\n");
    printf("Top 10:\n");
    for( unsigned int i=0; i < v.size(); i++ )
    {
        printf("uin<%d>, score<%d>, age<%d>, sex<%d>, zipcode<%d>\n", \
            v[i]->uin, v[i]->score,v[i]->age,v[i]->sex, v[i]->zipcode );
    }

   // test del
  /*  int delcnt = num / 2;
    for( int i = 0; i < delcnt; i++ )
    {
        CODETEST::StudentSortKey sortKey;
        sortKey.age = students[i].age;
        sortKey.score = students[i].score;
        skiplist.Delete(sortKey, &students[i]);
    }

    v.clear();
    skiplist.GetTopN(10, v);
    printf("After delete, Top 10:\n");
    for( unsigned int i=0; i < v.size(); i++ )
    {
        printf("uin<%d>, score<%d>, age<%d>, sex<%d>, zipcode<%d>\n", \
            v[i]->uin, v[i]->score,v[i]->age,v[i]->sex, v[i]->zipcode );
    }

    // test update
    int uptcnt = num / 2;
    for( int i = 0; i < uptcnt; i++ )
    {
        int j = random() % num;
        CODETEST::StudentSortKey oldKey;
        CODETEST::StudentSortKey newKey;

        oldKey.age = students[j].age;
        oldKey.score = students[j].score;

        newKey.age = 15 + random() % 10;
        newKey.score = random() % 150;

        students[j].age = newKey.age;
        students[j].score = newKey.score;

        printf( "update node: %d\n", students[j].uin );
        printf( "old key: score<%d>, age<%d>\n", oldKey.score, oldKey.age );
        printf( "new key: score<%d>, age<%d>\n", newKey.score, newKey.age );
        skiplist.Update(oldKey, newKey, &students[j]);
    }

    v.clear();
    skiplist.GetTopN(10, v);
    printf("After update, Top 10:\n");
    for( unsigned int i=0; i < v.size(); i++ )
    {
        printf("uin<%d>, score<%d>, age<%d>, sex<%d>, zipcode<%d>\n", \
            v[i]->uin, v[i]->score,v[i]->age,v[i]->sex, v[i]->zipcode );
    }

    v.clear();
    delete[] students;*/
#endif
    return 0;
}

