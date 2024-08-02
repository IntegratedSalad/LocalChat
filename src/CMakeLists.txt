project(LocalChat)
include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(LocalChat main.cpp)

if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	target_link_libraries(LocalChat ${Boost_LIBRARIES})
	target_link_libraries(LocalChat ${DNS_SD_LIBRARY})
endif()

add_library(BVService_Bonjour STATIC BVService_Bonjour.cpp)
target_link_libraries(LocalChat BVService_Bonjour)

# Compile every single .cpp and make static libs to
# link to the main executable

