#coding:utf-8
import xlrd
import threading

lock = threading.Lock()
class StaticData(object):
     # 定义静态变量实例
    __instance = None
    def __init__(self):
         data=xlrd.open_workbook(r'WebActivity/static/xlsx/Mission.xlsx')#在Nginx中打开表
         print "open"
         Mission1Table=data.sheet_by_name(u'活动任务表1')
         nrows=Mission1Table.nrows#获得行数
         Mission1TableData=[]
         for i in range(1,nrows):
            RowValue=Mission1Table.row_values(i)
            RowValue[6]=int(RowValue[6])
            RowValue[3]=int(RowValue[3])
            RowValue[5]="static/"+RowValue[5]+".png"
            Mission1TableData.append(RowValue)

         self.Mission1TableData=Mission1TableData

    def __new__(cls, *args, **kwargs):

            if not cls.__instance:
                print "instance not exist"

                try:
                    lock.acquire()
                    # double check
                    if not cls.__instance:
                        cls.__instance = object.__new__(cls, *args, **kwargs)
                finally:
                    lock.release()
            return cls.__instance
    @classmethod
    def instance(cls, *args, **kwargs):
        __instance=None
        if not hasattr(cls, "_instance"):
            cls._instance = cls()
            print "frist"
        return cls._instance


'''
    def getData(self):
         data=xlrd.open_workbook(r'WebActivit/static/xlsx/Mission.xlsx')#在Nginx中打开表
         print "open"
         MissionTable=data.sheet_by_name(u'任务表')
         ItemTable=data.sheet_by_name(u'物品表')
         MisssionIDList=MissionTable.col_values(0)
         ItemIDList=ItemTable.col_values(0)#物品表里的所有物品id
         #nrows=Mission.nrows#获得行数
         getdata={
             "MissionTable":MissionTable,
             "ItemTable":ItemTable,
             "MisssionIDList":MisssionIDList,
             "ItemIDList":ItemIDList
         }
         return getdata
'''
