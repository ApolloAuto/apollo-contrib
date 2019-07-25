#
# Makefile for adv_trigger tool
#

BAIDUROOT=../../..
CXX=g++
CFLAGS=-Wall -march=native -O3 -pipe -static
CXXFLAGS=-std=c++11 ${CFLAGS}
LIB_PATH=../../lib/sensor_sync
LIB=$(LIB_PATH)/libadv_sensor_sync.a

LIB_TRIGGER_PATH=../../lib/adv_trigger
LIB_TRIGGER=$(LIB_TRIGGER_PATH)/libadv_trigger.a

LIB_COMMON_PATH=../../lib/common
LIB_COMMON=$(LIB_COMMON_PATH)/libadv_plat_common.a

INC=.  \
     -I$(BAIDUROOT)/src/lib/sensor_sync/3rd/include \
	-I$(BAIDUROOT)/src/lib/adv_trigger \
	-I$(BAIDUROOT)/src/lib/common \
    -I$(BAIDUROOT)/src/kernel/include/uapi

LIB_3RD=$(BAIDUROOT)/src/lib/sensor_sync/3rd/lib/libjsoncpp.a \
        $(BAIDUROOT)/src/lib/sensor_sync/3rd/lib/libpcap.a

SRC=sensor_sync.cpp
OBJ=sensor_sync

.PHONY:all
all: $(OBJ)

$(OBJ): $(SRC) $(LIB) $(LIB_TRIGGER) $(LIB_COMMON)
	$(CXX) $(CXXFLAGS) -I$(INC) -I$(LIB_PATH) -o $(OBJ)  $(SRC) $(LIB) $(LIB_3RD) $(LIB_TRIGGER) $(LIB_COMMON)

$(LIB):
	$(MAKE) -C $(LIB_PATH)

$(LIB_TRIGGER):
	$(MAKE) -C $(LIB_TRIGGER_PATH)

$(LIB_COMMON):
	$(MAKE) -C $(LIB_COMMON_PATH)

.PHONY:install
install: all
	mkdir -p $(BAIDUROOT)/output/bin
	cp $(OBJ) $(BAIDUROOT)/output/bin/

.PHONY:clean
clean:
	rm -f $(OBJ)
