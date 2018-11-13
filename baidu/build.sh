#!/bin/bash

VERSION=$(cat .version | xargs)

make clean
make install
make clean

cd output
ldconfig -n lib
zip -y -r plat-sw-${VERSION}.zip *
rm -r bin include lib kernel
