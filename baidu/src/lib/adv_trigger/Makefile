#
# Makefile for adv_trigger library
#

CC=gcc
AR=ar
SRC=./adv_trigger_ctl.c
INC=../../../include/uapi/
CLFAGS=-O2

all: lib_adv_trigger_a lib_adv_trigger_so

lib_adv_trigger_a: $(SRC)
	$(CC) ${CFLAGS} -c -o adv_trigger_ctl.o -I$(INC) $(SRC)
	$(AR) rcs libadv_trigger_ctl.a adv_trigger_ctl.o

lib_adv_trigger_so:
	$(CC) ${CFLAGS} -fPIC -shared -o libadv_trigger_ctl.so -I$(INC) $(SRC)

clean:
	rm -f *.o *.a *.so
