cmake_minimum_required(VERSION 3.14)
project(my_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        b514bdc898e2951020cbdca1304b75f5950d1f59 # v1.15.2
)
FetchContent_MakeAvailable(googletest)

# enable_testing()

add_library(linked_list STATIC ${CMAKE_SOURCE_DIR}/src/linked_list.c ${CMAKE_SOURCE_DIR}/include/linked_list.h)

add_executable(
  dns_sd_test_Registration
  dns_sd_test_Registration.cpp
)

target_link_libraries(
  dns_sd_test_Registration
  GTest::gtest_main
)

add_executable(
  linked_list_test_Basic
  linked_list_test_Basic.cpp
)

target_link_libraries(
  linked_list_test_Basic
  GTest::gtest_main
  linked_list
)

option(USE_ASAN "Enable AddressSanitizer" OFF)

if(USE_ASAN)
    set(SANITIZE_FLAGS "-fsanitize=address" "-fno-omit-frame-pointer" "-g" "-O1")
    target_compile_options(linked_list PRIVATE ${SANITIZE_FLAGS})
    target_link_options(linked_list PRIVATE ${SANITIZE_FLAGS})
    target_compile_options(linked_list_test_Basic PRIVATE ${SANITIZE_FLAGS})
    target_link_options(linked_list_test_Basic PRIVATE ${SANITIZE_FLAGS})
endif()

include(GoogleTest)
gtest_discover_tests(dns_sd_test_Registration)
gtest_discover_tests(linked_list_test_Basic)

# TODO: Run all tests, either link every object to a single executable and run
#       or write script that executes all tests