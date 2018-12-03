#!/usr/bin/python
# -*- coding: utf-8 -*-
__author__ = 'star'
import sys
sys.path.append("..")

from  MysqlHandler import *

def winratestats(date,DWPara,QueryDBPara):
    QueryDB =  MysqlHandler(QueryDBPara)
    sql = "CREATE TABLE IF NOT EXISTS `Win_Rate_Stats`(\
    `id` int(10) NOT NULL AUTO_INCREMENT,\
    `Date` date DEFAULT NULL,\
    `GeneralID` int(11) DEFAULT NULL,\
    `GeneralName` varchar(32) DEFAULT NULL,\
    `AppearTimes` int(11) DEFAULT NULL,\
    `WinRate` float DEFAULT NULL,\
    PRIMARY KEY (`id`), INDEX `Date` (`Date`)\
    )ENGINE=InnoDB DEFAULT CHARSET=utf8;"
    QueryDB.execute(sql)

    DWHandler = MysqlHandler(DWPara)
    sql = "create view vAppearTimes as \
    SELECT GeneralID, COUNT(GeneralID) as Times \
    from CltFightStats \
    where DATE(DateTime)='%s' and MasterSkillLastTime=0 \
    group by CltFightStats.GeneralID;" % date
    DWHandler.execute(sql)

    sql = "select DATE(C.DateTime) as Date, C.GeneralID, C.GeneralName, v.Times, (COUNT(C.GeneralID)/v.Times) as WinRate \
    from CltFightStats C, vAppearTimes v \
    where DATE(C.DateTime)='%s' and C.Winner=C.RoleName and C.GeneralID = v.GeneralID \
    group by C.GeneralID;" % date
    DWHandler.execute(sql)
    for row in DWHandler.get_result():
        sql = "insert into Win_Rate_Stats(Date, GeneralID, GeneralName, AppearTimes, WinRate) values('%s', '%d', '%s','%d', '%f')" % row
        QueryDB.execute(sql)

    sql = "drop view vAppearTimes;"
    DWHandler.execute(sql)

    QueryDB.close()
    DWHandler.close()



