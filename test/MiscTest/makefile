include ../../global.mk

TARGET=TestMd5

SOURCE = $(wildcard *.cpp)

OBJ=$(SOURCE:%.cpp=%.o)	

TARGET_INC=-I./ $(INC_COMM) 
TARGET_LIB=$(LIB_COMM) $(LIB_TSF4G)

.PHONY: all clean

all: $(TARGET)

$(OBJ):%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@ $(TARGET_INC) 
	@echo compile $< ... $@

TestMd5 : TestMd5.o
	@$(CXX) $(CXXFLAGS) -o $@ -z defs $^ $(TARGET_LIB)
	@echo Build ...... $@

clean:
	@$(RM) $(OBJ)
	@$(RM) $(TARGET)
	@echo clean all *.o and $(TARGET) ...
	