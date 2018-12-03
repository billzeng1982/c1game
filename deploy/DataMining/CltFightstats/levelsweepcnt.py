#!/usr/bin/python
# -*- coding: utf-8 -*-
__author__ = 'star'
import sys
sys.path.append("..")

from  MysqlHandler import *

def levelsweepcntstats(date,DWPara,QueryDBPara):
    QueryDB =  MysqlHandler(QueryDBPara)
    sql = "CREATE TABLE IF NOT EXISTS `LevelSweepCnt` (\
    `DateTime` VARCHAR(32)   ,\
    `LevelID` INT UNSIGNED   ,\
    `SweepCnt` INT UNSIGNED   ,\
    INDEX `LevelID_index` (`LevelID`),\
    INDEX `DateTime_index` (`DateTime`))\
    ENGINE=InnoDB DEFAULT CHARSET=utf8;"
    QueryDB.execute(sql)

    DWHandler = MysqlHandler(DWPara)
    sql = "select str_to_date(DateTime, '%%Y-%%m-%%d'), LevelID , SUM(SweepCnt) as count from LevelSweepLog \
    where str_to_date(DateTime, '%%Y-%%m-%%d') = '%s' \
    group by LevelID;" % date
    DWHandler.execute(sql)
    for row in DWHandler.get_result():
        sql = "insert into LevelSweepCnt(DateTime, LevelID, SweepCnt) values('%s', '%d', '%d')" % row
        QueryDB.execute(sql)
    QueryDB.close()
    DWHandler.close()
