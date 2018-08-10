DRVVERSION=-DLEVEL=3 -DREVI=10 -DBUILD=4
ESD_CPPFLAGS += -DBOARD_pcie402
ESD_CPPFLAGS +=-DOSIF_OS_LINUX $(DRVVERSION)
ESD_CPPFLAGS += -I $(src)
ESD_CPPFLAGS +=  -DMAJOR_LINUX=52  -DHOST_DRIVER=1
ESD_CPPFLAGS += $(shell if ( cat /proc/version | grep -i suse 2>&1 > /dev/null ); then echo -DDISTR_SUSE; fi;)
ifdef KBUILD_CPPFLAGS
  KBUILD_CPPFLAGS += $(ESD_CPPFLAGS)
else
  ifdef ccflags-y
    ccflags-y += $(ESD_CPPFLAGS)
  else
    CPPFLAGS += $(ESD_CPPFLAGS)
  endif
endif
obj-m += esdcan-pcie402.o
esdcan-pcie402-objs := esdcan.o osif.o nucleus.o esdacc.o board.o boardrc.o crc32.o
