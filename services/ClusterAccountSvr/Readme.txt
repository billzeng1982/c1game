放到cluster一级，保存玩家帐号信息，用于外部三方功能查询

内部接受http json格式:

获取单个玩家信息:
{
	"SdkUserName" : "1234567abc",
	"ServerID"    :  2
}

测试curl命令:
curl -d "{\"SdkUserName\":\"zeng11\", \"ServerID\":6}" "http://172.16.1.20:8001/GetOneAccInfo" 

