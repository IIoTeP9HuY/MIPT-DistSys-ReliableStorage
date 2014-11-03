project(reliable_storage)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

add_definitions(-std=c++0x -Wall)
add_definitions(-DHAVE_CONFIG_H)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

# CPP
set(CPP_DIR ${SRC_DIR}/cpp)
set(CLIENT_SRC ${CPP_DIR}/Client.cpp)
set(SERVER_SRC ${CPP_DIR}/Server.cpp)
set(LOGGER_SRC ${CPP_DIR}/logger/logger.hpp ${CPP_DIR}/logger/logger.cpp)

# Thrift
find_package(Thrift REQUIRED)
include_directories(${THRIFT_INCLUDE_DIR})
set(THRIFT_DIR ${SRC_DIR}/main/thrift)
set(THRIFT_SRC ${THRIFT_DIR}/ReplicatedStorage.thrift)

set(GEN_FILES_PATH ${PROJECT_BINARY_DIR}/gen-cpp/)
set(THRIFT_TARGETS
    ${GEN_FILES_PATH}/Coordinator.cpp
    ${GEN_FILES_PATH}/ReplicatedStorage_types.cpp
    ${GEN_FILES_PATH}/ReplicatedStorage_constants.cpp
    )

add_custom_command(
OUTPUT ${THRIFT_TARGETS}
COMMAND
    thrift --gen cpp --gen java ${THRIFT_SRC}
COMMENT "Thrifting service" 
)

add_library(thrift_interface ${THRIFT_TARGETS})

add_library(logger ${LOGGER_SRC})

include_directories(${GEN_FILES_PATH})
add_executable(client ${CLIENT_SRC})
add_executable(server ${SERVER_SRC})

target_link_libraries(client logger thrift thrift_interface)
target_link_libraries(server logger thrift thrift_interface)
