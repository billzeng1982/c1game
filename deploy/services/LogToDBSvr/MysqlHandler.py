#!/usr/bin/python
# -*- coding: utf-8 -*-
# or__ = 'billzeng'

import MySQLdb

class MysqlHandler(object):
    def __init__(self, dbPara ):
        self.__conn = MySQLdb.connect(host=dbPara['host'],user=dbPara['username'], passwd=dbPara['password'], db=dbPara['db'], port=dbPara['port'], use_unicode=True)
        self.__cursor = self.__conn.cursor()
        self.__cursor.execute('SET NAMES utf8mb4')
        self.__cursor.execute("SET CHARACTER SET utf8mb4")
        self.__cursor.execute("SET character_set_connection=utf8mb4")

    def __del__(self):
        self.close()

    def close(self):
        if type(self.__conn)!=type(None):
            self.__cursor.close()
            self.__conn.commit()
            self.__conn.close()
            self.__conn = None

    def execute(self, sql):
        #print (sql)
        self.__cursor.execute(sql)
        

    def cursor(self):
        return self.__cursor

    def get_result(self):
        return self.__cursor.fetchall()
