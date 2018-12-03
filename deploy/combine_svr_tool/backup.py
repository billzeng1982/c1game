#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
合服工具 -- 首次备份数据
"""


import subprocess
import time
import sys
sys.path.append("..")
from utilpy.com_util import *

def main():

    print ("## start %s " % __file__)

    glo = load_json("../config/GenXml.json")
    com = glo["Global"]["Common"]
    com["MysqlSet"] = "latin1"
    
    #备份原始数据
    now = time.strftime("%Y%m%d.%H%M%S", time.localtime(time.time()))
    backup_name = "../ori_backup/ori_%s_%s_%s.bak" % (com["MysqlDBName"], com["GameServerID"], now)
    dump_database(com, backup_name)
    print ("## end %s " % __file__)

if __name__ == "__main__":
    main()

