#include "mysql/MysqlHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    CMysqlHandler oMysqlHandler;

    if (!oMysqlHandler.ConnectDB( "127.0.0.1",
                                 3306,
                                 "c1game",
                                 "root",
                                 "c1game123") )
    {
        printf("Connect mysql failed!\n");
        return -1;
    }

    // do reconnect
    for( int i = 0; i < 10; i++ )
    {
        oMysqlHandler.ReconnectDB();
        printf("reconnect %d times\n", i+1);
        sleep(2);
    }

    return 0;
}


