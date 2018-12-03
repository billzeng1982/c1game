#!/usr/bin/env python
# -*- coding: utf-8 -*-
__author__ = 'yingxi'
import sys

sys.path.append("..")

from MysqlHandler import *

def mskillcntstats(date, DWPara, QueryDBPara):
    QueryDB = MysqlHandler(QueryDBPara)
    sql = "CREATE TABLE IF NOT EXISTS `mskill_opt_stats`(\
	`id` int(10) NOT NULL AUTO_INCREMENT,\
    `DateTime` varchar(32) DEFAULT NULL,\
    `MSkillID` int(11) DEFAULT NULL,\
    `AvgPlayerCnt` int(11) DEFAULT NULL,\
	PRIMARY KEY (`id`), INDEX `DateTime` (`DateTime`)\
    )ENGINE=InnoDB DEFAULT CHARSET=utf8;"
    QueryDB.execute(sql)

    DWHandler = MysqlHandler(DWPara)

    sql = "set @Total = (select COUNT(distinct PlayerID) from AccountLoginLog where str_to_date(LoginTime, '%%Y-%%m-%%d') = '%s');" % date
    DWHandler.execute(sql)

    sql = "select str_to_date(MSKillLog.DateTime, '%%Y-%%m-%%d'), MSkillID, COUNT(MSKillLog.PlayerID)/@Total as AvgPlayerCnt\
	    from MSKillLog \
        where str_to_date(MSKillLog.DateTime, '%%Y-%%m-%%d') = '%s' \
        group by MSkillID;" % date

    DWHandler.execute(sql)
    for row in DWHandler.get_result():
        sql = "insert into general_opt_stats(DateTime, MSkillID, AvgPlayerCnt) values('%s', '%d', '%d')" % row
        QueryDB.execute(sql)

    QueryDB.close()
    DWHandler.close()
