#
# Makefile for Baidu CAN library
#

BAIDUROOT=../../..
VERSION=3.0.0.1
SONAME=libadv_bcan.so.1

CC=gcc
CFLAGS=-Wall -march=native -O2 -pipe -DLIB_VER='"$(VERSION)"'
SOFLAGS=-fPIC -shared -Wl,-soname,$(SONAME)
AR=ar
INC=$(BAIDUROOT)/src/kernel/include/uapi
SRC=bcan_lib.c
OBJ=bcan_lib.o
HDR=bcan.h
LIB=libadv_bcan.a
LIBSO=libadv_bcan.so.$(VERSION)

.PHONY:all
all: $(LIB) $(LIBSO)

$(LIB): $(SRC)
	$(CC) $(CFLAGS) -c -o $(OBJ) -I$(INC) $(SRC)
	$(AR) rcs $(LIB) $(OBJ)

$(LIBSO):
	$(CC) $(CFLAGS) $(SOFLAGS) -o $(LIBSO) -I$(INC) $(SRC)

.PHONY:install
install: all
	mkdir -p $(BAIDUROOT)/output/lib $(BAIDUROOT)/output/include
	cp $(LIB) $(BAIDUROOT)/output/lib/
	cp $(LIBSO) $(BAIDUROOT)/output/lib/
	cp $(HDR) $(BAIDUROOT)/output/include/

.PHONY:Clean
clean:
	rm -f *.o $(LIB) $(LIBSO)
