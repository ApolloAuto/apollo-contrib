#
# Makefile for Baidu CAN library
#

PSWROOT=../../..
CC=gcc
CFLAGS=-Wall -march=native -O2 -pipe
AR=ar
INC=$(PSWROOT)/include
SRC=bcan_lib.c
OBJ=bcan_lib.o
HDR=bcan.h
LIB=libadv_bcan.a
LIBSO=libadv_bcan.so

.PHONY:all
all: $(LIB) $(LIBSO)

$(LIB): $(SRC)
	$(CC) $(CFLAGS) -c -o $(OBJ) -I$(INC) $(SRC)
	$(AR) rcs $(LIB) $(OBJ)

$(LIBSO):
	$(CC) $(CFLAGS) -fPIC -shared -o $(LIBSO) -I$(INC) $(SRC)

.PHONY:install
install: all
	cp $(LIB) $(PSWROOT)/lib/
	cp $(LIBSO) $(PSWROOT)/lib/
	cp $(HDR) $(PSWROOT)/include/baidu/

.PHONY:Clean
clean:
	rm -f *.o *.so *.a
