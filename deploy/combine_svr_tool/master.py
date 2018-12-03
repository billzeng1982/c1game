#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
合服工具 -- 主服务器调用
"""

import subprocess
import time
import sys

sys.path.append("..")
from utilpy.com_util import *
from utilpy.XMLGenerator import XMLGenerator
import tidy_data

## 待合入的服务器列表
g_CombineSvrList = [5,6]

def backup_data_file():
    """
    删除文件数据 包括:排行榜 公会战等相关
    """
    bash="""
    mkdir -p /data/backdata/filedata/;
    mv ../services/RankSvr/Rank6v6Table /data/backdata/filedata/;
    mv ../services/RankSvr/RankLiTable /data/backdata/filedata/;
    mv ../services/RankSvr/RankDailyChallengeTable  /data/backdata/filedata/;
    mv ../services/RankSvr/RankGCardCntTable  /data/backdata/filedata/;
    mv ../services/RankSvr/RankGCardLiTable  /data/backdata/filedata/;
    mv ../services/RankSvr/RankPveStarTable /data/backdata/filedata/;
    mv ../services/RankSvr/RankGuildTable /data/backdata/filedata/;
    mv ../services/RankSvr/RankGFightTable /data/backdata/filedata/;
    mv ../services/RankSvr/RankPeakArena /data/backdata/filedata/;
    mv ../services/GuildSvr/GuildFightSchedule /data/backdata/filedata/;
    mv ../services/GuildSvr/GuildBossSchedule /data/backdata/filedata/;
    mv ../services/ReplaySvr/ReplayList /data/backdata/filedata/;
    mv ../services/AsyncPvpSvr/PlayerRankList /data/backdata/filedata/;
    """
    subprocess.call(bash, shell=True)


def import_becombined_c1game(target_svr_id, com):
    """
    导入要合并的数据库
    """
    tar_c1game = "./target_c1game/c1game_%s.sql" % target_svr_id
    ex_mysql_cmd(com, tar_c1game,"f")
    print("import %s ok" % tar_c1game)
    time.sleep(10)

def drop_becombined_c1game(target_svr_id, com):
    """
    删除导入的表
    """

def gen_combine_svr_xml(com, svr_id):
        xml_gen = XMLGenerator("CombineSvr.xml")
        root = xml_gen.init_root("CombineSvrCfg")
        xml_gen.create_add_node(root, "BeCombinedSidList", str(svr_id))
        xml_gen.create_add_node(root, "MysqlPingFreq", com["MysqlPingFreq"])
        xml_gen.create_add_node(root, "MysqlPort", com["MysqlPort"])
        xml_gen.create_add_node(root, "MysqlUser", com["MysqlUser"])
        xml_gen.create_add_node(root, "MysqlDBName", com["MysqlDBName"])
        xml_gen.create_add_node(root, "MysqlDBAddr", com["MysqlIp"])
        xml_gen.create_add_node(root, "MysqlPassword", com["MysqlPassword"])

        xml_gen.gen_xml()


def main():
    """
    执行
    """
    print ("## start %s " % __file__)

    backup_data_file()
    glo = load_json("../config/GenXml.json")
    com = glo["Global"]["Common"]
    com["MysqlSet"] = "latin1"


    for one in g_CombineSvrList:
        #生成新的CombineSvrXml
        gen_combine_svr_xml(com, one)

        #导入要合并的数据库
        import_becombined_c1game(one, com)

        #删除错误的或者无效的数据 避免合并失败
        tidy_data.main(one)



        #执行数据合并操作
        subprocess.call("./CombineSvrD --work-dir=./", shell=True)


        print("Combine Svr %s", one)






    print ("## end %s " % __file__)


if __name__ == "__main__":
    main()



