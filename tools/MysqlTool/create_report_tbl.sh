#!/bin/bash

help_info()
{
	echo "usage: $0 create <year> <mysql_username> <mysql_password> <group_num>";
}
tbl_name=t_dir_report_""$2

if [ $# -ne 5 ]; then
	help_info;
	exit 1;
fi

if [ "$1" != "create" ] && [ "$1" != "add" ]; then
	help_info;
	exit 1;
fi

group_str=""
for((i=1;i<=$5;i++));do
    group_str+=", Group"$i"_OnlineNum int(4) not null default 0"
done;

sql_str=""
for((m=1;m<=12;m++));do
    sql_str=$sql_str"CREATE TABLE IF NOT EXISTS $tbl_name$m(Id int(4) not null primary key auto_increment, TimeStatic int(4) not null, Total_OnlineNum int(4) not null default 0$group_str);"
done;
echo "$sql_str"

mysql -u$3 -p$4 -Drgame -e"$sql_str" 
echo "ok"

