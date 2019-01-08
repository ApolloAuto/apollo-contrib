cmake_minimum_required(VERSION 2.8)

include_directories(../third_party/security/include)

add_library(security security.cc common.c)
target_link_libraries(security cncrypto cert cncrypto crypto glog)

