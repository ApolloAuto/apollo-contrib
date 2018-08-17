#
# Makefile for adv_trigger library
#

PSWROOT=../../..
CC=gcc
CFLAGS=-Wall -Wno-unused-result -march=native -O2 -pipe
AR=ar
INC=$(PSWROOT)/include
SRC=adv_trigger_ctl.c
OBJ=adv_trigger_ctl.o
HDR=adv_trigger.h
LIB=libadv_trigger.a
LIBSO=libadv_trigger.so

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
	rm -f *.o *.a *.so
