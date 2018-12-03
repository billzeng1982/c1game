#!/usr/bin/env python
# coding:utf-8

__author_ = "star"


class AgentSvrMgr(object):

    def __init__(self):
        self.__server_dict = {}

    @classmethod
    def instance(cls):
        if not hasattr(cls, "_instance"):
            cls._instance = cls()
        return cls._instance

    def reg_server(self, server_id, name, status, url):
        if server_id in self.__server_dict.keys():
            self.__server_dict[server_id]['name'] = name
            self.__server_dict[server_id]['status'] = status
            self.__server_dict[server_id]['url'] = url
        else:
            server_info = {'name': name, 'status': status, 'url': url}
            self.__server_dict[server_id] = server_info

    def get_url_by_id(self, server_id):
        if server_id in self.__server_dict.keys():
            return "http://" + self.__server_dict[server_id]['url']
        else:
            return None

    def get_server_list(self):
        server_list = {}
        for server in self.__server_dict.keys():
            if self.__server_dict[server]['status'] == "1":
                server_list[server] = self.__server_dict[server]['name']
        return server_list
