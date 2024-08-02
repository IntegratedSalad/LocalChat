include(CMakePrintHelpers)
cmake_minimum_required(VERSION 3.2)
project(LocalChat)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(BOOST_ROOT /opt/homebrew/Cellar/boost)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
cmake_print_variables(Boost_INCLUDE_DIRS)
find_package(Boost)

include_directories(${CMAKE_SOURCE_DIR}/include)
find_library(DNS_SD_LIBRARY NAMES dns_sd PATHS /usr/local/lib)

add_subdirectory(src)
add_subdirectory(tests)
