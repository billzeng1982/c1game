#!/usr/bin/python
# -*- coding: utf-8 -*-
__author__ = 'star'
import sys
sys.path.append("..")

from  MysqlHandler import *

def avgdeathstats(date,DWPara,QueryDBPara):
    QueryDB =  MysqlHandler(QueryDBPara)
    sql = "CREATE TABLE IF NOT EXISTS `Avg_Death_Stats`(\
    `id` int(10) NOT NULL AUTO_INCREMENT,\
    `Date` date DEFAULT NULL,\
    `GeneralID` int(11) DEFAULT NULL,\
    `GeneralName` varchar(32) DEFAULT NULL,\
    `DeathTimes` int(11) DEFAULT NULL,\
    `AppearTimes` int(11) DEFAULT NULL,\
    `AvgDeath` float DEFAULT NULL,\
    PRIMARY KEY (`id`), INDEX `Date`(`Date`)\
    )ENGINE=InnoDB DEFAULT CHARSET=utf8;"
    QueryDB.execute(sql)

    DWHandler = MysqlHandler(DWPara)
    sql = "select DATE(C.DateTime) as Date, C.GeneralID, C.GeneralName, SUM(C.DeadWithdrawTimes) as DeadTimes, COUNT(C.GeneralID) AppearTimes, (SUM(C.DeadWithdrawTimes)/COUNT(C.GeneralID)) as AvgDead \
    from CltFightStats C \
    where DATE(C.DateTime)='%s' and C.MasterSkillLastTime=0 \
    group by C.GeneralID;" % date
    DWHandler.execute(sql)

    for row in DWHandler.get_result():
        sql = "insert into Avg_Death_Stats(Date, GeneralID, GeneralName,DeathTimes, AppearTimes, AvgDeath) values('%s', '%d', '%s','%d','%d', '%f')" % row
        QueryDB.execute(sql)

    QueryDB.close()
    DWHandler.close()

__author__ = 'Administrator'
