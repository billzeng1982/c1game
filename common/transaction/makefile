include ../../global.mk

SOURCE=$(wildcard *.cpp)
OBJ=$(SOURCE:%.cpp=%.o)	

INCLUDES=-I./ $(INC_TSF4G) -I../sys -I../ -I$(INC_COMM)

.PHONY: all clean

all: $(OBJ)
	$(AR) q ../$(COMM_LIB_FILE) $(OBJ)

$(OBJ):%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES)
	@echo compile $< ... $@

clean:
	@rm -f $(OBJ)
	@echo clean all *.o

