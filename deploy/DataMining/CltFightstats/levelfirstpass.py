#!/usr/bin/python
# -*- coding: utf-8 -*-
__author__ = 'star'
import sys
sys.path.append("..")

from  MysqlHandler import *

def levelfirstpassstats(date,DWPara,QueryDBPara):
    QueryDB =  MysqlHandler(QueryDBPara)
    sql = "CREATE TABLE IF NOT EXISTS `LevelFirstPass` (\
     `DateTime` VARCHAR(32)   ,\
     `LevelID` INT UNSIGNED   ,\
     `AvgTime2AccRegister` INT UNSIGNED   ,\
     `AvgStarEvalResult` FLOAT   ,\
     `AvgPlayerLevel` FLOAT   ,\
     `AvgTimeCost` INT UNSIGNED   ,\
     INDEX `LevelID_index` (`LevelID`),\
     INDEX `DateTime_index` (`DateTime`))\
     ENGINE=InnoDB DEFAULT CHARSET=utf8;"
    QueryDB.execute(sql)

    DWHandler = MysqlHandler(DWPara)
    sql = "select str_to_date(StartTime, '%%Y-%%m-%%d'), LevelID, SUM(PlayerLevel)/COUNT(LevelID) as AvgPlayerLevel,  SUM(TimeCost)/COUNT(LevelID) as AvgTimeCost, \
    SUM(StarEvalResult)/COUNT(LevelID) as AvgStarEvalResult, SUM(Time2AccRegister)/COUNT(LevelID) as AvgTime2AccRegister from LevelFirstPassLog \
    where str_to_date(StartTime, '%%Y-%%m-%%d') = '%s' and IsPass = 2\
    group by LevelID;" % date
    DWHandler.execute(sql)
    for row in DWHandler.get_result():
        sql = "insert into LevelFirstPass(DateTime, LevelID,  AvgPlayerLevel, AvgTimeCost, AvgStarEvalResult, AvgTime2AccRegister) values('%s', '%d', '%f', '%d','%f', '%d')" % row
        QueryDB.execute(sql)
    QueryDB.close()
    DWHandler.close()