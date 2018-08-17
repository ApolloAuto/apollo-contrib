#
# Makefile for Baidu CAN library
#

CC=gcc
CFLAGS=-Wall -m64 -O2
SRC=./bcan_lib.c
INC=../../../include/uapi/

all: lib_bcan_a lib_bcan_so

lib_bcan_a: bcan_lib.c bcan.h
	$(CC) $(CFLAGS) -I$(INC) -c -o bcan_lib.o $(SRC)
	$(AR) rcs libadv_bcan.a bcan_lib.o

lib_bcan_so:
	$(CC) $(CFLAGS) -I$(INC) -fPIC -shared -o libadv_bcan.so $(SRC)

clean:
	rm -f *.o *.so *.a
