cmake_minimum_required(VERSION 2.8)

## Use the variable PROJECT_NAME for changing the target name
set( PROJECT_NAME "solusek" )

## Set our project name
project(${PROJECT_NAME})


set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_compile_options(-fPIC -O2 -w)
add_definitions(-DUSE_OPENSSL)

if(MYSQL)
  message("USING MYSQL")
  add_definitions(-DUSE_MYSQL)
endif()
if(PQXX)
  message("USING PQXX")
  add_definitions(-DUSE_PQXX)
endif()

## Use all the *.cpp files we found under this folder for the project
FILE(GLOB SRCS "*.cpp" "*.c" "handlers/*.cpp" "server/*.cpp" "client/*.cpp" "util/*.cpp")


#set(CMAKE_VERBOSE_MAKEFILE ON)
## Define the executable
add_library(${PROJECT_NAME} SHARED ${SRCS})


target_link_libraries(${PROJECT_NAME} ssl crypto)

if(MYSQL)
  target_link_libraries(${PROJECT_NAME} mysqlclient)
endif()
if(PQXX)
  target_link_libraries(${PROJECT_NAME} pqxx)
endif()

install(TARGETS ${PROJECT_NAME}
        LIBRARY
          DESTINATION /usr/local/lib
          COMPONENT Libraries)
install(DIRECTORY include/ DESTINATION /usr/local/include)


include("../sample-server/CMakeLists.txt")
