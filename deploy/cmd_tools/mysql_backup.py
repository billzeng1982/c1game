#!/usr/bin/env python
#encoding=utf-8

__author_ = "vito"

import json
import os
import sys
import signal
import time


def main():
    cfg_path = os.path.join(os.path.dirname(__file__), "../config/GenXml.json")
    f = open(cfg_path, "r")
    data = json.load(f)
    database_user = data["Global"]["Common"]["MysqlUser"]
    database_port = data["Global"]["Common"]["MysqlPort"]
    database_pwd = data["Global"]["Common"]["MysqlPassword"]
    database_name = data["Global"]["Common"]["MysqlDBName"]
    flag = True
    while 1:
        if (time.localtime()[3] == 4) and flag:
            os.system('./mysqlbackup.sh ' + database_user + ' ' + database_pwd + ' ' + database_name + ' ' + database_port)
            flag = False
        else:
            time.sleep(30)
        if (time.localtime()[3] != 4):
            flag = True


if __name__=="__main__":
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
    main()
