cmake_minimum_required(VERSION 3.20)
project(gridex_tcan485_protocol VERSION 0.1.0 LANGUAGES CXX)

add_library(gridex_mbus_node src/MbusNode.cpp)
target_include_directories(gridex_mbus_node PUBLIC include)
target_compile_features(gridex_mbus_node PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(gridex_mbus_node PRIVATE /W4 /permissive-)
else()
    target_compile_options(gridex_mbus_node PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()
add_executable(gridex_mbus_node_tests tests/test_mbus.cpp)
target_link_libraries(gridex_mbus_node_tests PRIVATE gridex_mbus_node)
add_test(NAME gridex_mbus_node_tests COMMAND gridex_mbus_node_tests)
