#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
合服工具 -- 整理数据
"""


import subprocess
import time
import sys

sys.path.append("..")
from utilpy.com_util import *



def main(svr_id):
    print ("## start %s " % __file__)
    glo = load_json("../config/GenXml.json")
    com = glo["Global"]["Common"]
    com["MysqlSet"] = "latin1"
    becombined_id = svr_id

    #删除PubSeq=0的私人邮件, 一般是别的服务器的玩家的数据, 原因应该是Gm发邮件选错服务器造成的
    sql ="DELETE FROM tbl_mail_pri WHERE PubSeq = 0; DELETE FROM tbl_mail_pri_%s WHERE PubSeq = 0;" % becombined_id
    ex_mysql_cmd(com, sql)
    time.sleep(10)

    #删除名字为空的好友记录, 是无效的
    sql = "DELETE FROM tbl_friend WHERE \`Name\` = '';DELETE FROM tbl_friend_%s WHERE \`Name\` = '';" % becombined_id
    ex_mysql_cmd(com, sql)
    time.sleep(10)
    print ("## end %s " % __file__)


if __name__ == "__main__":
    main(3)


