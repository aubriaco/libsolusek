cmake_minimum_required(VERSION 2.8)

## Use the variable PROJECT_NAME for changing the target name
set( PROJECT_NAME "sample-server-test" )

## Set our project name
project(${PROJECT_NAME})

## Use all the *.cpp files we found under this folder for the project
FILE(GLOB SRCS "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

## Define the executable
add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME} solusek)

if(USE_MYSQL)
    add_definitions(-DDUSE_MYSQL)
    target_link_libraries(${PROJECT_NAME} mysqlclient)
endif()

include_directories(${PROJECT_NAME} "src/include/")
