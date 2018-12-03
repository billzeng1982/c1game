#!/usr/bin/env python
# -*- coding: utf-8 -*-

import random
import redis

chars = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
         'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z']

class Serial:

    def __init__(self, seq):
        self.__seq = seq
        self.__code = []
        self.__serial = ""

    def _convertTo36Base(self):
        """
        将seq变成36进制表示
        """
        seq = self.__seq
        for i in range(0, 3, 1):
            num = seq % 36
            self.__code.append(num)
            seq /= 36

    def _getRandomCode(self):
        """
        得到一组随机码
        """
        for i in range(0, 9, 1):
            self.__code.append(random.randint(0, len(chars)-1))

    def _mix(self):
        """
        混淆
        """
        for i in range(0, 3, 1):
            value = 0
            for j in range(3*i, 3*(i+1), 1):
                value += self.__code[j]
            self.__code[i+9] = (self.__code[i+9] + value) % 36

    def _getSerial(self):
        """
        获取最终的礼品码
        """
        for i in range(0, len(self.__code), 1):
            self.__serial += chars[self.__code[i]]
        return self.__serial

    def _clear(self):
        del self.__code[:]
        self.__serial = ""

    def genSerial(self):
        self._clear()
        self._getRandomCode()
        self._convertTo36Base()
        self._mix()
        return self._getSerial()

if __name__=="__main__":
    seq = int(raw_input("请输入要生成的激活码批次:"))
    count = int(raw_input("请输入要生成的激活码数量:"))

    i = 0
    serial_no = Serial(seq)
    serial_list = []
    serial_file = open("Serial_%s" % seq, 'w')
    r = redis.Redis(host='localhost', port=6379, db=0)

    while i < count:
        serial = serial_no.genSerial()
        if serial not in serial_list:
            serial_list.append(serial)
            i += 1
            serial_file.write(serial + '\r')
            r.sadd("Serial_%s" % seq, serial)