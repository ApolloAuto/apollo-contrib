#
# Makefile for Baidu softwares
#

LIBCOMMON=src/lib/common
LIBADV_TRIGGER=src/lib/adv_trigger
LIBADV_BCAN=src/lib/bcan
LIBSENSOR_SYNC=src/lib/sensor_sync
ADV_TRIGGER=src/tools/adv_trigger
BCAN=src/tools/bcan
SENSOR_SYNC=src/tools/sensor_sync
BAIDU_GPS=src/tools/gps
BASA=src/kernel/drivers/baidu/basa
MODULES=$(LIBADV_TRIGGER) $(LIBADV_BCAN) $(ADV_TRIGGER) $(BCAN) $(LIBCOMMON) $(LIBSENSOR_SYNC) $(SENSOR_SYNC) $(BAIDU_GPS) $(BASA)
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
