#!/bin/bash

help_info()
{
	echo "usage: $0 <mysql-ib_username> <mysql-ib_password> <database_name>";
}

if [ $# -ne 3 ]; then
	help_info;
	exit 1;
fi

mysql -u${1} -p${2} < ./init_log_database.sql
mysql -u${1} -p${2} -D${3} < ./init_log.sql
echo "finished"