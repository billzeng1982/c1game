#include "redis/RedisSyncHandler.h"
//#include "RedisSyncHandler.h"
#include "common_proto.h"
#include <string.h>
#include <stdio.h>
#include "strutil.h"

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
using namespace PKGMETA;
using namespace std;
void test(DT_FRIEND_PLAYER_INFO& ss)
{
	printf("heellll okokok\n");
	return;
}


int test_hset();
int test_hset_bindata();
int test_rpush();
int test_dynamic_new_char();

int main( int argc, char **argv )
{
	return  test_hset_bindata();
	getchar();
	getchar();
}
int test_hset_bindata()
{
	RedisSyncHandler oRedisHandler;
    oRedisHandler.Init("127.0.0.1", 6379 );
	if( !oRedisHandler.Connect() )
	{
		printf("Connect failed!\n");
		return 0;
	}
    /*
	char* kk[] = {"HMSET", "player", "a1", "value1", "a2", "vlaue2", "a3", "value3", "a4", "value4"};
	size_t kklen[20] ;
	int num = sizeof(kk)/sizeof(char*);

	for (int i=0; i<num; i++)
	{
		kklen[i] = strlen(kk[i]);
	}
    	redisReply* reply = oRedisHandler.SyncCommandArgv(num, (const char**)kk,  (const size_t*) kklen); //主要用来同步多个二进制数据
	*/
    string tmpStr("HMSET");
    oRedisHandler.PushArgv(tmpStr);
    oRedisHandler.PushArgv(tmpStr.assign("hello"));
    oRedisHandler.PushArgv(tmpStr.assign("a1"));
    oRedisHandler.PushArgv(tmpStr.assign("v1"));
    oRedisHandler.PushArgv(tmpStr.assign("a2"));
    oRedisHandler.PushArgv(tmpStr.assign("v2"));
    redisReply* reply = oRedisHandler.SyncCommandArgv(); //主要用来同步多个二进制数据
	if (reply->type != REDIS_REPLY_ARRAY)
	{
		return -2;
	}
	//取数据
	for (int i=0; i<reply->elements; i++)
	{
		printf("a%d   %s \n",i+1, reply->element[i]->str);
	}
	printf("all is ok \n");

	getchar();
	getchar();
	return 0;
}


int test_hset()
{
	RedisSyncHandler oRedisHandler;
    oRedisHandler.Init("127.0.0.1", 6379);
	if( !oRedisHandler.Connect() )
	{
		printf("Connect failed!\n");
		return 0;
	}
	//redisReply* reply = oRedisHandler.SyncCommand("HMSET player a1 a1value a2 a2value a3 a3value a4 a4value");
	redisReply* reply = oRedisHandler.SyncCommand("HSET player a1 %b","0010",4);
	if (!(reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str,"OK") == 0))
	{
		return -1;
	}
	reply = oRedisHandler.SyncCommand( "HMGET player a1 a2 a3 a4");
	if (reply->type != REDIS_REPLY_ARRAY)
	{
		return -2;
	}
	//取数据
	for (int i=0; i<reply->elements; i++)
	{
		printf("a%d   %s \n",i+1, reply->element[i]->str);
	}
	printf("all is ok \n");
	getchar();
	oRedisHandler.Close();
	return 0;
}


int test_set_player_info()
{
	RedisSyncHandler oRedisHandler;
    oRedisHandler.Init("127.0.0.1", 6379);
	if( !oRedisHandler.Connect() )
	{
		printf("Connect failed!\n");
		return 0;
	}
	uint64_t ulUseSize = 0;
	DT_FRIEND_PLAYER_INFO stPlayerInfo = {0};
	DT_FRIEND_PLAYER_BLOB stPlayerBlob = {0};

	DT_FRIEND_PLAYER_INFO* test_p = &stPlayerInfo;
	test(*test_p);
	stPlayerInfo.m_bIsOnline = 2 ;
	StrCpy(stPlayerInfo.m_szName, "TestName", MAX_NAME_LENGTH);
	stPlayerInfo.m_bMajestyLevel = 88;
	stPlayerInfo.m_ullLastOffTimestamp = 1198;
	stPlayerInfo.m_dwHeadIconId = 23;
	int iRet = stPlayerInfo.pack((char*)stPlayerBlob.m_szData, MAX_LEN_FRIEND_PLAYER_BLOB, &ulUseSize);
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		printf("pack m_stPlayerInfo failed, Ret=%d \n", iRet);
		return ERR_SYS;
	}
	stPlayerBlob.m_iLen = (int)ulUseSize;
	redisReply* reply = oRedisHandler.SyncCommand("SET %lu %b", 210, stPlayerBlob.m_szData ,(size_t)stPlayerBlob.m_iLen);
	if (!(reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str,"OK") == 0))
	{
		return ERR_SYS;
	}

	reply = oRedisHandler.SyncCommand( "GET %lu", 222);
	if (reply->type != REDIS_REPLY_STRING)
	{
		return ERR_SYS;
	}
	stPlayerBlob.m_iLen = reply->len;

	bzero(&stPlayerInfo, sizeof(DT_FRIEND_PLAYER_INFO));

	iRet = stPlayerInfo.unpack((char*)reply->str, reply->len);
	if (TdrError::TDR_NO_ERROR != iRet)
	{
		printf("unpack m_stPlayerBlob failed, Ret=%d \n", iRet);
		return ERR_SYS;
	}

	char *pBuf = new char[1024]; size_t t=0;
	stPlayerInfo.visualize(pBuf, 1024, &t, 0, '#');
	printf("GetResult=%s \n", pBuf);
	delete[] pBuf;

	oRedisHandler.Close();
	getchar();
	return 0;
}

int test_rpush()
{
	redisContext *c;

	c = redisConnect("localhost",6379);
	if (c->err) {
		cerr << "Connection error: " << c->errstr << endl;
		return -1;
	}
	const char *param[5] = { "AAAAA", "B", "CCCCCC", "DDD", "EEEEEEEEE" };
	vector<string> v( param, param+5 );

	try
	{
		vector<const char *> argv( v.size() + 2 );
		vector<size_t> argvlen( v.size() + 2 );
		int j = 0;

		static char rpushcmd[] = "RPUSH";
		argv[j] = rpushcmd;
		argvlen[j] = sizeof(rpushcmd)-1;
		++j;

		argv[j] = string("mykey").c_str();
		argvlen[j] = v.size();
		++j;

		for ( vector<string>::const_iterator i = v.begin(); i != v.end(); ++i, ++j )
			argv[j] = i->c_str(), argvlen[j] = i->size();

		void *r = redisCommandArgv(c, argv.size(), &(argv[0]), &(argvlen[0]) );
		if ( !r )
			throw runtime_error( "Redis error" );
		freeReplyObject( r );
	}
	catch ( runtime_error & )
	{
		cerr << "Error" << endl;
	}

	redisFree(c);
	return 0;
}

int test_dynamic_new_char()
{
	char ** ppArgv = NULL;
	int iCnt = 3;
#if 1
	ppArgv = new char*[iCnt];
	for (int i=0; i < iCnt; i++)
	{
		ppArgv[i] = new char[10];
		if (!ppArgv[i])
		{
			printf("init ppArgv faild! <%d> ", i);
			return -1;
		}
		sprintf(ppArgv[i], "hello_%d", i);
	}
#endif	
// 	ppArgv = new
// 
// 	for (int i=0; i<iCnt; i++)
// 	{
// 		printf("%d is <%s> \n", i, ppArgv[i]);
// 	}
	getchar();
	getchar();
	return 1;
}




