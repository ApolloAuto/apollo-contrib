#
# Makefile for adv_trigger tool
#

BAIDUROOT=../../..
CC=gcc
CFLAGS=-Wall -march=native -O2 -pipe -static
LIB_PATH=../../lib/adv_trigger
LIB=$(LIB_PATH)/libadv_trigger.a
INC=$(BAIDUROOT)/src/kernel/include/uapi
SRC=adv_trigger.c
OBJ=adv_trigger

.PHONY:all
all: $(OBJ)

$(OBJ): $(SRC) $(LIB)
	$(CC) $(CFLAGS) -I$(INC) -I$(LIB_PATH) -o $(OBJ) -L$(LIB_PATH) $(SRC) -ladv_trigger

$(LIB):
	$(MAKE) -C $(LIB_PATH)

.PHONY:install
install: all
	mkdir -p $(BAIDUROOT)/output/bin
	cp $(OBJ) $(BAIDUROOT)/output/bin/

.PHONY:clean
clean:
	rm -f $(OBJ)
