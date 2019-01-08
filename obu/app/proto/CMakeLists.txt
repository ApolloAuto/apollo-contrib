cmake_minimum_required(VERSION 2.8)

#find_package(Protobuf REQUIRED)
find_program(PROTOBUF_PROTOC_EXECUTABLE protoc)
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)

include(${CMAKE_CURRENT_SOURCE_DIR}/protobuf.cmake)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Setup proto gen file directories
set(PROTOBUF_GENERATE_CPP_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(MAKE_DIRECTORY ${PROTOBUF_GENERATE_CPP_PATH})

file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")

set(PROTOBUF_GENERATE_CPP_APPEND_PATH TRUE)
V2X_PROTOBUF_GENERATE_CPP(${PROTOBUF_GENERATE_CPP_PATH} PROTOSRCS PROTOHDRS ${PROTO_FILES})
add_custom_target(
    generate_proto
    DEPENDS ${PROTOSRCS} ${PROTOHDRS}
)

V2X_PROTOBUF_GENERATE_GRPC_CPP(${PROTOBUF_GENERATE_CPP_PATH} PROTOGRPCSRCS PROTOGRPCHDRS ${PROTO_FILES})
add_custom_target(
    generate_grpc
    DEPENDS ${PROTOGRPCSRCS} ${PROTOGRPCHDRS}
)

add_library(proto SHARED ${PROTOSRCS} ${PROTOHDRS} ${PROTOGRPCSRCS} ${PROTOGRPCHDRS})
add_dependencies(proto generate_proto generate_grpc)
target_link_libraries(proto protobuf grpc grpc++)
