cmake_minimum_required(VERSION 2.8)
#project(v2x_trafficlight)


###########
## Build ##
###########

#SET( CMAKE_CXX_FLAGS "-std=c++11 -O3 -Wall -lgflags -pthread")


#include_directories(
# ../../proto/include
#)
#include_directories(
# ../../msg_sets/asn1_source
#)
#include_directories(
# ../../proxy
#)
#include_directories(
#    ${CMAKE_CURRENT_SOURCE_DIR}
#    ${CMAKE_CURRENT_SOURCE_DIR}/../../
#    ${CMAKE_CURRENT_SOURCE_DIR}/../../../utils
#)
#link_directories(
#	${CMAKE_CURRENT_BINARY_DIR}../../build/proto
#	${CMAKE_CURRENT_BINARY_DIR}../../build/msg_sets
#)

#include_directories("/usr/local/include")  
#link_directories("/usr/local/lib")

#include_directories("/usr/include/x86_64-linux-gnu")  
#link_directories("/usr/lib/x86_64-linux-gnu")

add_library(policy v2x_traffic_light.cc)
target_link_libraries(policy protobuf gflags proto utils obu_conf glog)

add_executable(v2x_test ./test/v2x_traffic_light_test.cc)
target_link_libraries(v2x_test protobuf gflags grpc grpc++ proto policy proxy)



#############
## Install ##
#############
