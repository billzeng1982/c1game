#!/usr/bin/env python
#coding:utf-8

from MysqlHandler import *
import sys
import json
import logging
sys.path.append('./')

db_para_log = {
    "host": "localhost",
    "username": "root",
    "password": "123456",
    "db": "DWLog",
    "port": 3306
}

def QueryLog(uin, stime, etime, type):
    result = []
    DBHandler = MysqlHandler(db_para_log)
    logging.basicConfig(level=logging.DEBUG,
            format='%(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s %(message)s',
            datefmt='%a, %d %b %Y %H:%M:%S',
            filename='myapp.log',
            filemode='w')
    logging.debug("----------------")
    logging.debug(uin)
    logging.debug(stime);
    logging.debug(etime);
    logging.debug(type);
    if type == 1:
        # 登录日志查询
        sql = "select PlayerID, RoleName, LoginTime from AccountLoginLog \
                where PlayerID = %s and DATE(LoginTime) between '%s' and '%s'" % (uin, stime, etime)
        DBHandler.execute(sql)
        raws = DBHandler.get_result()
        for raw in raws:
            str = "{'PlayerID': %s, 'RoleName': '%s', 'LoginTime': '%s'}\n" % raw
            result.append(str.encode('utf8'))
    elif type == 2:
        # 充值日志查询
        # YuanLog
        sql = "select PlayerID, RoleName, DateTime, ChgType, ChgValue, CurValue, Approach from YuanLog \
                where PlayerID = %s and DATE(DateTime) between '%s' and '%s'" % (uin, stime, etime)
        DBHandler.execute(sql)
        raws = DBHandler.get_result()
        for raw in raws:
            str = "{'PlayerID': %s, 'RoleName': '%s', 'DateTime': '%s', 'ChgType': %s, 'ChgValue': %s, 'CurValue':%s, 'Approach': %s}\n" % raw
            result.append(str.encode('utf8'))
    elif type == 3:
        # 金币日志查询
        # GoldLog
        sql = "select PlayerID, RoleName, DateTime, ChgType, ChgValue, CurValue, Approach from GoldLog \
                where PlayerID = %s and DATE(DateTime) between '%s' and '%s'" % (uin, stime, etime)
        DBHandler.execute(sql)
        raws = DBHandler.get_result()
        for raw in raws:
            str = "{'PlayerID': %s, 'RoleName': '%s', 'DateTime': '%s', 'ChgType': %s, 'ChgValue': %s, 'CurValue':%s, 'Approach': %s}\n" % raw
            result.append(str.encode('utf8'))
    elif type == 4:
        # 钻石日志查询
        # DiamandLog
        logging.debug("in type 4")
        sql = "select PlayerID, RoleName, DateTime, ChgType, ChgValue, CurValue, Approach from DiamandLog \
                where PlayerID = %s and DATE(DateTime) between '%s' and '%s'" % (uin, stime, etime)
        DBHandler.execute(sql)
        raws = DBHandler.get_result()
        for raw in raws:
            str = "{'PlayerID': %s, 'RoleName': '%s', 'DateTime': '%s', 'ChgType': %s, 'ChgValue': %s, 'CurValue':%s, 'Approach': %s}\n" % raw
            result.append(str.encode('utf8'))
    elif type == 5:
        # 道具日志查询
        # PropLog
        sql = "select PlayerID, RoleName, DateTime, PropId, PropType, ChgType, ChgValue, CurValue, Approach from PropLog \
                where PlayerID = %s and DATE(DateTime) between '%s' and '%s'" % (uin, stime, etime)
        DBHandler.execute(sql)
        raws = DBHandler.get_result()
        for raw in raws:
            str = "{'PlayerID': %s, 'RoleName': '%s', 'DateTime': '%s', 'PropId': %s, 'PropType': %s, 'ChgType': %s, 'ChgValue': %s, 'CurValue':%s, 'Approach': %s}\n" % raw
            result.append(str.encode('utf8'))
    elif type == 6:
        # 商场购买日志查询
        # ItemPurchaseLog
        logging.debug("in type 6")
        sql = "select PlayerID, RoleName, DateTime, ItemType, ItemId, PriceType, PriceValue, PurchaseApproach from ItemPurchaseLog \
        where PlayerID = %s and DATE(DateTime) between '%s' and '%s'" % (uin, stime, etime)
        DBHandler.execute(sql)
        raws = DBHandler.get_result()
        for raw in raws:
            str = "{'PlayerID': %s, 'RoleName': '%s', 'DateTime': '%s', 'ItemType': %s, 'ItemId': %s, 'PriceType': %s, 'PriceValue': %s, 'PurchaseApproach':%s}\n" % raw
            result.append(str.encode('utf8'))
    return result

if __name__ == '__main__':
    # ret = QueryLog('807677460481','2016-11-27','2016-11-30',1,0,0,0)
    # ret = QueryLog('872013103105',0,0,101,2,0,0)
    ret = QueryLog('5617453105153','2017-6-29','2017-6-30',1)
    # ret = QueryLog('807677460481',0,0,101,2,0,0)
    if ret:
        pass
    else:
        str = "{'default': 'mei cha dao'}"
        ret.append(str.encode('utf8'))
    logging.debug(ret)