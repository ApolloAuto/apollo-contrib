#
# Makefile for adv_trigger library
#

BAIDUROOT=../../..
VERSION=3.1.0.2
SONAME=libadv_trigger.so.1

CC=gcc
CFLAGS=-Wall -Wno-unused-result -march=native -O2 -pipe -DLIB_VER='"$(VERSION)"'
SOFLAGS=-fPIC -shared -Wl,-soname,$(SONAME)
AR=ar
SRC=adv_plat_common.c
OBJ=adv_plat_common.o
HDR=adv_plat_common.h
LIB=libadv_plat_common.a
INC=./
LIBSO=libadv_plat_common.so.$(VERSION)

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
