#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
一些常用小方法
"""

import json
import socket
import struct
import subprocess


g_ISPRINT = True

def load_json(file_name):
    """
    加载json配置文件,并返回字典
    """
    f = file(file_name)
    cfg = json.load(f)
    f.close()
    return cfg


def get_local_ipv4():
    """
    获取本地IPv4地址,该IP 配置在/etc/hosts,返回与本机的主机名相同的条目
    """
    return socket.gethostbyname(socket.gethostname())

def is_private_net(target_ip):
    """
    是否为内网
    """
    begin = struct.unpack("!I", socket.inet_aton("172.16.0.0"))[0]
    end = struct.unpack("!I", socket.inet_aton("172.16.255.255"))[0]
    target = struct.unpack("!I", socket.inet_aton(target_ip))[0]
    if begin < target and target < end:
        return True
    return False


def dump_database(com, file_name):
    """
    dump数据
    """
    mysqldump = "mysqldump -u'%s' -h'%s' -p'%s' -P'%s'  --default-character-set=%s --databases %s > ./data/%s" \
    %   (com["MysqlUser"], com["MysqlIp"], com["MysqlPassword"], com["MysqlPort"] ,com["MysqlSet"], com["MysqlDBName"], file_name)
    my_print(mysqldump)
    subprocess.call(mysqldump, shell=True)


def ex_mysql_cmd(com, sql, sql_type="s", out_file="/dev/null"):
    """
    通过mysql命令行执行sql语句
    """
    if sql_type is "s":
        mysql = "mysql -N -u'%s' -h'%s' -p'%s' -P'%s' -D'%s' --default-character-set=%s -e \"%s\" > %s " % (com["MysqlUser"], com["MysqlIp"], \
        com["MysqlPassword"],  com["MysqlPort"], com["MysqlDBName"], com["MysqlSet"], sql, out_file)
    else:
        #执行文件
        mysql = "mysql -N -u'%s' -h'%s' -p'%s' -P'%s' -D'%s' --default-character-set=%s < %s > %s " % (com["MysqlUser"], com["MysqlIp"], \
        com["MysqlPassword"],  com["MysqlPort"], com["MysqlDBName"], com["MysqlSet"], sql, out_file)
    my_print(mysql)
    subprocess.call(mysql, shell=True)


def my_print(string):
    """
    打印
    """
    if g_ISPRINT:
        print(string)


