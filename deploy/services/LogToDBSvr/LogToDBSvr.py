#!/usr/bin/env python  
# -*- coding:utf-8 -*-
import re
import os
import sys
import datetime
import time
import xml.dom.minidom
import signal
import traceback
import logging
import codecs
from logging.handlers import TimedRotatingFileHandler
from MysqlHandler import *

#调试开关
g_DEBUG = 0
g_bExit = True
ISOTIMEFORMAT = '%Y-%m-%d %X'

db_para_log = {
    "host": "localhost",
    "username": "root",
    "password": "123456",
    "db": "DWLog",
    "port": 3306
}

class MyLog(object):

    def __init__(self):
        #初始化errlog参数
        self.__err_log = logging.getLogger("err_log")
        err_log_handler = TimedRotatingFileHandler(filename="../../log/LogToDBSvr_err.log", when="D", interval=1, backupCount=10)
        err_formatter = logging.Formatter('%(asctime)s %(filename)s[line:%(lineno)d] %(message)s')
        err_log_handler.setFormatter(err_formatter)
        self.__err_log.addHandler(err_log_handler)
        #初始化runlog参数
        self.__run_log = logging.getLogger("run_log")
        run_log_handler = TimedRotatingFileHandler(filename="../../log/LogToDBSvr_run.log", when="D", interval=1, backupCount=10)
        run_formatter = logging.Formatter('%(asctime)s %(filename)s[line:%(lineno)d] %(message)s')
        run_log_handler.setFormatter(run_formatter)
        self.__run_log.addHandler(run_log_handler)

    @classmethod
    def instance(cls):
        if not hasattr(cls, "_instance"):
            cls._instance = cls()
        return cls._instance

    def logerr(self, log_content):
        self.__err_log.error(log_content)

    def logrun(self, log_content):
        self.__run_log.error(log_content)


def log_run(log):
    MyLog.instance().logrun(log)


def log_err(log):
    MyLog.instance().logerr(log)


class LogToDB(object):

    def __init__(self):
        self.__DBConfig = {}
        self.__DstList = {}
        self.__DstPath = None
        self.__SrcPath = None
        self.__StartTime = None
        self.__flag = True

    def __findkey(self, line):
        return re.match(r'(.*?)\|', line).groups()[0]

    def __filterline(self, line):
        pos = line.index('|')
        return line[pos+1:]

    def __classifylog(self, date):
        path = self.__SrcPath + '/' + date
        if os.path.exists(path):
            files = os.listdir(path)
            for src_file in files:
                self.__classifyonefile(path + '/' + src_file, date)
        self.__closealldstfile()

    def __classifyonefile(self, src_file, date):
        src_fp = open(src_file, 'r')
        while 1:
            line = src_fp.readline()
            if not line:
                break
            key = self.__findkey(line)
            if key not in self.__DstList.keys():
                self.__createdstfile(key, date)
            result = self.__filterline(line)
            self.__DstList[key]['dst_file_fp'].write(result)
        src_fp.close()

    def __createdstfile(self, table, date):
            dst_file = self.__DstPath + '/' + table + '_' + date
            dst_node = {}
            dst_node['dst_file_fp'] = open(dst_file, 'w')
            dst_node['dst_file'] = dst_file
            self.__DstList[table] = dst_node

    def __closealldstfile(self):
        for key in self.__DstList.keys():
            self.__DstList[key]['dst_file_fp'].flush()
            self.__DstList[key]['dst_file_fp'].close()

    def __processlogfile(self, filename, tablename):
        src_fp = codecs.open(filename,'r')
        DBHandler = MysqlHandler(db_para_log)
        while 1:
            line = src_fp.readline()
            if not line:
                break
            sql = "insert into %s values ('%s');" %(tablename, line.replace("|","','"))
            DBHandler.execute(sql)
        src_fp.close()

    def __importlogtodb(self):
        for key in self.__DstList.keys():
            self.__processlogfile(self.__DstList[key]['dst_file'], key)

    def getconfig(self):
        dom = xml.dom.minidom.parse('../../config/LogToDB.xml')
        root = dom.documentElement
        DBConfig = root.getElementsByTagName('DBConfig')[0]
        self.__DBConfig['username'] = DBConfig.getAttribute('username')
        self.__DBConfig['password'] = DBConfig.getAttribute('password')
        self.__DBConfig['db'] = DBConfig.getAttribute('db')
        self.__SrcPath = root.getElementsByTagName('SrcPath')[0].getAttribute('path')
        self.__DstPath = root.getElementsByTagName('DstPath')[0].getAttribute('path')
        if not os.path.exists(self.__DstPath):
            os.makedirs(self.__DstPath)
        self.__StartTime = int(root.getElementsByTagName('StartTime')[0].getAttribute('hour'))

    def update(self):
        if (time.localtime()[3] == self.__StartTime) and self.__flag:
            os.chdir(os.path.split(os.path.realpath(__file__))[0])
            self.convertlogtodb()
            self.__flag = False
        else:
            time.sleep(30)

        if time.localtime()[3] != self.__StartTime:
            self.__flag = True

    def convertlogtodb(self):
        yesterday =datetime.date.today() - datetime.timedelta(days=1)
        date = yesterday.strftime('%Y-%m-%d')
        self.__DstList.clear()
        self.__classifylog(date)
        self.__importlogtodb()


def onsignal_term(a, b):
    global g_bExit
    g_bExit = False

def main():
    signal.signal(signal.SIGTERM, onsignal_term)
    logtodbtask = LogToDB()
    logtodbtask.getconfig()
    #正常逻辑
    while g_bExit:
        try:
            logtodbtask.update()
        except:
            s = traceback.format_exc()
            log_err(s)
            continue


if __name__=="__main__":
    #启动程序
    #调试时直接启用,不用fork
    if 0 == g_DEBUG:
        try:
            pid = os.fork()
            if pid > 0:
                sys.exit(0)
        except OSError, e:
            print >>sys.stderr, "fork #1 failed: %d (%s)" % (e.errno, e.strerror)
            sys.exit(1)
        os.setsid()
        os.umask(0)
        try:
            pid = os.fork()
            if pid > 0:
                sys.exit(0)
        except OSError, e:
            print >>sys.stderr, "fork #2 failed: %d (%s)" % (e.errno, e.strerror)
            sys.exit(1)
    else:
        print("Debug Now----->")
    main()
