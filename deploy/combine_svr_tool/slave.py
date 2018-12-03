#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
合服工具 -- 从服务器调用
"""


import subprocess
import time
import sys

sys.path.append("..")
from utilpy.com_util import *


def gen_add_table_suffix_sql(com):
    sql = "select CONCAT('rename table \`',table_name,'\` to \`', table_name,'_%s\`;') from information_schema.tables where table_schema='%s';" %\
     (com["GameServerID"], com["MysqlDBName"])
    return sql

def gen_del_table_suffix_sql(com):
    sql = "select CONCAT('rename table \`',table_name,'\` to \`', left(table_name, length(table_name) - %s ), '\`;') \
    from information_schema.tables where table_schema='%s';" % ( len(com["GameServerID"]) + 1, com["MysqlDBName"])
    return sql


def save_redis_aof(com):
    """
    保存redis信息 通过aof的方式保存
    """
    redis = "redis-cli -p %s BGREWRITEAOF" % com["RedisPort"]
    subprocess.call(redis, shell=True)





def main():
    """
    执行
    """
    print ("## start %s " % __file__)
    glo = load_json("../config/GenXml.json")
    com = glo["Global"]["Common"]
    com["MysqlSet"] = "latin1"
    #备份原始数据
    now = time.strftime("%Y%m%d.%H%M%S", time.localtime(time.time()))
    backup_name = "%s_%s_%s.bak" % (com["MysqlDBName"], com["GameServerID"], now)
    dump_database(com, backup_name)
    print("data backup to %s ok" % backup_name)
    time.sleep(10)

    #表名增加后缀
    rename = "rename.sql"
    ex_mysql_cmd(com, gen_add_table_suffix_sql(com), out_file=rename)
    ex_mysql_cmd(com, rename, "f")
    time.sleep(10)

    #dump增加后缀的数据库
    combine_name = "%s_%s.sql" % (com["MysqlDBName"], com["GameServerID"])
    dump_database(com, combine_name)
    time.sleep(10)

    #还原表名
    #rename = "rename.del.sql"
    #ex_mysql_cmd(com, gen_del_table_suffix_sql(com), out_file=rename)
    #ex_mysql_cmd(com, rename, "f")

    #保存redis
    save_redis_aof(com)

    print ("## end %s " % __file__)


if __name__ == "__main__":
    main()
