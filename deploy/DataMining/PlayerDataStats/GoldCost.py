#!/usr/bin/env python
# -*- coding: utf-8 -*-
__author__ = 'yingxi'
import sys

sys.path.append("..")

from MysqlHandler import *

def goldcoststats(date, DWPara, QueryDBPara):
    QueryDB = MysqlHandler(QueryDBPara)
    sql = "CREATE TABLE IF NOT EXISTS `gold_cnt_stats`(\
		`id` int(10) NOT NULL AUTO_INCREMENT,\
		`DateTime` varchar(32) DEFAULT NULL,\
		`ChgType` int(11) DEFAULT NULL,\
		`Approach` int(11) DEFAULT NULL,\
		`GoldCostPerSys` int(11) DEFAULT NULL,\
		`CostPerPlayerSys` int(11) DEFAULT NULL, \
		`CostTotalSvr` int(11) DEFAULT NULL,\
        `CostPerPlayerSvr` int(11) DEFAULT NULL,\
		PRIMARY KEY (`id`), INDEX `DateTime` (`DateTime`)\
		)ENGINE=InnoDB DEFAULT CHARSET=utf8;"
    QueryDB.execute(sql)

    DWHandler = MysqlHandler(DWPara)
    sql = "create OR REPLACE view vGoldAddTotal as \
    select SUM(ChgValue) as total, SUM(ChgValue)/COUNT(PlayerID) as costPerPlayer \
    from GoldLog \
    where DATE(DateTime)='%s' and ChgType = 1;" % date
    DWHandler.execute(sql)

    sql = "create OR REPLACE view vGoldMinusTotal as \
        select SUM(ChgValue) as total, SUM(ChgValue)/COUNT(PlayerID) as costPerPlayer \
        from GoldLog \
        where DATE(DateTime)='%s' and ChgType = 2;" % date
    DWHandler.execute(sql)

    sql = "select str_to_date(DateTime, '%%Y-%%m-%%d'), ChgType ,Approach, SUM(ChgValue), SUM(ChgValue)/COUNT(PlayerID) as CostPerPlayerSys, \
        vGoldAddTotal.total as CostTotalSvr, vGoldAddTotal.costPerPlayer as CostPerPlayerSvr\
		from GoldLog ,vGoldAddTotal\
		where str_to_date(DateTime, '%%Y-%%m-%%d') = '%s' and ChgType = 1\
		group by Approach;" % date
    DWHandler.execute(sql)
    for row in DWHandler.get_result():
        sql = "insert into gold_cnt_stats(DateTime, ChgType, Approach, GoldCostPerSys, CostPerPlayerSys, CostTotalSvr, \
            CostPerPlayerSvr) values('%s', '%d', '%d','%d', '%d', '%d', '%d')" % row
        QueryDB.execute(sql)

    sql = "select str_to_date(DateTime, '%%Y-%%m-%%d'), ChgType ,Approach, SUM(ChgValue), SUM(ChgValue)/COUNT(PlayerID) as CostPerPlayerSys, \
        vGoldMinusTotal.total as CostTotalSvr, vGoldMinusTotal.costPerPlayer as CostPerPlayerSvr\
    	from GoldLog ,vGoldMinusTotal\
    	where str_to_date(DateTime, '%%Y-%%m-%%d') = '%s' and ChgType = 2\
    	group by Approach;" % date
    DWHandler.execute(sql)
    for row in DWHandler.get_result():
        sql = "insert into gold_cnt_stats(DateTime, ChgType, Approach, GoldCostPerSys, CostPerPlayerSys, CostTotalSvr, \
            CostPerPlayerSvr) values('%s', '%d', '%d','%d', '%d', '%d', '%d')" % row
        QueryDB.execute(sql)

    sql = "drop view vGoldAddTotal;"
    DWHandler.execute(sql)

    sql = "drop view vGoldMinusTotal;"
    DWHandler.execute(sql)

    QueryDB.close()
    DWHandler.close()
