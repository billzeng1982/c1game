Log生成sql建表基础语句:
../../tools/tsf4g/tmeta2tab -B ../../deploy/protocol/dwlog_svr.bin -o ./dwlog_svr.sql -c utf8 -m LevelFirstPassLog
生成建表语句后，注意流水日志表的 ENGINE=InnoDB 需要替换成ENGINE=INFOBRIGHT


StatsIndicator.xml: 统计指标描述，用于生成查询表的建表语句
../../tools/tsf4g/tmeta2tab -B ../../deploy/protocol/StatsIndicator.bin -o ./StatsIndicator.sql -c utf8 -m LevelFirstPass                         
