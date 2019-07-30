#
# Makefile for adv_trigger tool
#

BAIDUROOT=../../..
CC=gcc
CFLAGS=-Wall -march=native -O2 -pipe -static
INC=$(BAIDUROOT)/src/kernel/include/uapi
SRC=baidu_gps_time.c
OBJ=baidu_gps_time

LIB_COMMON_PATH=../../lib/common
LIB_COMMON=$(LIB_COMMON_PATH)/libadv_plat_common.a

.PHONY:all
all: $(OBJ)

$(OBJ): $(SRC) $(LIB_COMMON)
	$(CC) $(CFLAGS) -I$(INC) -I$(LIB_COMMON_PATH) -o $(OBJ)  $(SRC) $(LIB_COMMON)

.PHONY:install
install: all
	mkdir -p $(BAIDUROOT)/output/bin
	cp $(OBJ) $(BAIDUROOT)/output/bin/
	cp init_fpga_time.sh $(BAIDUROOT)/output/bin/

$(LIB_COMMON):
	$(MAKE) -C $(LIB_COMMON_PATH)


.PHONY:clean
clean:
	rm -f $(OBJ) $(LIB_COMMON)
