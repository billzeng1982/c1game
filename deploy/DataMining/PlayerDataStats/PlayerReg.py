#!/usr/bin/env python
# -*- coding: utf-8 -*-
__author__ = 'yingxi'

import sys

sys.path.append("..")

from  MysqlHandler import *

def playerregstats(date, DWPara, QueryPara):
    QueryDB = MysqlHandler(QueryPara)
    sql = "CREATE TABLE IF NOT EXISTS `player_reg_stats`(\
    `id` int(10) NOT NULL AUTO_INCREMENT,\
    `DateTime` VARCHAR(32)  DEFAULT NULL ,\
    `PlayerRegCnt` int(11) DEFAULT NULL,\
	PRIMARY KEY (`id`), INDEX `DateTime` (`DateTime`)\
	)ENGINE=InnoDB DEFAULT CHARSET=utf8;"
    QueryDB.execute(sql)

    DWHandler = MysqlHandler(DWPara)

    sql = "select str_to_date(CreateTime, '%%Y-%%m-%%d'),COUNT(PlayerID) as PlayerRegCnt " \
          "from CreateNewLog \
          where str_to_date(CreateTime, '%%Y-%%m-%%d') = '%s';" % date
    DWHandler.execute(sql)
    for row in DWHandler.get_result():
        sql = "insert into player_reg_stats(DateTime, PlayerRegCnt) values ('%s', '%d')" % row
        QueryDB.execute(sql)
    QueryDB.close()
    DWHandler.close()
