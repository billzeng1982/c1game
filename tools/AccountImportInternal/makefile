include ../../global.mk

TARGET=./AccountImportInternal 

SOURCE = $(wildcard *.cpp)

OBJ=$(SOURCE:%.cpp=%.o)	

TARGET_INC=-I./ $(INC_COMM) $(INC_REDIS) $(INC_SERV_LIB) $(INC_MYSQL)
TARGET_LIB=$(LIB_COMM) $(LIB_REDIS) $(LIB_RESLOAD) $(LIB_TSF4G) $(LIB_MYSQL)

LDENV = LD_RUN_PATH=$(shell /usr/bin/mysql_config --variable=pkglibdir)
 
.PHONY: all clean

all: $(TARGET)

$(OBJ):%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@ $(TARGET_INC) 
	@echo compile $< ... $@

$(TARGET) : $(OBJ)
	@$(LDENV) $(CXX) $(CXXFLAGS) -o $@ -z defs $^ $(TARGET_LIB) -pthread
	@echo Build ...... $@

clean:
	@$(RM) $(OBJ)
	@$(RM) $(TARGET)
	@echo clean all *.o and $(TARGET) ...

