#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import tornado.httpserver
import tornado.ioloop
import tornado.web
import signal
import os
import sys
import socket
import struct
import urlparse

__author__ = 'star'

g_DEBUG = False


class GetSvrListHandler(tornado.web.RequestHandler):
    @tornado.web.asynchronous
    def post(self):
        print self.request.body
        server_list = self._gen_svr_list(self._get_channel(self.request.body))
        rsp_data = {"SvrList":  server_list}
        print rsp_data
        self.finish(json.dumps(rsp_data, ensure_ascii=False))

    def _gen_svr_list(self, channel):
        new_svr_list = []
        old_svr_list = DirSvr.instance().svr_list
        rule = DirSvr.instance().rule
        exclude_svr_ids = rule.get(channel, [])

        for server in old_svr_list:
            print "exclude_svr_ids:", exclude_svr_ids
            if server["Id"] in exclude_svr_ids:
                continue

            server_info = {"Id": server["Id"], "Name": server["Name"], "Port": server["Port"], "State": server["State"],
                           "Ip": struct.unpack("I", socket.inet_aton(server["Ip"]))[0], "Url":server["Url"]}


            new_svr_list.append(server_info)
        return new_svr_list

    def _get_channel(self, para):
        tmp = urlparse.parse_qs(para).get("channel",[])
        if len(tmp) is 0:
            return "0";
        else:
            return tmp[0]

class ReloadSvrConfigHandler(tornado.web.RequestHandler):
    @tornado.web.asynchronous
    def post(self):
        pass


class Application(tornado.web.Application):
    def __init__(self):
        handles = [
            (r"/GetSvrList", GetSvrListHandler),
            (r"/ReloadSvrConfig", ReloadSvrConfigHandler),
        ]
        tornado.web.Application.__init__(self, handles)


class DirSvr(object):
    def __init__(self):
        self.port = 0
        self.server = tornado.httpserver.HTTPServer(Application())
        self.svr_list = None
        self.rule = None

    @classmethod
    def instance(cls):
        if not hasattr(cls, "_instance"):
            cls._instance = cls()
        return cls._instance

    def load_cfg(self):
        f = file("Config.json")
        cfg = json.load(f, encoding="utf8")
        f.close()
        self.port = cfg["Port"]
        self.svr_list = cfg["SvrList"]
        self.rule = cfg["Rule"]

    def start(self):
        self.load_cfg()
        self.server.listen(self.port)

    def stop(self):
        pass


def main():
    signal.signal(signal.SIGTERM, onsignal_term)
    __on_start()


def __on_start():
    DirSvr.instance().start()
    tornado.ioloop.IOLoop.instance().start()


def onsignal_term(a, b):
    DirSvr.stop()
    tornado.ioloop.IOLoop.instance().stop()

if __name__=="__main__":
    #启动程序
    #调试时直接启用,不用fork
    if not g_DEBUG:
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
