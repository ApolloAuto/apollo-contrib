#!/bin/bash
set -e
#set -u
set -o pipefail
CORENUM=$(grep -c ^processor /proc/cpuinfo)
export MAKEFLAGS="-j${CORENUM} -l${CORENUM}"
function clean() {
    if [[ -d "./build" ]]; then
        rm -rf ./build
    fi
    if [[ -d "./output" ]]; then
        rm -rf ./output
    fi
}
function build() {
    mkdir -p ./build ./output
    cd build
    cmake .. 
    make
    cd ../output
    if [ -d install ]; then
        rm -rf install
    fi
    mkdir install
    cd install
    cp ../../build/v2x .
    cp ../../conf/v2x.flag . 
    cp ../../vendor/cmcc/lib/libcmcc.so .
    files=`find ../../build/ -name "*.so*" |wc -l`
    if [ $files -ne 0 ]; then
        find ../../build/ -name "*.so*" |xargs cp -d -t .
    fi

    find ../../third_party/ -name "*.so*" |xargs cp -d -t .
    cd ..
    tar -zcvf v2x.tar.gz ./install
    rm -rf install
    cd ..
}
case $1 in
    clean)
        clean
        ;;
    build)
        build
        ;;
    *)
        clean && build
        ;;
esac
