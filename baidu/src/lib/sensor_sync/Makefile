#
# Makefile for sensor_sync library
#

BAIDUROOT=../../..
VERSION=3.0.0.4
SONAME=libadv_sensor_sync.so

CXX=g++
CFLAGS=-Wall -Wno-unused-result -march=native -O2 -pipe -DLIB_VER='"$(VERSION)"'
CXXFLAGS=-std=c++11 ${CFLAGS}
SOFLAGS=-fPIC -shared -Wl,-soname,$(SONAME)
AR=ar
INC=.  \
     -I$(BAIDUROOT)/src/lib/sensor_sync/3rd/include \
	-I$(BAIDUROOT)/src/lib/adv_trigger \
	-I$(BAIDUROOT)/src/lib/common \
    -I$(BAIDUROOT)/src/kernel/include/uapi
SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp, %.o, $(SRC))
HDR=sensor_sync.h
LIB=libadv_sensor_sync.a
LIBSO=libadv_sensor_sync.so.$(VERSION)
LIB_3RD=$(BAIDUROOT)/src/lib/sensor_sync/3rd/lib/libjsoncpp.a

.PHONY:all
all: $(LIB) $(LIBSO)

$(LIB): $(SRC)
	$(CXX) $(CXXFLAGS) -c $(SRC) $(INC)
	$(AR) rcs $(LIB) $(OBJ) $(LIB_3RD)

$(LIBSO):
	$(CXX) $(CXXFLAGS) $(SOFLAGS) -o $(LIBSO) -I$(INC) $(SRC)

.PHONY:install
install: all
	mkdir -p $(BAIDUROOT)/output/lib $(BAIDUROOT)/output/include
	cp $(LIB) $(BAIDUROOT)/output/lib/
	cp $(LIBSO) $(BAIDUROOT)/output/lib/
	cp $(HDR) $(BAIDUROOT)/output/include/

.PHONY:Clean
clean:
	rm -f *.o $(LIB) $(LIBSO)
