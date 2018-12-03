#!/bin/bash

help_info()
{
	echo "usage: $0 create <mysql_username> <mysql_password> <group_num>";
}

if [ $# -ne 4 ]; then
	help_info;
	exit 1;
fi

if [ "$1" != "create" ] && [ "$1" != "add" ]; then
	help_info;
	exit 1;
fi

sql_str=""
for((i=1;i<=$4;i++));do
    sql_str=${sql_str}"DROP TABLE IF EXISTS tbl_role_$i;CREATE TABLE tbl_role_$i (id int(10) NOT NULL AUTO_INCREMENT COMMENT 'record ID',Uin bigint(20) unsigned NOT NULL,RoleName varchar(32) DEFAULT NULL,LastLoginTime int(10) unsigned DEFAULT NULL,Level tinyint(3) unsigned DEFAULT NULL,RoleInfoBlob mediumblob,ItemInfoBlob mediumblob,TaskInfoBlob mediumblob,GrowthItemInfoBlob mediumblob,MiscInfoBlob mediumblob,ShareDataBlob mediumblob,PRIMARY KEY (id),UNIQUE KEY Uin (Uin),UNIQUE KEY RoleName (RoleName)) ENGINE=InnoDB DEFAULT CHARSET=utf8;"
done;
echo "$sql_str"

mysql -u$2 -p$3 -Drgame -e"$sql_str"
echo "ok"



