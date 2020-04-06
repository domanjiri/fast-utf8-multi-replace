cmake_minimum_required(VERSION 3.10)
 
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++2a -O3 -msse4.2")
    set(CMAKE_EXE_LINKER_FLAGS "-s")
endif()

add_executable(replace_test replace_test.cpp)

add_custom_command(
        TARGET replace_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/test/*.tsv
                ${CMAKE_BINARY_DIR}/)

target_link_libraries(replace_test ${GTEST_LIBRARIES} replace pthread -ltbb)
