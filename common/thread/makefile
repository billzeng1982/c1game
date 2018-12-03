include ../../global.mk

SOURCE = $(wildcard *.cpp)

OBJ_FILES = $(patsubst %.cpp,%.o,$(SOURCE))

%.o:%.cpp
	@$(CXX) $(CXXFLAGS) -c $< $(INC_TSF4G) -I./ -I../ -I../sys -I../utils -I../protocol/PKGMETA -I../log
	@echo [CXX] $@

all:$(OBJ_FILES)
	$(AR) q ../$(COMM_LIB_FILE) $(OBJ_FILES)

.PHONY: clean

clean:
	@rm -f $(OBJ_FILES)
	@echo clean ... $(OBJ_FILES)

