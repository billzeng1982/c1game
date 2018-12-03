#!/usr/bin/env python
#encoding=utf-8

import redis
import sys
table1 = "SerialNum"
table2 = "UsedTable"

def del_used_serialnum(host, port):
    client = redis.StrictRedis(host=host, port=port)
    keys1 = client.hgetall(table1).keys()
    keys2 = client.hgetall(table2).keys()
    "del element in serialnum table if the key in usedtable"
    keys3 = [key for key in keys1 if key in keys2]
    for key in keys3:
        client.hdel(table1, key)
        client.hdel(table2, key)

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print "error: num of para less 3"
        sys.exit()
    del_used_serialnum(sys.argv[1], sys.argv[2])
