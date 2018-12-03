#=============================================================
# makefile for generating server resouce header file 
#=============================================================
include ../../global.mk
TDR=$(PATH_TOOLS)/tsf4g/tdr

XML_FILE = ov_res_keywords.xml ov_res_public.xml ov_res_server.xml
HEADER_FILE = $(patsubst %.xml,%.h,$(XML_FILE))

header : $(XML_FILE)
	$(TDR) -H --add_custom_prefix="m_" $^ -O .

.PHONY: clean all

all : header

clean:
	rm -f $(HEADER_FILE)


