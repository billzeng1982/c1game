# /bin/bash
# _author_ vito
PARA_NUM=$#
if [ $PARA_NUM != 4 ]; then
    echo "usage1: $0 database user name";
    echo "usage2: $0 database user password"
    echo "usage3: $0 database name"
    echo "usage4: $0 database port"
    exit 1;
fi
DB_USER=$1
DB_PASS=$2
DB_NAME=$3
DB_PORT=$4
BIN_DIR="/usr/bin"
BACK_DIR="/data/backdata"
DATE="mysql-`date +'%Y%m%d-%H:%M:%S'`"
LogFile="$BACK_DIR"/dbbakup.log #日志记录保存的目录
BackNewFile=$DATE.sql

$BIN_DIR/mysqldump --opt --force -u$DB_USER --single-transaction  -p$DB_PASS -P$DB_PORT $DB_NAME > $BACK_DIR/$DATE.sql

echo -----------------------"$(date +"%y-%m-%d %H:%M:%S")"----------------------- >> $LogFile 

echo  createFile:"$BackNewFile" >> $LogFile

#-ctime表示创建时间，这里表示删除创建时间为多少天之前的文件，也就是结果只保留多少天的数据
find "/data/backdata/" -ctime +7 -type f -name "*.sql" -print > deleted.txt

echo -e "delete files:\n" >> $LogFile 

#循环删除匹配到的文件
cat deleted.txt | while read LINE
do
  rm -rf $LINE
  echo $LINE>> $LogFile
done

echo "---------------------------------------------------------------" >> $LogFile
