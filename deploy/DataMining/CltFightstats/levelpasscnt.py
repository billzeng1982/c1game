#!/usr/bin/python
# -*- coding: utf-8 -*-
__author__ = 'star'
import sys
sys.path.append("..")

from  MysqlHandler import *

def levelpasscntstats(date,DWPara,QueryDBPara):
    QueryDB =  MysqlHandler(QueryDBPara)
    sql = "CREATE TABLE IF NOT EXISTS `LevelPassCnt` (\
    `DateTime` VARCHAR(32)   ,\
    `LevelID` INT UNSIGNED   ,\
    `PassCnt` INT UNSIGNED   ,\
    INDEX `LevelID_index` (`LevelID`),\
    INDEX `DateTime_index` (`DateTime`))\
    ENGINE=InnoDB DEFAULT CHARSET=utf8;"
    QueryDB.execute(sql)

    DWHandler = MysqlHandler(DWPara)
    sql = "select str_to_date(StartTime, '%%Y-%%m-%%d'), LevelID, COUNT(LevelID) as cout from LevelPassLog \
    where str_to_date(StartTime, '%%Y-%%m-%%d') = '%s' \
    group by LevelID;" % date
    DWHandler.execute(sql)
    for row in DWHandler.get_result():
        sql = "insert into LevelPassCnt(DateTime, LevelID, PassCnt) values('%s', '%d', '%d')" % row
        QueryDB.execute(sql)
    QueryDB.close()
    DWHandler.close()