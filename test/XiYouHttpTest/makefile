include ../../global.mk

TARGET=XiYouHttpTest

SOURCE = $(wildcard *.cpp)

OBJ=$(SOURCE:%.cpp=%.o)	

TARGET_INC=-I./ $(addprefix -I,$(SRCDIR)) $(INC_SERV_LIB) $(INC_COMM) $(INC_LIBEVENT) $(INC_LIBCURL)
TARGET_LIB=$(LIB_SERV) $(LIB_COMM) $(LIB_TSF4G) $(LIB_RESLOAD) $(LIB_LIBEVENT) $(LIB_LIBCURL) -lpthread

.PHONY: all clean

all: $(TARGET)

$(OBJ):%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@ $(TARGET_INC) 
	@echo compile $< ... $@

XiYouHttpTest : XiYouHttpTest.o
	@$(CXX) $(CXXFLAGS) -o $@ -z defs $^ $(TARGET_LIB)
	@echo Build ...... $@

clean:
	@$(RM) $(OBJ)
	@$(RM) $(TARGET)
	@echo clean all *.o and $(TARGET) ...
	