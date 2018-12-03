#!/usr/bin/env python
# coding:utf-8

import time
import os
import json
import hashlib

__author_ = "star"


class AccountMgr(object):
    def __init__(self):
        self.__secret_key = None
        self.__session_timeout = 0
        self.__session_dict = {}
        self.__account_dict = {}
        self.__trust_ip_list = []
        self.load_cfg()

    @classmethod
    def instance(cls):
        if not hasattr(cls, "_instance"):
            cls._instance = cls()
        return cls._instance

    def load_cfg(self):
        cfg_path = os.path.join(os.path.dirname(__file__), "config.json")
        f = open(cfg_path, "r")
        data = json.load(f)
        self.__account_dict = data["account_list"]
        self.__trust_ip_list = data["trust_ip_list"]
        self.__secret_key = data["secret_key"]
        self.__session_timeout = data["session_timeout"]

    def login(self, user, pwd, ip):
        self.clear_session()
        if user not in self.__account_dict.keys():
            return None
        # if ip not in self.__trust_ip_list:
        #     return None
        if pwd != self.__account_dict[user]:
            return None
        sig_str = user + str(time.localtime()) + self.__secret_key
        my_md5 = hashlib.md5()
        my_md5.update(sig_str)
        session_id = my_md5.hexdigest()
        user_info = {"user": user, "ip": ip, "deadline": int(time.time()) + self.__session_timeout}
        self.__session_dict[session_id] = user_info
        return session_id

    def authentication(self, session_id, ip):
        if session_id not in self.__session_dict.keys():
            return False
        user_info = self.__session_dict[session_id]
        if user_info['ip'] != ip:
            return False
        if int(time.time()) > user_info['deadline']:
            self.__session_dict.pop(session_id)
            return False
        user_info['deadline'] = int(time.time()) + self.__session_timeout
        return True

    def clear_session(self):
        for session_id in self.__session_dict.keys():
            user_info = self.__session_dict[session_id]
            if int(time.time()) > user_info['deadline']:
                self.__session_dict.pop(session_id)
