
cmake_minimum_required(VERSION 2.8)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
#include_directories(
# ./asn1_source
#)
#add_subdirectory(asn1_source)
#add_subdirectory(ctfo)
#set(CMAKE_CXX_FLAGS "-std=c++11")
add_library(msg_set message_set.cc uu_message_set.cc)

#for uu_message_set

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/obu/include/asn1_source/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../vendor)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/cmcc)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/cmcc/include)

#link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/cmcc/build)

#add_library(uu_msg_set uu_message_set.cc)
#target_link_libraries(uu_msg_set cmcc)
