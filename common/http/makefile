include ../../global.mk

SOURCE = $(wildcard *.cpp)

OBJ_FILES = $(patsubst %.cpp,%.o,$(SOURCE))

%.o:%.cpp
	@$(CXX) $(CXXFLAGS) -c $< $(INC_TSF4G) $(INC_LIBEVENT) $(INC_LIBCURL) -I./ -I../ -I../utils $(INC_COMM)
	@echo [CXX] $@

all:$(OBJ_FILES)
	$(AR) q ../$(COMM_LIB_FILE) $(OBJ_FILES)

.PHONY: clean

clean:
	@rm -f $(OBJ_FILES)
	@echo clean ... $(OBJ_FILES)


