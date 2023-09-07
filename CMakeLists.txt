cmake_minimum_required(VERSION 3.22)
project(PBUFHTTP CXX)

#Including usage of external content
include(FetchContent)
include(FindPkgConfig)
#########
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
set(_GRPC_GRPCPP gRPC::grpc++)
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)

find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")


#libhttpserver
find_library(HTTPSERVER_LIB
  NAMES httpserver
  HINTS ${INCLUDE_DIRECTORIES}
  REQUIRED)

#curlpp
pkg_check_modules(CURLPP REQUIRED curlpp)
###############################################################################
#simple_exchange
get_filename_component(simpleex_proto "./simple_exchange.proto" ABSOLUTE)
get_filename_component(simpleex_proto_path "${simpleex_proto}" PATH)
set(simpleex_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/simple_exchange.pb.cc")
set(simpleex_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/simple_exchange.pb.h")
set(simpleex_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/simple_exchange.grpc.pb.cc")
set(simpleex_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/simple_exchange.grpc.pb.h")
add_custom_command(
  OUTPUT "${simpleex_proto_srcs}" "${simpleex_proto_hdrs}" "${simpleex_grpc_srcs}" "${simpleex_grpc_hdrs}"
  COMMAND ${_PROTOBUF_PROTOC}
  ARGS 
  --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
  --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
  --proto_path="${simpleex_proto_path}"
  --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
  "${simpleex_proto}"
  DEPENDS "${simpleex_proto}")

add_library(simpleex_grpc_proto
  ${simpleex_grpc_srcs}
  ${simpleex_grpc_hdrs}
  ${simpleex_proto_srcs}
  ${simpleex_proto_hdrs})
target_link_libraries(simpleex_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

foreach(_target 
    simple_exchange_server simple_exchange_client)
  add_executable(${_target} "${_target}.cc")
  target_link_libraries(${_target}
    simpleex_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    ${HTTPSERVER_LIB}
    ${CURLPP_LDFLAGS}
    )
endforeach()

