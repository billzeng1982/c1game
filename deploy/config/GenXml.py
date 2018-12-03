#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
    执行说明:
        默认 加载脚本同级目录下的GenXml.json
        选项:
            -c #加载脚本同级目录下的GenXmlCluster.json
            --config JsonName   #加载指定文件名的json文件

    生成xml配置文件
    Global:
        Common:
            为公有参数值,在ProcessServer里配置指定字段值时可直接加前缀"v_com_"取值
            也可以转换成Python表达式算值
            如
            "DBAddr"                        :   "v_com_MysqlIp"
            "PlayerMaxNum"                  :   "%str(int(int(v_com_MaxPlayerNum)*0.8))"
            "URLRoot"                       :   "%'http://'+v_com_DataSvrPubIp+'/Replay/GuildReplay'"

            建议把经常修改的值放在Common中

        TbusSvrAddr:
            配置各个Process的tbus地址,可直接加前缀"v_addr_"取值
            如  "ProcIDStr":"v_addr_FriendSvr"
        TbusSvrToSvr:
            配置Process之间的通信
            如:ZoneSvr和ZoneConn需要建立tbus连接进行通信,则 ["ZoneSvr",  "ZoneConn"]
            注意:不要配重了
        ServerList:
            配置服务器列表，服务器属于哪个级别world,cluster


    内网模式:

        目的:
            自动读取本地Ip生成, 不用每次更新GenXml.json后都要重写Ip地址
        作用条件:
            本机地址为 172.16.1.*
            DeployType = "all"

        准备工作:
            1.在Python命令行模式中, import socket ,然后调用socket.gethostname()
            2.在/etc/hosts中增加一行 172.16.1.*  HOSTNAME(上面返回的字符串)

"""

import logging
import socket
import struct
import sys
import getopt

sys.path.append("..")
from utilpy.XMLGenerator import XMLGenerator
from utilpy.com_util import *

JSON_NAME = "GenXml.json"




def main():
    analyse_opt()
    cfg = load_json(JSON_NAME)





    glo = cfg['Global']
    local_ip = get_local_ipv4()

    if is_private_net(local_ip) and glo["Common"]["DeployType"] == "all":
        glo["Common"]["WorldSvrPubIp"] = local_ip
        glo["Common"]["WorldSvrPriIp"] = local_ip
        glo["Common"]["ClusterSvrPubIp"] = local_ip
        glo["Common"]["ClusterSvrPriIp"] = local_ip
        glo["Common"]["IdipSvrIp"] = local_ip
        glo["Common"]["DirSvrPubIp"] = local_ip

    print "WorldSvrPubIp %s \t WorldSvrPriIp %s \nClusterSvrPubIp %s\t ClusterSvrPriIp %s" %    \
     (glo["Common"]["WorldSvrPubIp"], glo["Common"]["WorldSvrPriIp"],                   \
     glo["Common"]["ClusterSvrPubIp"], glo["Common"]["ClusterSvrPriIp"])

    cfg_dict = init_cft_to_dict(cfg)

    global_arg = globals()
    #更新到全局变量中
    global_arg.update(cfg_dict)


    local_sever_list = gen_local_server_list(glo["Common"]["DeployType"], glo["ServerList"])
    server_dic = gen_server_dic(glo["ServerList"])

    gen_tbusd_xml(glo["Common"])
    gen_tbus_services_xml(glo["TbusSvrAddr"], glo["TbusSvrToSvr"], local_sever_list,  glo["Common"])

    gen_tbus_trelaymgr_xml(glo["TbusSvrAddr"], glo["TbusSvrToSvr"], local_sever_list, server_dic, glo["Common"])
    gen_svr_xml(cfg, local_sever_list)


def print_ret(name, ret):
    """
    打印
    """
    ret_tag = "[Error]"
    if ret:
        ret_tag = "[Ok]"

    print "%-8sGenerate\t %s " % (ret_tag, name)






def gen_local_server_list(deploy_type, server_list):
    """
    生成本机的serve列表
    """
    try:
        local_server_list = []
        deploy_type_list = deploy_type.split('|')
        for one in deploy_type_list:
            if one == "world":
                local_server_list += server_list["WorldSvr"]
            elif one == "cluster":
                local_server_list += server_list["ClusterSvr"]
            elif one == "all":
                local_server_list = server_list["WorldSvr"] + server_list["ClusterSvr"]
            else:
                raise StandardError
        return local_server_list
    except StandardError, e:
        print_ret("gen_local_server_list", False)
        logging.exception(e)


def gen_server_dic(server_list):
    """
    ServerList中做反向映射,方便后面根据SvrName找到对应的SvrPriIp
    """
    try:
        server_dic = {}

        for svr_key, svr_list  in server_list.items():
            for svr in svr_list:
                server_dic[svr] = svr_key

        return server_dic
    except StandardError, e:
        print_ret("gen_server_dic", False)
        logging.exception(e)
    pass


def gen_tbusd_xml(com):
    """
    生成tbusd.xml 配置
    """
    try:
        xml_gen = XMLGenerator("tbusd.xml")
        root = xml_gen.init_root("Tbusd")
        xml_gen.create_add_node(root, "RelayShmKey", com["RelayShmKey"])
        xml_gen.create_add_node(root, "GCIMShmKey", com["GCIMShmKey"])
        #取服务器的部署方式
        deploy_type = com["DeployType"].split('|')[0]
        if deploy_type == "cluster":
            listen_ip = com["ClusterSvrPriIp"]
        elif deploy_type == "world":
            listen_ip = com["WorldSvrPriIp"]
        elif deploy_type == "all":
            listen_ip = com["WorldSvrPriIp"]
        else:
            raise StandardError
        listen = "tcp://%s:%s?reuse=1" % (listen_ip, com["RelayPort"])
        xml_gen.create_add_node(root, "Listen", listen)
        xml_gen.create_add_node(root, "PkgMaxSize", com["SendSize"])
        xml_gen.create_add_node(root, "SendBuff", com["SendSize"])
        xml_gen.create_add_node(root, "RecvBuff", com["RecvSize"])
        xml_gen.set_node_att(root, "version", "9")
        xml_gen.gen_xml()
        print_ret("tbusd.xml", True)

    except StandardError, e:
        print_ret("tbusd.xml", False)
        logging.exception(e)


def gen_tbus_services_xml(tbus_svr_addr, tbus_svr_to_svr, server_list, com):
    """
    生成 tbus_services.xml
    """
    try:
        xml_gen = XMLGenerator("tbus_services.xml")
        root = xml_gen.init_root("TbusGCIM")
        xml_gen.create_add_node(root, "AddrTemplet", com["AddrTemplet"])
        xml_gen.create_add_node(root, "GCIMShmKey", com["GCIMShmKey"])

        for one in tbus_svr_to_svr:
            if one[0] not in server_list and one[1] not in server_list: #一组配置中的两个进程都不在本机，不用配置
                continue
            channel = xml_gen.create_add_node(root, "Channels")
            xml_gen.create_add_node(channel, "Priority", "8")
            xml_gen.create_add_node(channel, "Desc", one[0] + "\tand\t\t" + one[1])
            xml_gen.create_add_node(channel, "Address", tbus_svr_addr[one[0]])
            xml_gen.create_add_node(channel, "Address", tbus_svr_addr[one[1]])
            xml_gen.create_add_node(channel, "SendSize", com["SendSize"])
            xml_gen.create_add_node(channel, "RecvSize", com["RecvSize"])

        xml_gen.gen_xml()
        print_ret("tbus_services.xml", True)

    except StandardError, e:
        print_ret("tbus_services.xml", False)
        logging.exception(e)


def gen_tbus_trelaymgr_xml(tbus_svr_addr, tbus_svr_to_svr, server_list, server_dic, com):
    """
    生成 trelaymgr.xml, tbus路由配置
    """
    try:
        xml_gen = XMLGenerator("trelaymgr.xml")
        root = xml_gen.init_root("RelayMnger")
        xml_gen.create_add_node(root, "AddrTemplet", com["AddrTemplet"])
        xml_gen.create_add_node(root, "RelayShmKey", com["RelayShmKey"])
        complete_server_list = []

        for one in tbus_svr_to_svr:
            #只有当一个服务器在本地，另一个不在时，才需要配置路由
            if one[0] in server_list and one[1] in server_list:
                continue
            if one[0] not in server_list and one[1] not in server_list:
                continue
            outer_server = one[1]
            if one[1] in server_list:
                outer_server = one[0]

            if outer_server in complete_server_list:
                continue
            #print server_dic.get(outer_server,"")
            route_ip = com.get(server_dic.get(outer_server,"") + "PriIp", None)
            route_port = com["RelayPort"]
            mconn = "tcp://%s:%s" % (route_ip, route_port)
            #生成节点
            child = xml_gen.create_add_node(root, "Relays")
            xml_gen.set_node_att(child, "type", "ShmRelay")
            xml_gen.create_add_node(child, "Addr", tbus_svr_addr[outer_server])
            xml_gen.create_add_node(child, "MConn", mconn)
            xml_gen.create_add_node(child, "Desc", outer_server)
            #将处理完的服务器加入complete列表
            complete_server_list.append(outer_server)

        xml_gen.gen_xml()
        print_ret("trelaymgr.xml", True)

    except StandardError, e:
        print_ret("trelaymgr.xml", False)
        logging.exception(e)


def gen_svr_xml(cfg, local_sever_list):
    """
    生成 功能Svr配置文件
    """

    #for (_, value) in [("_", cfg["ProcessServer"]["MiscSvr"])]:
    for (svr, value) in sorted(cfg["ProcessServer"].items()):
        if value.get("XML_Switch", "1") == "1" and svr in local_sever_list:
            file_name = "" + value["XML_FileName"]
            root_node_name = value["XML_RootNodeName"]
            del value["XML_Switch"]
            del value["XML_FileName"]
            del value["XML_RootNodeName"]
            gen_one_svr_xml(file_name, root_node_name, value)


def gen_one_svr_xml(file_name, root_node_name, svr_args):
    """
    生成指定Svr的xml文件
    """
    try:
        xml_gen = XMLGenerator(file_name)
        root = xml_gen.init_root(root_node_name)
        parse_dict_to_xml(xml_gen, root,  svr_args)
        xml_gen.gen_xml()
        print_ret(file_name, True)

    except StandardError, e:
        print_ret(file_name, False)
        logging.exception(e)


def parse_dict_to_xml(xml_gen, root,  parse_args):
    """
    递归分析json转化为xml
    """
    for (key, value) in sorted(parse_args.items()):
        if isinstance(value, dict):
            child = xml_gen.create_add_node(root, key)
            parse_dict_to_xml(xml_gen, child,  value)
        elif len(value) >= 2 and value[0] == "v" and value[1] == '_':
            xml_gen.create_add_node(root, key, eval(value))
        elif value[0] == "%":
            xml_gen.create_add_node(root, key, eval(value[1:]))
        else:
            xml_gen.create_add_node(root, key, value)



def init_cft_to_dict(cfg):
    """
    初始化某些变量
    """
    cfg_dict = {}
    for (key, value) in cfg["Global"]["Common"].items():
        cfg_dict["v_com_" + key] = value

    for (key, value) in cfg["Global"]["TbusSvrAddr"].items():

        if "." not in value:

            #字符串中没有点,需要程序自动生成
            cfg_dict["v_addr_" + key] = cfg["Global"]["Common"]["GameClusterID"] + "." +\
            cfg["Global"]["Common"]["GameServerID"] + "." +  value + ".1"

            #回写数据
            cfg["Global"]["TbusSvrAddr"][key] = cfg_dict["v_addr_" + key]
        else:
            cfg_dict["v_addr_" + key] = value






    return cfg_dict


def analyse_opt():
    """
    分析参数
    """
    glo = globals()
    opts, args = getopt.getopt(sys.argv[1:],"hc", ["config="] )
    for op, value in opts:
        if op == "-h":
            print __doc__
            sys.exit()
        elif op == "-c":
            glo["JSON_NAME"] = "GenXmlCluster.json"
            print "load " + JSON_NAME
        elif op == "--config":
            glo["JSON_NAME"] = value
            print "load " + JSON_NAME

    return

if __name__ == "__main__":

    main()

