cmake_minimum_required(VERSION 3.20)
project(gridex_rockpie_base VERSION 0.1.0 LANGUAGES CXX)

add_subdirectory(.. ${CMAKE_BINARY_DIR}/gridex-core EXCLUDE_FROM_ALL)

add_library(gridex_rockpie_transport
    src/MbusRtuClient.cpp
    src/PosixModbusTcpClient.cpp
)
target_include_directories(gridex_rockpie_transport PUBLIC include)
target_link_libraries(gridex_rockpie_transport PUBLIC gridex_edge)
target_compile_features(gridex_rockpie_transport PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(gridex_rockpie_transport PRIVATE /W4 /permissive-)
else()
    target_compile_options(gridex_rockpie_transport PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(gridex_rockpie_service src/main.cpp)
target_link_libraries(gridex_rockpie_service PRIVATE gridex_rockpie_transport)

install(TARGETS gridex_rockpie_service RUNTIME DESTINATION bin)
install(FILES systemd/gridex-rockpie.service DESTINATION lib/systemd/system)
install(FILES config/gridex-rockpie.env.example DESTINATION share/gridex)
