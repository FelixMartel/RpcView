cmake_minimum_required (VERSION 3.0.2)

message("[RpcDec]")

project(RpcDec)

include_directories(${CMAKE_SOURCE_DIR}/RpcCore ${CMAKE_SOURCE_DIR}/RpcDecompiler)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(
    RpcDec
    RpcDec.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/wd4100 /wd4101 /wd4189 /wd4244 /wd4312")