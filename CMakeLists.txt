cmake_minimum_required(VERSION 2.8)

## Use the variable PROJECT_NAME for changing the target name
set( PROJECT_NAME "solusek" )

## Set our project name
project(${PROJECT_NAME})


set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(DEBUG)
  add_compile_options(-fPIC -g -w -std=c++11)
else()
  add_compile_options(-fPIC -O2 -w -std=c++11)
endif()
add_definitions(-DUSE_OPENSSL)

if(USE_MYSQL)
  message("USING MYSQL")
  add_definitions(-DUSE_MYSQL)
endif()
if(USE_PQXX)
  message("USING PQXX")
  add_definitions(-DUSE_PQXX)
endif()
if(USE_REDIS)
  message("USING REDIS")
  add_definitions(-DUSE_REDIS)
endif()

## Use all the *.cpp files we found under this folder for the project
FILE(GLOB SRCS "src/*.cpp" "src/*.c" "src/handlers/*.cpp" "src/server/*.cpp" "src/client/*.cpp" "src/util/*.cpp")


#set(CMAKE_VERBOSE_MAKEFILE ON)
## Define the executable
add_library(${PROJECT_NAME} SHARED ${SRCS})


target_link_libraries(${PROJECT_NAME} ssl crypto)

if(USE_MYSQL)
  target_link_libraries(${PROJECT_NAME} mysqlclient)
endif()
if(USE_PQXX)
  target_link_libraries(${PROJECT_NAME} pqxx)
endif()
if(USE_REDIS)
  target_link_libraries(${PROJECT_NAME} hiredis)
endif()

install(TARGETS ${PROJECT_NAME}
        LIBRARY
          DESTINATION /usr/local/lib
          COMPONENT Libraries)
install(DIRECTORY src/include/ DESTINATION /usr/local/include)


include("sample-server/CMakeLists.txt")
