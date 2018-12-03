#!/usr/bin/env python
# -*- coding: utf-8 -*-

__author__ = 'star'

import re
import json
import hashlib
import os
import sys
import tornado.httpclient
import tornado.httpserver
import tornado.ioloop
import tornado.web
import signal
import logging
import traceback
import urllib
import time
from logging.handlers import TimedRotatingFileHandler

g_DEBUG = 0
ISOTIMEFORMAT='%Y-%m-%d %X'



class MyLog(object):

    def __init__(self):
        #初始化充值流水日志参数
        self.__pay_log = logging.getLogger("pay_log")
        pay_log_handler = TimedRotatingFileHandler(filename="pay_log", when="midnight", interval=1, backupCount=1000)
        self.__pay_log.addHandler(pay_log_handler)
        #初始化errlog参数
        self.__err_log = logging.getLogger("err_log")
        err_log_handler = TimedRotatingFileHandler(filename="../../log/ClusterSdkCbSvr_err.log", when="midnight", interval=1, backupCount=10)
        err_formatter = logging.Formatter('%(asctime)s %(filename)s[line:%(lineno)d] %(message)s')
        err_log_handler.setFormatter(err_formatter)
        self.__err_log.addHandler(err_log_handler)
        #初始化runlog参数
        self.__run_log = logging.getLogger("run_log")
        run_log_handler = TimedRotatingFileHandler(filename="../../log/ClusterSdkCbSvr_run.log", when="midnight", interval=1, backupCount=10)
        run_formatter = logging.Formatter('%(asctime)s %(filename)s[line:%(lineno)d] %(message)s')
        run_log_handler.setFormatter(run_formatter)
        self.__run_log.addHandler(run_log_handler)

    @classmethod
    def instance(cls):
        if not hasattr(cls, "_instance"):
            cls._instance = cls()
        return cls._instance

    def paylog(self, log_content):
        self.__pay_log.error(log_content)

    def logerr(self, log_content):
        self.__err_log.error(log_content)

    def logrun(self, log_content):
        self.__run_log.error(log_content)


def log_run(log):
    MyLog.instance().logrun(log)


def log_err(log):
    MyLog.instance().logerr(log)


def log_pay(log):
    MyLog.instance().paylog(log)


#将字典转化为key1=value1&key2=value2的格式(按照key的字典序)
def stringify(req_dict, exclude_key_list=None):
        result = ""
        if not isinstance(req_dict, dict):
            return result
        for key in sorted(req_dict.keys()):
            if exclude_key_list is not None and key in exclude_key_list:
                continue
            result += "%s=%s&" % (key, req_dict[key])
        return result[0:-1]

class ZoneSvrMgr(object):

    def __init__(self):
        self.zonesvr_list = {}
        self.combine_svr_list = {}
        f = file("combine_svr.json")
        cfg = json.load(f)
        f.close()
        self.combine_svr_list = cfg["CombineSvrList"]


    @classmethod
    def instance(cls):
        if not hasattr(cls, "_instance"):
            cls._instance = cls()
        return cls._instance

    def reg_zonesvr(self, svr_id, addr):
        self.zonesvr_list[svr_id] = addr

    def get_cb_addr(self, svr_id):
        """
        回包到合服之后的服务器
        """
        svr_id = str(svr_id)
        tar_svr_id = self.combine_svr_list.get(svr_id, svr_id)
        return self.zonesvr_list.get(tar_svr_id, None)




class ZoneSvrRegHandler(tornado.web.RequestHandler):

    def post(self):
        log_run("Recv reg msg : " + self.request.body)
        data = json.loads(self.request.body)
        if "SvrID" not in data.keys():
            self.write("FAIL")
            return
        if "CbUrl" not in data.keys():
            self.write("FAIL")
            return
        ZoneSvrMgr.instance().reg_zonesvr(data["SvrID"], data["CbUrl"])
        self.write("SUCCESS")


class XYPayCbHandler(tornado.web.RequestHandler):

    def __init__(self, application, request, **kwargs):
        tornado.web.RequestHandler.__init__(self, application, request, **kwargs)
        self.__para_dict = {}
        self.__appSecret = "Gui47576SkEPC247cUCpZC867G4757c6"
        self.PARA_LIST = ["productName", "orderNo", "channelOrderNo", "userID", "appID", "serverID",
                          "money", "currency", "extension", "sign"]
        self.set_header("Connection", "close")

    @tornado.web.asynchronous
    def post(self):
        log_run("Recv Xiyou paycb msg :" + self.request.body)
        #解析数据
        self.__parse_data(self.request.body)
        #检查数据
        if not self.__check_para():
            self.finish("Fail")
            return
        log_content = "%s|%s|%s|%s|%s|%s" % (self.__para_dict["roleID"], self.__para_dict["userID"], self.__para_dict["serverID"],
                                            self.__para_dict["money"], self.__para_dict["orderNo"], time.strftime(ISOTIMEFORMAT, time.localtime()))
        log_pay(log_content)
        #检查签名
        if not self.__check_sig(self.request.body):
            self.finish("Fail")
            return
        #将数据转为json格式
        json_data = self.__data_to_json()
        if json_data is None:
            self.finish("Fail")
            return
        #发给游戏服务器
        url = ZoneSvrMgr.instance().get_cb_addr(self.__para_dict["serverID"])
        if url is None:
            log_err("can't find server " + self.__para_dict["serverID"])
            self.finish("Fail")
            return

        log_run("send post req to " + url + ". postinfo=" + json_data)
        http_client = tornado.httpclient.AsyncHTTPClient()
        http_request = tornado.httpclient.HTTPRequest(url, method="POST", body=json_data, request_timeout=10)
        http_client.fetch(http_request, self.__on_response)

    def __on_response(self, response):
        if response is None:
            log_err("call gamesvr timeout")
            self.finish("Fail")
        else:
            self.finish(response.body)

    def __parse_data(self, data):
        """
            解析原始的请求数据，生成一个kye——value对应的字典
        """
        self.__para_dict.clear()
        #用&作为分割符分割请求字符串
        para_list = data.split("&")
        #遍历分割后的每个子串，并用=作为分割符分割出key和value
        for para in para_list:
            record = para.split("=")
            if len(record) > 1:
                self.__para_dict[record[0]] = record[1]
            elif len(record) == 1:
                self.__para_dict[record[0]] = ""
        para_list = self.__para_dict["extension"].split("-")
        self.__para_dict["productID"] = para_list[0]
        self.__para_dict["roleID"] = para_list[1]

    def __check_para(self):
        """
            检查请求数据的格式是否正确，参数是否齐全
        """
        for para in self.PARA_LIST:
            if para not in self.__para_dict.keys():
                log_err("check req para fail, " + para + "is not exist")
                return False
        return True

    def __check_sig(self, data):
        """
            检查请求数据的签名是否正确
        """
        #计算回调数据的签名
        exclude_key_list = ["sign", "productID", "roleID"]
        sig_str = stringify(self.__para_dict, exclude_key_list) + self.__appSecret
        m = hashlib.md5()
        m.update(urllib.unquote(sig_str))
        sig = m.hexdigest()
        #判断计算的签名和回调数据中的签名是否一致
        if sig == self.__para_dict["sign"]:
            return True
        else:
            log_err("cal sig(%s) and para sig(%s) is not matched" % (sig, self.__para_dict["sign"]))
            return False

    def __data_to_json(self):
        """
            将请求数据转为json以便发给游戏服务器
        """
        json_data = '{"productID":%s,"roleID":%s,"userID":"%s"}' % (self.__para_dict["productID"], self.__para_dict["roleID"], self.__para_dict["userID"])
        return json_data


class CommonPayCbHandler(tornado.web.RequestHandler):

    def __init__(self, application, request, **kwargs):
        tornado.web.RequestHandler.__init__(self, application, request, **kwargs)
        self.__para_dict = {}
        self.__appSecret = "WE3CWgM35Qaw00ktwAZCme8M88m2mWw2"
        self.PARA_LIST = ["channel", "uid", "orderId", "serverId", "productId", "buyNum", "money", "roleId", "extension"]

    @tornado.web.asynchronous
    def post(self):
        log_run("Recv common paycb msg :" + self.request.body)
        self.__para_dict = json.loads(self.request.body)
        #检查数据
        if not self.__check_para():
            self.finish("Fail")
            return
        #记录充值流水
        log_content = "%s|%s|%s|%s|%s|%s" % (self.__para_dict["roleId"], self.__para_dict["uid"], self.__para_dict["serverId"],
                                            self.__para_dict["money"], self.__para_dict["orderId"], time.strftime(ISOTIMEFORMAT, time.localtime()))
        log_pay(log_content)
        #将数据转为json格式
        json_data = self.__data_to_json()
        if json_data is None:
            self.finish("Fail")
            return

        #发给游戏服务器
        url = ZoneSvrMgr.instance().get_cb_addr(str(self.__para_dict["serverId"]))
        if url is None:
            log_err("can't find server " + str(self.__para_dict["serverId"]))
            self.finish("Fail")
            return
        log_run("send post req to " + url + ". postinfo=" + json_data)
        http_client = tornado.httpclient.AsyncHTTPClient()
        http_request = tornado.httpclient.HTTPRequest("http://"+ url, method="POST", body=json_data, request_timeout=5)
        http_client.fetch(http_request, self.__on_response)

    def __on_response(self, response):
        if response is None:
            log_err("call gamesvr timeout")
            self.finish("Fail")
        else:
            self.finish(response.body)

    def __check_para(self):
        """
            检查请求数据的格式是否正确，参数是否齐全
        """
        for para in self.PARA_LIST:
            if para not in self.__para_dict.keys():
                log_err("check req para fail, " + para + "is not exist")
                return False
        return True

    def __data_to_json(self):
        """
            将请求数据转为json以便发给游戏服务器
        """
        json_data = '{"productID":%s,"roleID":%s,"userID":"%s","orderId":"%s"}' % (self.__para_dict["productId"], self.__para_dict["roleId"], self.__para_dict["uid"], self.__para_dict["orderId"])
        return json_data


class TestHandler(tornado.web.RequestHandler):
    def get(self):
        log_run("test ClusterCbSvr")
        self.write("test OK")


class Application(tornado.web.Application):
    def __init__(self):
        handles = [
            (r"/RayE/ZoneSvrReg", ZoneSvrRegHandler),
            (r"/RayE/XiYou/PayCallBack", XYPayCbHandler),
            (r"/RayE/Common/PayCallBack", CommonPayCbHandler),
            (r"/RayE/test", TestHandler),
        ]
        tornado.web.Application.__init__(self, handles)


def main():
    signal.signal(signal.SIGTERM, onsignal_term)
    log_run("start ClusterSdkCbSVr")
    server = tornado.httpserver.HTTPServer(Application())
    server.listen(8081)
    try:
        tornado.ioloop.IOLoop.instance().start()
    except:
        s = traceback.format_exc()
        log_err(s)


def onsignal_term(a, b):
    tornado.ioloop.IOLoop.instance().stop()


if __name__=="__main__":
    #启动程序
    #调试时直接启用,不用fork
    if 0 == g_DEBUG:
        try:
            pid = os.fork()
            if pid > 0:
                sys.exit(0)
        except OSError, e:
            print >>sys.stderr, "fork #1 failed: %d (%s)" % (e.errno, e.strerror)
            sys.exit(1)
        os.setsid()
        os.umask(0)
        try:
            pid = os.fork()
            if pid > 0:
                sys.exit(0)
        except OSError, e:
            print >>sys.stderr, "fork #2 failed: %d (%s)" % (e.errno, e.strerror)
            sys.exit(1)
    else:
        print("Debug Now----->")
    main()
