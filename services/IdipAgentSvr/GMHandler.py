#!/usr/bin/env python
# coding:utf-8
# Copyright 2016 raye
__author_ = "yingxi"

import os.path
import tornado.httpserver
import tornado.httpclient
import tornado.web
import tornado.ioloop
import tornado.options
import json

from tornado.options import define, options

define("port", default=8800, help="set the port for this server", type=int)
# define("agenturl", default="http://172.16.1.241", help="set the url for agent", type=str)

class MainHandler(tornado.web.RequestHandler):
    _paras = {}
    @tornado.web.asynchronous
    def post(self):
        print 'post message'
        print self.request.body
        dict = json.loads(self.request.body)
        agent_client = tornado.httpclient.AsyncHTTPClient()
        if(dict.get('ZoneServerID', 0) == 1):
            url = "http://172.16.1.241:8082/GmQuery"
        else:
            self.write("can shu cuo wu~")
            self.finish()
            return
        if(dict.get('ReqType', 0) == 1):
            pkg = {}
            pkg['Uin'] = dict.get("Uin")
            pkg["StartTime"] = dict.get("StartTime")
            pkg["EndTime"] = dict.get("EndTime")
            pkg["Type"] = dict.get("LogType")
            pkg["Para1"] = dict.get("Para1",0)
            pkg["Para2"] = dict.get("Para2",0)
            pkg["Para3"] = dict.get("Para3",0)
            print json.dumps(pkg)
            self._send_request(url, json.dumps(pkg), callback=self._agent_responce)
        elif(dict.get('ReqType', 0) == 2):
            pkg = {}
            pkg["EndTime"] = '0'
            pkg["Para1"] = 2
            pkg["Para2"] = dict.get("Para2",0)
            pkg["Para3"] = dict.get("Para3",0)
            if(dict.get("Uin",0) == 0):
                pkg['Uin'] = 0
                pkg["StartTime"] = dict.get('Name', 0)
                if(dict.get('isGuild') == 0):
                    pkg["Type"] = 102 #玩家名查询
                else:
                    pkg["Type"] = 103 #查询军团信息
            #Uin查询
            else:
                pkg['Uin'] = dict.get("Uin", 0)
                pkg["Type"] = 101
                pkg["StartTime"] = '0'
            print json.dumps(pkg)
            self._send_request(url, json.dumps(pkg), callback=self._agent_responce)
        else:
            self.write("can shu cuo wu~")
            self.finish()
            return
            
    def _agent_responce(self, response):
        split_ch = chr(0)
        print response.body
        #strJson = "".join([string.strip().rsplit("}", 1)[0], "}"])
        if(response.body == None):
            self.write("{'error':'服务器没反馈，请重试'}")
        elif(response.body == ''):
            self.write("kong")
        # content = json.loads(strJson)
        else:
            contents = response.body.strip().split(split_ch)
            for content in contents:
                self.write(content)
        self.finish()
        
    def _send_request(self, url, data, callback):
        client = tornado.httpclient.AsyncHTTPClient()
        request = tornado.httpclient.HTTPRequest(url=url, method="POST", body=data, connect_timeout=3)
        client.fetch(request, callback=callback)
    # def _send_wrong(self):
    #     self.write("something is wrong")
    #     self.finish()


def main():
    tornado.options.parse_command_line()
    app = tornado.web.Application([
        (r"/", MainHandler),
    ])
    app.listen(options.port)
    tornado.ioloop.IOLoop.current().start()

if __name__ == "__main__":
    main()


