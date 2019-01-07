add_library(proxy STATIC proxy.cc)
target_link_libraries(proxy proto protobuf grpc grpc++ pthread)
