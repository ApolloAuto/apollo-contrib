#
# Makefile for adv_trigger tool
#

PSWROOT=../../..
CC=gcc
CFLAGS=-Wall -march=native -O2 -pipe -static
LIB_PATH=../../lib/adv_trigger
LIB=$(LIB_PATH)/libadv_trigger.a
INC=$(PSWROOT)/include
SRC=adv_trigger.c

.PHONY:all
all: adv_trigger

adv_trigger: $(SRC) $(LIB)
	$(CC) $(CFLAGS) -I$(INC) -I$(LIB_PATH) -o adv_trigger -L$(LIB_PATH) $(SRC) -ladv_trigger

$(LIB):
	$(MAKE) -C $(LIB_PATH)

.PHONY:install
install: all
	cp adv_trigger $(PSWROOT)/bin/

.PHONY:clean
clean:
	rm -f adv_trigger
