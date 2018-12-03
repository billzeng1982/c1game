#!/usr/bin/python
# -*- coding: UTF-8 -*-
import xml.dom.minidom as xmldocument
import getopt
import sys


__author__ = 'nitong'

TIME = ''
"""
    部署该脚本到远程端,与ZoneSvr.xml同目录下
    修改ZoneSvr.xml中白名单
    --white_list状态:
        - close
        - open
"""


def open_white_list(time):
    # 使用minidom解析器打开 XML 文档
    dom = xmldocument.parse("ZoneSvr.xml")
    root = dom.documentElement
    # 修改LoginSDKSwitch为维护状态2
    login_element = root.getElementsByTagName("LoginSDKSwitch")[0]
    login_node = login_element.childNodes[0]
    login_node.nodeValue = 2
    # 修改SvrFinishUptTime
    time_element = root.getElementsByTagName("SvrFinishUptTime")[0]
    time_node = time_element.childNodes[0]
    time_node.nodeValue = time

    # 美化XML格式
    dom = beautiful_format(dom)
    # 写回xml文件
    with open("ZoneSvr.xml", 'w') as f:
        f.write(dom)


def close_white_list():
    # 使用minidom解析器打开 XML 文档
    dom = xmldocument.parse("ZoneSvr.xml")
    root = dom.documentElement
    # 修改LoginSDKSwitch为正常状态1
    login_element = root.getElementsByTagName("LoginSDKSwitch")[0]
    login_node = login_element.childNodes[0]
    login_node.nodeValue = 1

    # 美化XML格式
    dom = beautiful_format(dom)
    # 写回xml文件
    with open("ZoneSvr.xml", 'w') as f:
        f.write(dom)


def beautiful_format(xml_dom):
    # 美化xml格式
    if xml_dom:
        # 优化格式显示
        xml_str = xml_dom.toprettyxml(indent='', newl='', encoding='utf-8')
        xml_str = xml_str.replace('\t', '').replace('\n', '')
        xml_dom = xmldocument.parseString(xml_str)
        xml_str = xml_dom.toprettyxml(indent='\t', newl='\n', encoding='utf-8')
        return xml_str
    else:
        return False


def parse_argv(argv):
    try:
        opts, args = getopt.getopt(argv, "h", ["white=", "time="])
        if opts == []:
            print 'run python ./LoginSDKSwitch.py -h for help'
            sys.exit()
    except getopt.GetoptError, e:
        print str(e) 
        print 'run python ./LoginSDKSwitch.py -h for help'
        sys.exit()
    for opt, value in opts:
        if opt == '-h':
            print 'python ./LoginSDKSwitch.py --time=<2018/03/01:17:30:00> --white=<open|close> '
            sys.exit()
        elif opt == '--time':
            globals()["TIME"] = str(value)
            print "TIME::", value
        elif opt == '--white':
            white = value
            if 'open' == white:
                open_white_list(TIME)
                sys.exit()
            elif 'close' == white:
                close_white_list()
                sys.exit()
            else:
                print 'run python ./LoginSDKSwitch.py -h for help'
                sys.exit()


if __name__ == '__main__':
    parse_argv(sys.argv[1:])
    print "finished"

