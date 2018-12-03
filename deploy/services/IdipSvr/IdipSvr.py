#!/usr/bin/env python
# coding: utf-8

__author_ = "star"

import tornado.httpserver
import tornado.httpclient
import tornado.web
import tornado.ioloop
import tornado.options
import json
import os
import sys
import base64
from AgentSvrMgr import *
from AccountMgr import *

g_DEBUG = 1
g_PRINT = False

def is_print():
    """
    是否打印信息
    """
    return g_PRINT

class LoginHandler(tornado.web.RequestHandler):
    @tornado.web.asynchronous
    def post(self):
        #TODO
        if is_print():
            print "LoginHandler recv req, requst=[%s], body=[%s]" % (self.request, self.request.body)
        data = json.loads(self.request.body)
        session_id = AccountMgr.instance().login(data["user"], data["pwd"], self.request.remote_ip)
        rsp = {'ret': 0, 'msg': '', 'session': ''}
        if session_id is None:
            rsp['ret'] = -1
            rsp['msg'] = '用户名或密码错误'
        else:
            rsp['session'] = session_id
        if is_print():
            print "LoginHandler response, data=[%s]" % str(rsp)
        self.finish(json.dumps(rsp))


class ServerRegHandler(tornado.web.RequestHandler):
    def post(self):
        #TODO
        if is_print():
            print self.request.body
        server_info = json.loads(self.request.body)
        AgentSvrMgr.instance().reg_server(server_info['id'], server_info['name'], server_info['status'], server_info['url'])
        self.write('OK')


class GetSvrListHandler(tornado.web.RequestHandler):
    def __init__(self, application, request, **kwargs):
        tornado.web.RequestHandler.__init__(self, application, request, **kwargs)
        self.__rsp_data = {'ret': 0, 'msg': '', 'server_list': {}}

    @tornado.web.asynchronous
    def get(self):
        #TODO
        if is_print():
            print "GetSvrListHandler recv req, requst=[%s]" % self.request
        is_login = False
        session_id = self.get_cookie("SessionId")
        if is_print():
            print "session=" + session_id
        if session_id is not None:
            is_login = AccountMgr.instance().authentication(session_id, self.request.remote_ip)
        if not is_login:
            self.__rsp_data['ret'] = -99
            self.__rsp_data['msg'] = '请先登录'
        else:
            self.__rsp_data['server_list'] = AgentSvrMgr.instance().get_server_list()
        #TODO
        if is_print():
            print "GetSvrListHandler response, data=[%s]" % str(self.__rsp_data)
        self.finish(json.dumps(self.__rsp_data))


class GmOpHandler(tornado.web.RequestHandler):
    def __init__(self, application, request, **kwargs):
        tornado.web.RequestHandler.__init__(self, application, request, **kwargs)
        self.__rsp_data = {'ret': 0, 'msg': '', 'rsp': {}}

    @tornado.web.asynchronous
    @tornado.gen.coroutine
    def post(self):
        is_login = False
        session_id = self.get_cookie("SessionId")
        if session_id is not None:
            is_login = AccountMgr.instance().authentication(session_id, self.request.remote_ip)
        if not is_login:
            self.__rsp_data['ret'] = -99
            self.__rsp_data['msg'] = '请先登录'
            self.finish(json.dumps(self.__rsp_data))
            return
        #TODO
        if is_print():
            print "GmOpHandler recv req, request=[%s], body=[%s]" % (str(self.request), self.request.body)
        data = json.loads(self.request.body)
        server_list = data['server_list'].split('|')
        for server_id in server_list:
            url = AgentSvrMgr.instance().get_url_by_id(server_id)
            if url is None:
                self.__rsp_data['ret'] = -1
                self.__rsp_data['msg'] += "服务器" + str(server_id) + "没有找到\n"
                continue
            else:
                req_data = base64.b64decode(data["req"])
                http_client = tornado.httpclient.AsyncHTTPClient()
                request = tornado.httpclient.HTTPRequest(url=url, method="POST", body=req_data, connect_timeout=2, request_timeout=5)
                response = None
                #TODO
                if is_print():
                    print "request idipagent, server=[%s], request=[%s]" % (server_id, str(request))
                try:
                    response = yield http_client.fetch(request)
                except Exception, e:
                    if is_print():
                        print e.message
                finally:
                    self._on_response(response, server_id)
        #TODO
        if is_print():
            print "GmOpHandler response, data=[%s]" % str(self.__rsp_data)
        self.finish(json.dumps(self.__rsp_data))

    def _on_response(self, response, server_id):
        #TODO
        if is_print():
            print "recv idipagent, server=[%s] rsp=[%s]" % (server_id, str(response))
        if response is not None:
            self.__rsp_data['rsp'][server_id] = base64.b64encode(response.body)
        else:
            self.__rsp_data['ret'] = -2
            self.__rsp_data['msg'] += "服务器" + str(server_id) + "请求超时\n"


class TxHandler(tornado.web.RequestHandler):
    def get(self):
        self.write("OK")


class Application(tornado.web.Application):
    def __init__(self):
        handles = [
            (r"/RayE/Gm/Login", LoginHandler),
            (r"/RayE/Gm/ServerReg", ServerRegHandler),
            (r"/RayE/Gm/GetSvrList", GetSvrListHandler),
            (r"/RayE/Gm/GmOp", GmOpHandler),
            (r"/RayE/Gm/test", TxHandler)
        ]
        tornado.web.Application.__init__(self, handles)


def main():
    port = 8090
    server = tornado.httpserver.HTTPServer(Application())
    server.listen(port)
    try:
        tornado.ioloop.IOLoop.instance().start()
    except:
        #s = traceback.format_exc()
        #logging.error(s)
        if is_print():
            print "error"


if __name__ == "__main__":
    if 0 == g_DEBUG:
        try:
            pid = os.fork()
            if pid > 0:
                sys.exit(0)
        except OSError, e:
            print >> sys.stderr, "fork #1 failed: %d (%s)" % (e.errno, e.strerror)
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

