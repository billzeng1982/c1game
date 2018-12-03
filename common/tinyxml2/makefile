#=============================================================
# makefile for tinyxml2
#=============================================================

include ../../global.mk

SOURCE = $(wildcard *.cpp)
OBJ_FILES := $(patsubst %.cpp,%.o,$(SOURCE))

%.o:%.cpp	
	@$(CXX) $(CXXFLAGS) -c $< -I./	
	@echo [CXX] $@

all:$(OBJ_FILES)
	$(AR) q ../$(COMM_LIB_FILE) $(OBJ_FILES)

.PHONY: clean

clean:
	rm -f $(OBJ_FILES)

