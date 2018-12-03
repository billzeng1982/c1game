include ../../global.mk

TARGET=UdpRelayCltTest UdpRelaySvrTest
SOURCE = $(wildcard *.cpp)

OBJ=$(SOURCE:%.cpp=%.o)	

TARGET_INC=-I./ $(INC_COMM)
TARGET_LIB=$(LIB_COMM) -lpthread
.PHONY: all clean

all: $(TARGET)

$(OBJ):%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@ $(TARGET_INC)
#	@echo compile $< ... $@

UdpRelayCltTest : UdpRelayCltTest.o
	@$(CXX) $(CXXFLAGS) -o $@ -z defs $^ $(TARGET_LIB) 
	@echo Build ...... $@

UdpRelaySvrTest : UdpRelaySvrTest.o
	@$(CXX) $(CXXFLAGS) -o $@ -z defs $^ $(TARGET_LIB)
	@echo Build ...... $@

clean:
	@$(RM) $(OBJ)
	@$(RM) $(TARGET)
	@echo clean all *.o and $(TARGET) ...