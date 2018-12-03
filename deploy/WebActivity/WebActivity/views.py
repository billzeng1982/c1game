#coding:utf-8
from django.shortcuts import render
from django.http import HttpResponse,HttpResponseRedirect,HttpRequest
import xlrd
import os
import json
import requests
import staticdata



def index(request):

        print request.GET

        request.session['Uin']=request.GET['Uin']
        request.session['SvrId']=request.GET['SvrId']
        request.session['SvrIP']=request.GET['SvrIP']
        return render(request,"Index.html")
def indexPage(request):
    return render(request,"Index.html")

def activity1(request):

            MissionList={}
            Mission1TableData=staticdata.StaticData.instance().Mission1TableData
            for i in range(0,7):
                 listNum=str(i+1)
                 mission="mission"+listNum
                 MissionList[mission]=Mission1TableData[i]

            MissionList['Uin']=request.session['Uin']
            MissionList['SvrIP']=request.session['SvrIP']
            MissionList['SvrId']=request.session['SvrId']

            return render(request,'activity1.html',MissionList)

def activity2(request):
    return render(request,"activity2.html")

def Notice(request,id):
     id=id
     return render(request,"Notice.html",{"id":id})


