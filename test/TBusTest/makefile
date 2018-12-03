include ../../global.mk

TARGET=send recv
SOURCE = $(wildcard *.cpp)

OBJ=$(SOURCE:%.cpp=%.o)	

TARGET_INC=-I./ $(INC_COMM) $(INC_TSF4G)
TARGET_LIB=$(LIB_COMM) $(LIB_TSF4G) -lpthread
.PHONY: all clean

all: $(TARGET)

$(OBJ):%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@ $(TARGET_INC)
#	@echo compile $< ... $@

send : send.o
	@$(CXX) $(CXXFLAGS) -o $@ -z defs $^ $(TARGET_LIB) 
	@echo Build ...... $@

recv : recv.o
	@$(CXX) $(CXXFLAGS) -o $@ -z defs $^ $(TARGET_LIB)
	@echo Build ...... $@

clean:
	@$(RM) $(OBJ)
	@$(RM) $(TARGET)
	@echo clean all *.o and $(TARGET) ...

