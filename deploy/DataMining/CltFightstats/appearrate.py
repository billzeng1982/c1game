#!/usr/bin/python
# -*- coding: utf-8 -*-
__author__ = 'star'
import sys
sys.path.append("..")

from  MysqlHandler import *

def appearratestats(date,DWPara,QueryDBPara):
    QueryDB =  MysqlHandler(QueryDBPara)
    sql = "CREATE TABLE IF NOT EXISTS `Appear_Rate_Stats` (\
    `id` int(10) NOT NULL AUTO_INCREMENT,\
    `Date` datetime DEFAULT NULL,\
    `GeneralID` int(11) DEFAULT NULL,\
    `GeneralName` varchar(32) DEFAULT NULL,\
    `AppearTimes` int(11) DEFAULT NULL,\
    `AppearStats` float DEFAULT NULL,\
    PRIMARY KEY (`id`), INDEX `Date`(`Date`), INDEX `GeneralID` (`GeneralID`)\
    )ENGINE=InnoDB DEFAULT CHARSET=utf8;"
    QueryDB.execute(sql)

    DWHandler = MysqlHandler(DWPara)
    sql = "set @Total:=(select COUNT(DateTime) from CltFightStats where DATE(DateTime)='%s');" % date
    DWHandler.execute(sql)
    sql = "select DATE(DateTime) as Date,GeneralID, GeneralName, COUNT(GeneralID) as AppearTimes, (COUNT(GeneralID)/@Total)*3 as Percent \
    from CltFightStats \
    where DATE(DateTime)='%s' \
    group by GeneralID;" % date
    DWHandler.execute(sql)
    for row in DWHandler.get_result():
        sql = "insert into Appear_Rate_Stats(Date, GeneralID, GeneralName, AppearTimes, AppearStats) values('%s', '%d', '%s','%d', '%f')" % row
        QueryDB.execute(sql)
    QueryDB.close()
    DWHandler.close()



