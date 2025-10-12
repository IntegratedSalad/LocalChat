include(CMakePrintHelpers)
cmake_minimum_required(VERSION 3.5)
project(LocalChat)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(BOOST_ROOT /opt/homebrew/Cellar/boost)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
cmake_print_variables(Boost_INCLUDE_DIRS)
find_package(Boost)
find_package(Threads REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(tests)

add_library(linked_list STATIC ${CMAKE_SOURCE_DIR}/src/linked_list.c ${CMAKE_SOURCE_DIR}/include/linked_list.h)

include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(LocalChat ${CMAKE_SOURCE_DIR}/src/main.cpp)

if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	target_link_libraries(LocalChat ${Boost_LIBRARIES})
endif()

enable_testing()

add_library(BVService_Bonjour STATIC ${CMAKE_SOURCE_DIR}/src/BVService_Bonjour.cpp)
add_library(BVDiscovery_Bonjour STATIC ${CMAKE_SOURCE_DIR}/src/BVDiscovery_Bonjour.cpp)
target_link_libraries(LocalChat BVService_Bonjour)
target_link_libraries(LocalChat BVDiscovery_Bonjour)
target_link_libraries(LocalChat linked_list)

# We can compile different targets - only BVService_Bonjour, BVService (for BonVoyage main app and for LocalChat) or we can export these classes
# onto a separate git repository. Then, we could use it as a submodule for LC and BV.

# We also should build DEBUG and PROD targets.
# DEBUG target has always memory sanitizer attached to the process, for memory leak checks etc.