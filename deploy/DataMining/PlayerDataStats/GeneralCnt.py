#!/usr/bin/env python
# -*- coding: utf-8 -*-
__author__ = 'yingxi'
import sys

sys.path.append("..")

from MysqlHandler import *


def generalcntstats(date, DWPara, QueryDBPara):
    QueryDB = MysqlHandler(QueryDBPara)
    sql = "CREATE TABLE IF NOT EXISTS `general_opt_stats`(\
	`id` int(10) NOT NULL AUTO_INCREMENT,\
    `DateTime` varchar(32) DEFAULT NULL,\
    `OpType` int(11) DEFAULT NULL,\
    `AvgPlayerCnt` int(11) DEFAULT NULL,\
	PRIMARY KEY (`id`), INDEX `DateTime` (`DateTime`)\
    )ENGINE=InnoDB DEFAULT CHARSET=utf8;"
    QueryDB.execute(sql)

    DWHandler = MysqlHandler(DWPara)
    sql = "create view vPlayerLogin as \
    select COUNT(distinct PlayerID) as times from AccountLoginLog\
    where DATE(LoginTime)='%s' ;" % date

    DWHandler.execute(sql)

    sql = "select str_to_date(GeneralLog.DateTime, '%%Y-%%m-%%d'), GeneralLog.OpType, COUNT(GeneralLog.PlayerID)/vPlayerLogin.times as AvgPlayerCnt\
	    from vPlayerLogin, GeneralLog \
        where str_to_date(GeneralLog.DateTime, '%%Y-%%m-%%d') = '%s' \
        group by OpType;" % date

    DWHandler.execute(sql)
    for row in DWHandler.get_result():
        sql = "insert into general_opt_stats(DateTime, OpType, AvgPlayerCnt) values('%s', '%d', '%d')" % row
        QueryDB.execute(sql)

    sql = "drop view vPlayerLogin;"
    DWHandler.execute(sql)

    QueryDB.close()
    DWHandler.close()
