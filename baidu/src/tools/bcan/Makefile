#
# Makefile for bcan tools
#

BAIDUROOT=../../..
CC=gcc
CFLAGS=-Wall -march=native -O2 -pipe -static
LIB_PATH=../../lib/bcan
LIB=$(LIB_PATH)/libadv_bcan.a
INC=-I$(BAIDUROOT)/src/kernel/include/uapi -I$(LIB_PATH)
LDFLAGS=-L$(LIB_PATH) -ladv_bcan -lpthread
CAN_APP_SRC=can_app.c
CAN_APP=can_app
RADAR_APP_SRC=radar_can_app.c
RADAR_APP=radar_can_app

.PHONY:all
all: $(CAN_APP) $(RADAR_APP)

$(CAN_APP): $(CAN_APP_SRC) $(LIB)
	$(CC) $(CFLAGS) $(INC) -o $(CAN_APP) $(CAN_APP_SRC) $(LDFLAGS)

$(RADAR_APP): $(RADAR_APP_SRC) $(LIB)
	$(CC) $(CFLAGS) $(INC) -o $(RADAR_APP) $(RADAR_APP_SRC) $(LDFLAGS)

$(LIB):
	$(MAKE) -C $(LIB_PATH)

.PHONY:install
install: all
	mkdir -p $(BAIDUROOT)/output/bin
	cp $(CAN_APP) $(BAIDUROOT)/output/bin/
	cp $(RADAR_APP) $(BAIDUROOT)/output/bin/

.PHONY:clean
clean:
	rm -f $(CAN_APP) $(RADAR_APP)
