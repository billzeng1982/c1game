#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
打包软件
本脚本需放在和deploy同级的文件夹下
tardeploy   -h|a|(s [--name=file1 | --dname=dfile1 | --dir=dir1]) [-o packed_name] [-p TargetDir]

            -h #帮助
            -a #打包所有文件
            -o packed_name #指定打包后的文件名
            -p TargetDir #指定打包的文件夹路径,必须指定到deploy,不指定默认为当前工作目录下的deploy
            -s #特殊打包, 必须结合下面的选项 ,可组合使用

                --name=file1[,file2...]     #打包 指定文件名文件,重名文件会被打进去
                --dname=dfile1[,dfile2...]  #打包指定文件名文件,路径从deploy下开始
                --dir=dir1[,dir2...]        #打包指定文件夹,路径从deploy下开始
执行样例:
./tardeploy.py -s --name=tbusd.xml,tbusd_log.xml --dname=config/CloneBattleSvr.xml,config/ZoneSvr.xml  --dir=cmd_tools,services/ZoneSvr
./tardeploy.py -s --name=tbusd.xml,tbusd_log.xml -o deploy.test.tgz -p "../../trunk/deploy"
./tardeploy.py -a -p "../../trunk/deploy" -o deploy.test.tar.gz

注意:
    没有过滤数据文件,需打包前删除
    打包后的文件保存在本脚本的同级目录下
    打包后最好check一下  packed_file_list.txt
"""
import sys
import os
import tarfile
import re
import getopt
import time
import shutil

### 是否全打包
IS_PACK_ALL = False
### 打包后的文件名
OUTPUT_FILE = None

### 打包的所有文件, 用于check,特别是增量打包的时候
PACKED_FILES = []
### 保存打包的文件列表的文件名
PACKED_FLIE_LIST_NAME = "packed_file_list.txt"
### 打包的文件夹路径
TARGET_DIR = None

### 打包选项对应的变量
TAR_NAME = []
TAR_DNAME = []
TAR_DIR = []
### 只打包目录且目录位于deploy层级下
#only_pack_dir = ["log", "pid"]

def usage():
    """
    用法
    """
    print __doc__


def analyse_opt():
    """
    分析参数
    """
    glo = globals()
    opts, args = getopt.getopt(sys.argv[1:],"haso:p:", ["name=", "dname=", "dir="])
    for op, value in opts:
        if op == "-h":
            usage()
            sys.exit()
        elif op == "-a":
            glo["IS_PACK_ALL"] = True
            print "will pack all files!"
        elif op == "-s":
            glo["IS_PACK_ALL"] = False
        elif op == "-o":
            glo["OUTPUT_FILE"] = value
            print "output_file:", OUTPUT_FILE, value
        elif op == "--name":
            glo["TAR_NAME"] = value.split(",")
        elif op == "--dname":
            glo["TAR_DNAME"] = [ "deploy/" + x for x in value.split(",")]
        elif op == "--dir":
            glo["TAR_DIR"] = [ "deploy/" + x for x in value.split(",")]
            print TAR_DIR
        elif op == "-p":
            glo["TARGET_DIR"] = value
            print "TARGET_DIR:", TARGET_DIR

    return

def make_targz(target_dir, out_name = None):
    """
    打包
    """
    print "make_targz:: ",out_name
    base_name = os.path.basename(target_dir)
    if out_name is None:
        out_name = base_name + ".tar.gz"

    tar = tarfile.open(name=out_name, mode="w:gz")

    for root, dir1, files in os.walk(base_name):
        #print root,dir1,files
        for file1 in files:
            full_path = os.path.join(root, file1)
            if re.compile(".svn").search(full_path):
                #print "exclude: "+full_path
                continue
            tar.add(full_path, filter=targz_filter)

    tar.close()

def is_in_tar_dir(filename, dirs):
    for tardir in dirs:
        if filename[0:len(tardir)] == tardir:
            return True

    return False

def targz_filter(tarinfo):
    """
    过滤器
    针对单个文件,返回tarinfo,是要打包,返回None,不打包
    """
    #print tarinfo.name
    if IS_PACK_ALL \
    or os.path.basename(tarinfo.name) in TAR_NAME \
    or tarinfo.name in TAR_DNAME\
    or is_in_tar_dir(tarinfo.name, TAR_DIR):
        PACKED_FILES.append(tarinfo.name)
        print "Packed:", tarinfo.name
        return tarinfo

    return None


def get_table_file():
    """
    得到要过滤的数据文件
    """
    exclude_files = []
    try:
        clearfile = open("./deploy/cmd_tools/clear.sh", "r")
        pattern = r"rm (.*)"
        for line in clearfile.readlines():
            ret_m = re.match(pattern, line)
            if ret_m is not None:
                exclude_files.append(os.path.basename(ret_m.group(1)))


        clearfile.close()
        return exclude_files
    except:

        return exclude_files

def write_packed_files(files):
    ff = file(PACKED_FLIE_LIST_NAME,"w")
    [ff.write(x+"\n") for x in files]
    ff.close()

def main():
    """
    主函数
    """
    analyse_opt()

    output_file = OUTPUT_FILE
    if output_file is None:
        output_file = "deploy_" + time.strftime('%Y%m%d.%H%M%S',time.localtime(time.time())) + ".tar.gz"

    target_dir = TARGET_DIR
    if target_dir is None:
        target_dir = "./deploy"

    old_cwd = os.getcwd()
    os.chdir(os.path.dirname(target_dir))  #切换到与target_dir的同级目录 即与deploy同级
    print "cur_work_dir:" + os.getcwd()
    base_name = os.path.basename(target_dir)
    #exclude_files = get_table_file()
    make_targz(base_name, output_file)
    write_packed_files(PACKED_FILES)
    if old_cwd != os.getcwd():
        shutil.move(output_file, old_cwd)
        shutil.move(PACKED_FLIE_LIST_NAME, old_cwd)
    os.chdir(old_cwd)


if __name__ == "__main__":
    main()

