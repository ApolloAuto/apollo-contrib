
MODULES=src/lib/adv_trigger src/lib/bcan src/tools/adv_trigger
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
