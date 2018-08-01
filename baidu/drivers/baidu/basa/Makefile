#
# Makefile for Baidu Sensor Aggregation driver
#

obj-m += basa.o
basa-objs := basa_driver.o basa_intr.o \
	basa_main.o basa_chan.o basa_sysfs.o basa_fwupdate.o \
	basa_can.o basa_video.o basa_cam_hci.o basa_gps.o \
	basa_trigger.o basa_i2c.o basa_reg.o
ccflags-y := -I${PWD}/../../../include/uapi/

KERNEL_SOURCE ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	${MAKE} -C ${KERNEL_SOURCE} M=${PWD} $(MAKE_OPTS) modules
clean:
	${MAKE} -C ${KERNEL_SOURCE} M=${PWD} clean
	rm -f *.o.ur-safe
