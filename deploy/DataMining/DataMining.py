#!/usr/bin/env python
# -*- coding: utf-8 -*-
__author__ = 'Administrator'

import datetime
import os
import sys
import xml.dom.minidom
from CltFightstats import *
from PlayerDataStats import *

def Datamining():
    db_dict = {}
    stats_dict = {}
    #print(os.path.realpath(__file__))
    #print(os.path.split(os.path.realpath(__file__)))
    #print( "Get source:",os.path.abspath(inspect.getsourcefile(lambda:0)) )
    #print( "Current Dir:",os.getcwd())
    #print(os.path.split(os.path.realpath(__file__))[0]) 
    #print("input any Key to next")
    #raw_input()
    #os.chdir(os.path.split(os.path.realpath(__file__))[0])
    ## 因为os.chdir 切换当前目录后,再通过os.path.realpath(__file__) 就不准确了
    dom = xml.dom.minidom.parse(os.path.split(os.path.realpath(__file__))[0] + '/DataMining.xml')
    root =dom.documentElement

    g_config = root.getElementsByTagName('globleconfig')[0]

    db_config = g_config.getElementsByTagName('Database')

    for db in db_config:
        db_dict_node={}
        name = db.getAttribute('name')
        db_dict_node['host'] = db.getAttribute('host')
        db_dict_node['port'] = int(db.getAttribute('port'))
        db_dict_node['username'] = db.getAttribute('username')
        db_dict_node['password']  = db.getAttribute('password')
        db_dict[name] = db_dict_node
        
    #print(db_dict)    
    stats_config = root.getElementsByTagName('statistics')
    for stats in stats_config:
        stats_node_config = stats.getElementsByTagName('StatsNode')
        for stats_node in stats_node_config:
            stats_dict_node = {}
            name = stats_node.getAttribute('name')
            stats_dict_node['scriptpath'] = stats_node.getAttribute('scriptpath')
            stats_dict_node['DWdb'] = stats.getAttribute('DWdb')
            stats_dict_node['Querydb'] = stats.getAttribute('dimension')
            stats_dict[name] = stats_dict_node
            #print(name, stats_dict[name])

    
    task_config = root.getElementsByTagName('task')

    yesterday =datetime.date.today() - datetime.timedelta(days=1)
    #import pdb
    #pdb.set_trace()
    for task_node in task_config:
        name = task_node.getAttribute('name')
        if task_node.getAttribute('run') == '0':
            continue
        funcname = stats_dict[name]['scriptpath'] + "stats"
        db_dict['DataWarehouse']['db'] = stats_dict[name]['DWdb']
        db_dict['QueryDB']['db'] = stats_dict[name]['Querydb']
        #print("****", funcname, yesterday, db_dict["DataWarehouse"], db_dict["QueryDB"])
        #raw_input()
        eval(funcname)(yesterday, db_dict["DataWarehouse"], db_dict["QueryDB"])

if __name__=="__main__":
    Datamining()



