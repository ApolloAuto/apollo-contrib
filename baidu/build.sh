#!/bin/bash

VERSION=$(cat .version | xargs)
DRV_HDR=src/kernel/drivers/baidu/basa/basa_regs.h
DRV_VER=$(grep ZYNQ_MOD_VER ${DRV_HDR} | cut -d'"' -f2)

make clean
make install
make clean

cd output
ldconfig -n lib
zip -y -r plat-sw-${VERSION}.zip bin include lib
zip -y -r basa-${DRV_VER}.zip kernel
rm -r bin include lib kernel
