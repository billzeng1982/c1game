include ../../global.mk

#SOURCE=$(wildcard *.cpp)
#SRCDIR_RAW=$(shell ls -R | grep "/" | awk -F : '{print $1}')
#SRCDIR=$(shell echo $(SRCDIR_RAW) | sed 's/://g' )

SOURCE=$(shell find ./ -name "*.cpp")
OBJ=$(SOURCE:%.cpp=%.o)	

INCLUDES=$(INC_TSF4G) $(INC_MYSQL) $(INC_REDIS) -I./ -I../ $(INC_COMM)

.PHONY: all clean

all: $(OBJ)
	$(AR) q ../$(COMM_LIB_FILE) $(OBJ)

$(OBJ):%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -D_REENTRANT -c -o $@ $< $(INCLUDES)
	@echo compile $< ... $@

clean:
	@rm -f $(OBJ)
	@echo clean $(OBJ)

