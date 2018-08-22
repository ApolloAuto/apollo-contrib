
LIBADV_TRIGGER=src/lib/adv_trigger
LIBADV_BCAN=src/lib/bcan
ADV_TRIGGER=src/tools/adv_trigger
BASA=src/kernel/drivers/baidu/basa
MODULES=$(LIBADV_TRIGGER) $(LIBADV_BCAN) $(ADV_TRIGGER) $(BASA)
MODINSTALL=$(addsuffix .install,$(MODULES))
MODCLEAN=$(addsuffix .clean,$(MODULES))

.PHONY:all $(MODULES)
all: $(MODULES)
$(MODULES):
	$(MAKE) -C $@

.PHONY:install $(MODINSTALL)
install: $(MODINSTALL)
$(MODINSTALL): %.install:
	$(MAKE) install -C $*

.PHONY:clean $(MODCLEAN)
clean: $(MODCLEAN)
$(MODCLEAN): %.clean:
	$(MAKE) clean -C $*
