include ../../global.mk

TARGET=RedisTest

SOURCE = $(wildcard *.cpp)

OBJ=$(SOURCE:%.cpp=%.o)	

TARGET_INC=-I./ $(INC_COMM) $(INC_REDIS)
TARGET_LIB=$(LIB_COMM) $(LIB_REDIS)

.PHONY: all clean

all: $(TARGET)

$(OBJ):%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@ $(TARGET_INC) 
	@echo compile $< ... $@

$(TARGET) : $(OBJ)
	@$(CXX) $(CXXFLAGS) -o $@ -z defs $^ $(TARGET_LIB) -pthread
	@echo Build ...... $@

clean:
	@$(RM) $(OBJ)
	@$(RM) $(TARGET)
	@echo clean all *.o and $(TARGET) ...

