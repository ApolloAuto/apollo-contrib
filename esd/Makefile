#########################################################################
# FILE NAME Makefile
#
# BRIEF MODULE DESCRIPTION
# 	    Makefile for ESDCAN-release-package
#
#
# History:
#
#  22.01.2016 - some cleanup, 32 & 64 bit builds for cantest       mj
#  23.07.2002 - initial version (based on MFs-makefiles in esdcan) ab
#
#########################################################################
#  Copyright (c) 1996 - 2016 by electronic system design gmbh
#
#  This software is copyrighted by and is the sole property of
#  esd gmbh.  All rights, title, ownership, or other interests
#  in the software remain the property of esd gmbh. This
#  software may only be used in accordance with the corresponding
#  license agreement.  Any unauthorized use, duplication, transmission,
#  distribution, or disclosure of this software is expressly forbidden.
#
#  This Copyright notice may not be removed or modified without prior
#  written consent of esd gmbh.
#
#  esd gmbh, reserves the right to modify this software without notice.
#
#  electronic system design gmbh          Tel. +49-511-37298-0
#  Vahrenwalder Str 207                   Fax. +49-511-37298-68
#  30165 Hannover                         http://www.esd-electronics.com
#  Germany                                sales@esd-electronics.com
#
#########################################################################
include ./config.mk

ifeq ($(TARGET_ARCH),ppc)
ifdef CROSS_COMPILE
CROSS      = $(CROSS_COMPILE)
else
CROSS      = ppc_4xx-
endif
endif

CC	   = $(CROSS)gcc
LD	   = $(CROSS)ld
STRIP	   = $(CROSS)strip

BLD_PATH = ./src/
BLDOBJECTS = *.o *.mod.c *.ko.cmd *.ko .tmp_versions .*.cmd *.order *.symvers

VPATH = $(BLD_PATH)

INCL = -I./include

CFLAGS    += -Wall -Wstrict-prototypes -fomit-frame-pointer
CFLAGS    += -O2

ifeq ($(TARGET_ARCH),ppc)
CFLAGS    += -DOSIF_ARCH_PPC -I$(KERNELPATH)/arch/$(TARGET_ARCH)
# kernel 2.6 cross compilation requires ARCH= argument
# do not forget to set CROSS_COMPILE correctly!
# Note: export ARCH in your shell before building!
KERNELARCH =
endif

ifeq ($(TARGET_ARCH),x86)
CFLAGS    += -DOSIF_ARCH_X86
endif

ifeq ($(TARGET_ARCH),arm)
CFLAGS    += -DOSIF_ARCH_ARM
endif


PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELPATH) $(KERNELARCH) SUBDIRS=$(PWD)/src ESDCAN_FLAGS=$(ESDCAN_FLAGS) modules
	@echo "Please do not forget to create nodes for the can-device in /dev/!"


BITSLIST:=$(subst lib,,$(wildcard lib*))

NTC_LIB_MAJOR=$(basename $(basename $(NTC_LIB_VERSION)))

cantest: $(foreach BITS,$(BITSLIST),bin$(BITS)/cantest)

cantest_static: $(foreach BITS,$(BITSLIST),bin$(BITS)/cantest_static)


ifneq (,$(findstring 32,$(BITSLIST)))
liblinks32:
	@cd ./lib32; \
	test -L libntcan.so.$(NTC_LIB_MAJOR) || ln -s ./libntcan.so.$(NTC_LIB_MAJOR).* ./libntcan.so.$(NTC_LIB_MAJOR); \
	test -L libntcan.so || ln -s ./libntcan.so.$(NTC_LIB_MAJOR) ./libntcan.so

bin32/cantest: example/cantest.c include/ntcan.h liblinks32
	$(CC) $(CFLAGS) -m32 -L./lib32 $(INCL) -o $@ $< -lntcan

bin32/cantest_static: example/cantest.c include/ntcan.h lib32/libntcan.a
	$(CC) $(CFLAGS) -static -m32 -L./lib32/ $(INCL) -o $@ $< -lntcan
endif


ifneq (,$(findstring 64,$(BITSLIST)))
liblinks64:
	@cd ./lib64; \
	test -L libntcan.so.$(NTC_LIB_MAJOR) || ln -s ./libntcan.so.$(NTC_LIB_MAJOR).* ./libntcan.so.$(NTC_LIB_MAJOR); \
	test -L libntcan.so || ln -s ./libntcan.so.$(NTC_LIB_MAJOR) ./libntcan.so

bin64/cantest: example/cantest.c include/ntcan.h liblinks64
	$(CC) $(CFLAGS) -m64 -L./lib64 $(INCL) -o $@ $< -lntcan

bin64/cantest_static: example/cantest.c include/ntcan.h lib64/libntcan.a
	$(CC) $(CFLAGS) -static -m64 -L./lib64/ $(INCL) -o $@ $< -lntcan
endif


clean:
	for t in $(BLDOBJECTS); do \
		rm -rfv $(BLD_PATH)$${t}; \
	done

show:
	@echo "KERNELPATH=$(KERNELPATH)"
	@echo "NTC_LIB_VERSION=$(NTC_LIB_VERSION)"
	@echo "NTC_LIB_MAJOR=$(NTC_LIB_MAJOR)"
	@echo "BITSLIST=$(BITSLIST)"

FORCE:
