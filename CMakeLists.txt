cmake_minimum_required(VERSION 3.20)
project(gridex_edge_gateway VERSION 0.1.0 LANGUAGES CXX)

add_library(gridex_edge
    src/SafetyEnvelope.cpp
    src/SunStoragePro261Driver.cpp
    src/EdgeController.cpp
)

target_include_directories(gridex_edge PUBLIC include)
target_compile_features(gridex_edge PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(gridex_edge PRIVATE /W4 /permissive-)
else()
    target_compile_options(gridex_edge PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(gridex_edge_demo src/main.cpp)
target_link_libraries(gridex_edge_demo PRIVATE gridex_edge)

enable_testing()
add_executable(gridex_edge_tests tests/test_main.cpp)
target_link_libraries(gridex_edge_tests PRIVATE gridex_edge)
add_test(NAME gridex_edge_tests COMMAND gridex_edge_tests)

