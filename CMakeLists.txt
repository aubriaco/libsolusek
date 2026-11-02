cmake_minimum_required(VERSION 3.16)

project(solusek VERSION 1.0.0 LANGUAGES CXX)

option(DEBUG "Build with debug symbols and without optimization" OFF)
option(USE_MYSQL "Build the MySQL/MariaDB database backend" OFF)
option(USE_PQXX "Build the PostgreSQL/libpqxx database backend" OFF)
option(USE_REDIS "Build the Redis cache backend" OFF)
option(BUILD_SAMPLE_SERVER "Build the sample server" ON)

if(USE_MYSQL AND USE_PQXX)
    message(FATAL_ERROR "USE_MYSQL and USE_PQXX are mutually exclusive; select one database backend")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

if(DEBUG)
    add_compile_options(-g -O0)
else()
    add_compile_options(-O2)
endif()


file(GLOB_RECURSE SOLUSEK_SOURCES CONFIGURE_DEPENDS
    "src/*.cpp" "src/*.c")
add_library(solusek SHARED ${SOLUSEK_SOURCES})
target_include_directories(solusek PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(solusek PUBLIC OpenSSL::SSL OpenSSL::Crypto Threads::Threads)
target_compile_definitions(solusek PRIVATE USE_OPENSSL)

if(MSVC)
    target_compile_options(solusek PRIVATE /W4)
else()
    target_compile_options(solusek PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(USE_MYSQL)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MARIADB REQUIRED IMPORTED_TARGET libmariadb)
    target_compile_definitions(solusek PUBLIC USE_MYSQL)
    target_link_libraries(solusek PUBLIC PkgConfig::MARIADB)
elseif(USE_PQXX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)
    target_compile_definitions(solusek PUBLIC USE_PQXX)
    target_link_libraries(solusek PUBLIC PkgConfig::PQXX)
endif()

if(USE_REDIS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(HIREDIS REQUIRED IMPORTED_TARGET hiredis)
    target_compile_definitions(solusek PUBLIC USE_REDIS)
    target_link_libraries(solusek PUBLIC PkgConfig::HIREDIS)
endif()

include(GNUInstallDirs)
install(TARGETS solusek EXPORT solusekTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY src/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(BUILD_SAMPLE_SERVER)
    add_executable(sample-server-test sample-server/main.cpp)
    target_link_libraries(sample-server-test PRIVATE solusek)
endif()
