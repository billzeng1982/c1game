include ../../global.mk

TARGET=HttpServer_ucloud HttpServer2_ucloud HttpClient_ucloud HttpClient2_ucloud

SOURCE = $(wildcard *.cpp)

OBJ=$(SOURCE:%.cpp=%.o)	

TARGET_INC=-I./ $(INC_COMM) $(INC_LIBEVENT) $(INC_LIBCURL)
TARGET_LIB=$(LIB_COMM) $(LIB_LIBEVENT) $(LIB_LIBCURL)

.PHONY: all clean

all: $(TARGET)

$(OBJ):%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@ $(TARGET_INC)
#	@echo compile $< ... $@

HttpServer_ucloud : HttpServer_ucloud.o
	@$(CXX) $(CXXFLAGS) -o $@ -z defs $^ $(TARGET_LIB)
	@echo Build ...... $@

HttpServer2_ucloud : HttpServer2_ucloud.o
	@$(CXX) $(CXXFLAGS) -o $@ -z defs $^ $(TARGET_LIB)
	@echo Build ...... $@


HttpClient_ucloud : HttpClient_ucloud.o
	@$(CXX) $(CXXFLAGS) -o $@ -z defs $^ $(TARGET_LIB)
	@echo Build ...... $@

HttpClient2_ucloud : HttpClient2_ucloud.o
	@$(CXX) $(CXXFLAGS) -o $@ -z defs $^ $(TARGET_LIB)
	@echo Build ...... $@

clean:
	@$(RM) $(OBJ)
	@$(RM) $(TARGET)
	@echo clean all *.o and $(TARGET) ...


