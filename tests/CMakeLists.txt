cmake_minimum_required(VERSION 3.14)
project(my_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        b514bdc898e2951020cbdca1304b75f5950d1f59 # v1.15.2
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  dns_sd_test_Registration
  dns_sd_test_Registration.cpp
)
target_link_libraries(
  dns_sd_test_Registration
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(dns_sd_test_Registration)

# TODO: Run all tests, either link every object to a single executable and run
#       or write script that executes all tests