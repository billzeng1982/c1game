#!/usr/bin/env python
# -*- coding: utf-8 -*-
__author__ = 'yingxi'
import sys

sys.path.append("..")

from MysqlHandler import *


def awardratestats(date, DWPara, QueryDBPara):
    QueryDB = MysqlHandler(QueryDBPara)
    sql = "CREATE TABLE IF NOT EXISTS `award_rate_stats`(\
    `id` int(10) NOT NULL AUTO_INCREMENT,\
    `DateTime` varchar(32) DEFAULT NULL,\
    `AwardType` int(11) DEFAULT NULL,\
    `DrawRate` float DEFAULT NULL,\
    PRIMARY KEY (`id`), INDEX `DateTime` (`DateTime`)\
    )ENGINE=InnoDB DEFAULT CHARSET=utf8;"
    QueryDB.execute(sql)

    DWHandler = MysqlHandler(DWPara)

    sql = "set @Total = (select COUNT(distinct PlayerID) from AccountLoginLog where DATE(LoginTime)='%s');" % date
    DWHandler.execute(sql)

    sql = "select str_to_date(AwardLog.DateTime, '%%Y-%%m-%%d'), AwardLog.AwardType , COUNT(AwardLog.PlayerID)/@Total as rate from AwardLog \
    where str_to_date(AwardLog.DateTime, '%%Y-%%m-%%d') = '%s' \
    group by AwardType;" % date
    DWHandler.execute(sql)

    for row in DWHandler.get_result():
        sql = "insert into award_rate_stats(DateTime, AwardType, DrawRate) values('%s', '%d', '%f')" % row
        QueryDB.execute(sql)

    QueryDB.close()
    DWHandler.close()
