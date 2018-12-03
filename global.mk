#============================================================
# global makefile option
#============================================================

#=============================================================
# edition type
#-------------------------------------------------------------
# DEBUG   : debug edition
# RELEASE : release edition
#=============================================================

#BUILD = RELEASE
BUILD = DEBUG

#=============================================================
# file postfix setting
#=============================================================

ifeq ($(BUILD), DEBUG)
D = D
endif

ifeq ($(BUILD), RELEASE)
D =
endif

#=============================================================
# environment setting
#=============================================================

PATH_ROOT := $(dir $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))

PATH_TOOLS = $(PATH_ROOT)/tools
PATH_COMM = $(PATH_ROOT)/common
TSF4G_HOME = $(PATH_COMM)/tsf4g
PATH_TSF4G_LIB = $(TSF4G_HOME)/lib

COMM_LIB_FILE = libhdcomm$(D).a
SERV_LIB_FILE = libhdserv$(D).a

# libs ...
LIB_TSF4G = -L$(PATH_TSF4G_LIB) -ltsf4g -lscew -lexpat -ltlog
LIB_COMM  = -L$(PATH_COMM) -lhdcomm$(D)
LIB_RESLOAD = -L$(PATH_COMM)/tresload -lresloader

#includes ...
INC_TSF4G = -I$(PATH_COMM)/tsf4g/include
INC_COMM = -I$(PATH_COMM)/ \
		   -I$(PATH_COMM)/utils \
		   -I$(PATH_COMM)/tinyxml2 \
		   -I$(PATH_COMM)/transaction \
		   -I$(PATH_COMM)/tresload \
		   -I$(PATH_COMM)/tsf4g/include \
		   -I$(PATH_COMM)/algo \
		   -I$(PATH_COMM)/log \
		   -I$(PATH_COMM)/sys \
		   -I$(PATH_COMM)/mng \
		   -I$(PATH_COMM)/protocol/PKGMETA \
		   -I$(PATH_COMM)/protocol/DWLOG \
		   -I$(PATH_COMM)/dbhandler \
		   -I$(PATH_COMM)/transaction \
		   -I$(PATH_COMM)/thread \
		   -I$(PATH_COMM)/http \
		   -I$(PATH_COMM)/msglayer \
		   -I$(PATH_COMM)/coroutine

#=============================================================
# dependencies
#=============================================================
DEPS_ROOT = $(PATH_ROOT)/deps
INC_REDIS = -I$(DEPS_ROOT)/redis/include
LIB_REDIS = -L$(DEPS_ROOT)/redis/lib -lhiredis -ljemalloc
INC_LIBEVENT = -I$(DEPS_ROOT)/libevent/include
LIB_LIBEVENT = -L$(DEPS_ROOT)/libevent/lib -levent -lrt
INC_LIBCURL = -I$(DEPS_ROOT)/libcurl/include
LIB_LIBCURL = -L$(DEPS_ROOT)/libcurl/lib -lcurl -lz
INC_LUA = -I$(DEPS_ROOT)/lua/include
LIB_LUA = -L$(DEPS_ROOT)/lua/lib -llua -ldl
INC_LUABRIDGE = -I$(DEPS_ROOT)/luabridge/include
LIB_LUABRIDGE = -L$(DEPS_ROOT)/luabridge/lib
#LIB_PYTHON = $(shell /usr/bin/python-config --libs)
#INC_PYTHON = $(shell /usr/bin/python-config --includes)
LIB_PYTHON = $(shell python-config --libs)
INC_PYTHON = $(shell python-config --includes)

#mysql
#INC_MYSQL=$(shell /usr/bin/mysql_config --include)
#LIB_MYSQL=$(shell /usr/bin/mysql_config --libs)
INC_MYSQL=-I$(DEPS_ROOT)/mysql/include
LIB_MYSQL=-L$(DEPS_ROOT)/mysql/lib -lmysqlclient -lz -lcrypt -lnsl -lm -lssl -lcrypto -lrt -ldl

#=============================================================
# tools
#=============================================================
TDR=$(PATH_TOOLS)/tsf4g/tdr
TMETA2TAB=$(PATH_TOOLS)/tsf4g/tmeta2tab
RANLIB 	= ranlib
AR  	= ar
RM		= rm -rf

#=============================================================
# compile setting
#=============================================================

CC  = gcc
#CXX	= distcc g++
CXX 	= g++

#=============================================================
# compile option
#=============================================================

ifeq ($(BUILD), DEBUG)
CFLAGS = -Wall -g -D_DEBUG -fstack-protector-all
CXXFLAGS = -Wall -Wno-deprecated -Woverloaded-virtual -g -D_DEBUG -fstack-protector-all
DEFS = -DGM_ON -D_CPUSAMPLE
CFLAGS += $(DEFS)
CXXFLAGS += $(DEFS)
endif

ifeq ($(BUILD), RELEASE)
CFLAGS = -Wall -g -DNDEBUG 
CXXFLAGS = -Wall -Wno-deprecated -Woverloaded-virtual -g -DNDEBUG
endif

CFLAGS += -DLINUX -fPIC
CXXFLAGS += -DLINUX -fPIC


