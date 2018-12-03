#==============================================================
# makefile for common library 
#==============================================================

include ../global.mk

#protocol ±ÿ–Îœ»±‡“Î
MODULE = protocol sys algo utils mng tinyxml2 log dbhandler transaction tresload thread http msglayer coroutine

all:
	rm -f $(COMM_LIB_FILE)
	@for dir in $(MODULE);do \
	( $(MAKE) -C $$dir all ); done

	@echo -en "\033[1;36m"
	$(RANLIB) $(COMM_LIB_FILE)
	$(AR) -t $(COMM_LIB_FILE)

	@echo -en "\033[0m"
	@echo
	@echo -e "\033[1;36m======= Packing Library $(COMM_LIB_FILE) Done ============ \033[0m"

clean:
	rm -f $(COMM_LIB_FILE)
	@for dir in $(MODULE);do \
	( $(MAKE) -C $$dir clean ); done
	@echo
	@echo -e "\033[1;38m====== Cleaning Target Done ==================== \033[0m"

