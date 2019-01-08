cmake_minimum_required(VERSION 2.8)
#project(v2x)
# set the cross compiling
#set(CMAKE_SYSTEM_NAME Linux)
# cross_compiler for C
#set(CMAKE_C_COMPILER "arm-poky-linux-gnueabi-gcc")
# cross_compiler for C++
#set(CMAKE_CXX_COMPILER "arm-poky-linux-gnueabi-g++")

#add_compile_options(-std=c++11)

# where is the target environment 
#set(CMAKE_FIND_ROOT_PATH  "/opt/fsl-imx-fb/4.9.88-2.0.0/sysroots/cortexa9hf-neon-poky-linux-gnueabi/usr" )
#    "./3rdLibs/glfags/lib" 
#    "./3rdLibs/glog/lib"
#    "./3rdLibs/grpc/lib"
#    "./3rdLibs/protobuf/lib")
 
# search for programs in the build host directories
#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# where to find header files
#include_directories(.)
#include_directories(../../3rdLibs/gflags/include)
#include_directories(../../3rdLibs/glog/include)
#include_directories(../../3rdLibs/grpc/include)
#include_directories(../../3rdLibs/protobuf/include)
#include_directories
# sub directories
#add_subdirectory(gps)
aux_source_directory(. DIRSRCS)
add_library(tl ${DIRSRCS})
add_dependencies(tl proto policy)
#target_link_libraries(v2x app util driver pal)
